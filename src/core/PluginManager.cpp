// PluginManager.cpp - Simplified implementation
#include "PluginManager.h"
#include "../managers/PluginSecurityManager.h"
#include "PluginCommunicationBus.h"
#include "../utils/PluginHelpers.h"
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <QJsonDocument>
#include <QDirIterator>
#include <QProcess>
#include <QCryptographicHash>
#include <QtConcurrent>
#include <QFutureWatcher>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QJsonObject>

Q_LOGGING_CATEGORY(pluginManager, "plugin.manager")

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , m_securityManager(std::make_unique<PluginSecurityManager>(this))
    , m_communicationBus(std::make_unique<PluginCommunicationBus>(this))
    , m_metricsCollector(nullptr)
    , m_hotReloadEnabled(false)
    , m_performanceMonitoringEnabled(false)
    , m_loadedPluginCount(0)
{
    qCInfo(pluginManager) << "PluginManager initialized";
}

PluginManager::~PluginManager() {
    unloadAllPlugins();
}

PluginManager::LoadResult PluginManager::loadPlugin(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qCWarning(pluginManager) << "Plugin file not found:" << filePath;
        return LoadResult::FileNotFound;
    }

    QString pluginName = fileInfo.baseName();
    
    // Check if already loaded
    {
        QReadLocker locker(&m_pluginsLock);
        if (m_plugins.find(pluginName) != m_plugins.end()) {
            qCWarning(pluginManager) << "Plugin already loaded:" << pluginName;
            return LoadResult::AlreadyLoaded;
        }
    }

    // Load the plugin
    auto loader = std::make_unique<QPluginLoader>(filePath);
    QObject* instance = loader->instance();
    if (!instance) {
        qCWarning(pluginManager) << "Failed to load plugin:" << loader->errorString();
        return LoadResult::InvalidPlugin;
    }

    auto* plugin = qobject_cast<IPlugin*>(instance);
    if (!plugin) {
        qCWarning(pluginManager) << "Plugin does not implement IPlugin interface";
        return LoadResult::InvalidPlugin;
    }

    // Initialize plugin
    if (!plugin->initialize()) {
        qCWarning(pluginManager) << "Plugin initialization failed:" << pluginName;
        return LoadResult::InitializationFailed;
    }

    // Create plugin info
    auto pluginInfo = std::make_unique<PluginInfo>();
    pluginInfo->filePath = filePath;
    pluginInfo->name = pluginName;
    pluginInfo->uuid = plugin->uuid();
    pluginInfo->loader = std::move(loader);
    pluginInfo->instance = plugin;
    pluginInfo->loadTime = QDateTime::currentDateTime();
    pluginInfo->status = PluginStatus::Running;

    // Register plugin
    {
        QWriteLocker locker(&m_pluginsLock);
        m_plugins[pluginName] = std::move(pluginInfo);
        m_pluginUuidMap[plugin->uuid()] = pluginName;
        ++m_loadedPluginCount;
    }

    emit pluginLoaded(pluginName);
    emit pluginCountChanged(m_loadedPluginCount);
    
    qCInfo(pluginManager) << "Plugin loaded successfully:" << pluginName;
    
    return LoadResult::Success;
}

PluginManager::UnloadResult PluginManager::unloadPlugin(const QString& pluginName, bool /*force*/) {
    QWriteLocker locker(&m_pluginsLock);
    
    auto it = m_plugins.find(pluginName);
    if (it == m_plugins.end()) {
        qCWarning(pluginManager) << "Plugin not found:" << pluginName;
        return UnloadResult::PluginNotFound;
    }

    auto& pluginInfo = it->second;
    
    if (pluginInfo->instance) {
        pluginInfo->instance->cleanup();
    }
    
    m_pluginUuidMap.remove(pluginInfo->uuid);
    m_plugins.erase(it);
    --m_loadedPluginCount;
    
    emit pluginUnloaded(pluginName);
    emit pluginCountChanged(m_loadedPluginCount);
    
    qCInfo(pluginManager) << "Plugin unloaded successfully:" << pluginName;
    
    return UnloadResult::Success;
}

void PluginManager::unloadAllPlugins() {
    QWriteLocker locker(&m_pluginsLock);
    
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if (it->second->instance) {
            it->second->instance->cleanup();
        }
    }
    
    m_plugins.clear();
    m_pluginUuidMap.clear();
    m_loadedPluginCount = 0;
    
    emit pluginCountChanged(m_loadedPluginCount);
    qCInfo(pluginManager) << "All plugins unloaded";
}

void PluginManager::scanDirectory(const QString& directory, bool recursive) {
    QDir dir(directory);
    if (!dir.exists()) {
        qCWarning(pluginManager) << "Plugin directory does not exist:" << directory;
        return;
    }

    qCInfo(pluginManager) << "Scanning directory:" << directory;

    QStringList filters;
    filters << "*.dll" << "*.so" << "*.dylib";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        auto result = loadPlugin(filePath);
        Q_UNUSED(result)
    }
    
    if (recursive) {
        QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& subdir : subdirs) {
            scanDirectory(subdir.absoluteFilePath(), true);
        }
    }
}

QStringList PluginManager::loadedPlugins() const {
    QReadLocker locker(&m_pluginsLock);
    QStringList result;
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        result.append(it->first);
    }
    return result;
}

IPlugin* PluginManager::getPlugin(const QString& name) const {
    QReadLocker locker(&m_pluginsLock);
    auto it = m_plugins.find(name);
    return (it != m_plugins.end()) ? it->second->instance : nullptr;
}

bool PluginManager::isPluginLoaded(const QString& name) const {
    QReadLocker locker(&m_pluginsLock);
    return m_plugins.find(name) != m_plugins.end();
}

// **Property implementations**
int PluginManager::loadedPluginCount() const {
    return m_loadedPluginCount.load();
}

void PluginManager::setHotReloadEnabled(bool enabled) {
    if (m_hotReloadEnabled != enabled) {
        m_hotReloadEnabled = enabled;
        emit hotReloadEnabledChanged(enabled);

        if (enabled) {
            // Setup file system watcher for hot reload
            setupFileWatcher();
        } else {
            // Cleanup file system watcher
            if (m_fileWatcher) {
                m_fileWatcher->removePaths(m_fileWatcher->files());
            }
        }
    }
}

void PluginManager::setAutoLoadEnabled(bool enabled) {
    if (m_autoLoadEnabled != enabled) {
        m_autoLoadEnabled = enabled;
        emit autoLoadEnabledChanged(enabled);
    }
}

// **Public slot implementations**
void PluginManager::pausePlugin(const QString& pluginName) {
    QReadLocker locker(&m_pluginsLock);
    auto it = m_plugins.find(pluginName);
    if (it != m_plugins.end() && it->second->instance) {
        if (it->second->instance->pause()) {
            it->second->status = PluginStatus::Paused;
            emit pluginStatusChanged(pluginName, PluginStatus::Paused);
            qCInfo(pluginManager) << "Plugin paused:" << pluginName;
        } else {
            qCWarning(pluginManager) << "Failed to pause plugin:" << pluginName;
        }
    }
}

void PluginManager::resumePlugin(const QString& pluginName) {
    QReadLocker locker(&m_pluginsLock);
    auto it = m_plugins.find(pluginName);
    if (it != m_plugins.end() && it->second->instance) {
        if (it->second->instance->resume()) {
            it->second->status = PluginStatus::Loaded;
            emit pluginStatusChanged(pluginName, PluginStatus::Loaded);
            qCInfo(pluginManager) << "Plugin resumed:" << pluginName;
        } else {
            qCWarning(pluginManager) << "Failed to resume plugin:" << pluginName;
        }
    }
}

void PluginManager::restartPlugin(const QString& pluginName) {
    QReadLocker locker(&m_pluginsLock);
    auto it = m_plugins.find(pluginName);
    if (it != m_plugins.end() && it->second->instance) {
        if (it->second->instance->restart()) {
            emit pluginStatusChanged(pluginName, PluginStatus::Loaded);
            qCInfo(pluginManager) << "Plugin restarted:" << pluginName;
        } else {
            qCWarning(pluginManager) << "Failed to restart plugin:" << pluginName;
        }
    }
}

void PluginManager::refreshPluginList() {
    qCInfo(pluginManager) << "Refreshing plugin list...";

    // Rescan all plugin directories
    for (const QString& path : pluginSearchPaths()) {
        scanDirectory(path);
    }

    // Signal that plugin list has been refreshed
    emit pluginCountChanged(m_loadedPluginCount.load());
}

// **Private slot implementations**
void PluginManager::handleFileChanged(const QString& path) {
    if (!m_hotReloadEnabled) return;

    qCInfo(pluginManager) << "File changed, attempting hot reload:" << path;

    // Find plugin by file path and reload it
    QReadLocker locker(&m_pluginsLock);
    for (const auto& [name, info] : m_plugins) {
        if (info->filePath == path) {
            // Reload the plugin
            locker.unlock();
            auto unloadResult = unloadPlugin(name);
            auto loadResult = loadPlugin(path);
            Q_UNUSED(unloadResult)
            Q_UNUSED(loadResult)
            break;
        }
    }
}

void PluginManager::performanceCheck() {
    if (!m_performanceMonitoringEnabled) return;

    QReadLocker locker(&m_pluginsLock);
    for (const auto& [name, info] : m_plugins) {
        if (info->instance && info->status == PluginStatus::Loaded) {
            QJsonObject metrics = info->instance->getMetrics();

            // Check for performance issues
            if (metrics.contains("memoryUsage")) {
                double memoryMB = metrics["memoryUsage"].toDouble();
                if (memoryMB > 100.0) { // 100MB threshold
                    emit performanceAlert(name, tr("High memory usage: %1 MB").arg(memoryMB));
                }
            }

            if (metrics.contains("cpuUsage")) {
                double cpuPercent = metrics["cpuUsage"].toDouble();
                if (cpuPercent > 80.0) { // 80% threshold
                    emit performanceAlert(name, tr("High CPU usage: %1%").arg(cpuPercent));
                }
            }
        }
    }
}

void PluginManager::cleanupUnusedPlugins() {
    QStringList toRemove;

    {
        QReadLocker locker(&m_pluginsLock);
        for (const auto& [name, info] : m_plugins) {
            if (info->status == PluginStatus::Error ||
                info->status == PluginStatus::Stopped) {
                toRemove << name;
            }
        }
    }

    for (const QString& name : toRemove) {
        auto result = unloadPlugin(name, true);
        Q_UNUSED(result)
    }

    qCInfo(pluginManager) << "Cleaned up" << toRemove.size() << "unused plugins";
}

void PluginManager::saveMetrics() {
    if (!m_metricsCollector) return;

    QJsonObject systemMetrics = getSystemMetrics();

    // Save metrics to file or database
    QString metricsFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/metrics.json";
    QFile file(metricsFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(systemMetrics);
        file.write(doc.toJson());
        qCInfo(pluginManager) << "Metrics saved to:" << metricsFile;
    }
}

// **Helper methods**
void PluginManager::setupFileWatcher() {
    if (!m_fileWatcher) {
        m_fileWatcher = std::make_unique<QFileSystemWatcher>(this);
        connect(m_fileWatcher.get(), &QFileSystemWatcher::fileChanged,
                this, &PluginManager::handleFileChanged);
    }

    // Add all loaded plugin files to watcher
    QReadLocker locker(&m_pluginsLock);
    for (const auto& [name, info] : m_plugins) {
        if (!info->filePath.isEmpty()) {
            m_fileWatcher->addPath(info->filePath);
        }
    }
}

// **Additional method implementations**
QStringList PluginManager::pluginSearchPaths() const {
    return m_searchPaths;
}

void PluginManager::addPluginSearchPath(const QString& path) {
    if (!m_searchPaths.contains(path)) {
        m_searchPaths.append(path);
        qCInfo(pluginManager) << "Added plugin search path:" << path;
    }
}

void PluginManager::startPerformanceMonitoring() {
    m_performanceMonitoringEnabled = true;
    if (m_performanceTimer) {
        m_performanceTimer->start();
    }
    qCInfo(pluginManager) << "Performance monitoring started";
}

QJsonObject PluginManager::getSystemMetrics() const {
    QJsonObject metrics;

    metrics["totalPlugins"] = static_cast<int>(m_plugins.size());
    metrics["loadedPlugins"] = m_loadedPluginCount.load();
    metrics["hotReloadEnabled"] = m_hotReloadEnabled;
    metrics["autoLoadEnabled"] = m_autoLoadEnabled;
    metrics["performanceMonitoringEnabled"] = m_performanceMonitoringEnabled;
    metrics["sandboxEnabled"] = m_sandboxEnabled;

    // Add memory and CPU usage if available
    metrics["systemMemoryUsage"] = 0; // Placeholder
    metrics["systemCpuUsage"] = 0.0; // Placeholder

    return metrics;
}

QJsonObject PluginManager::getPluginMetrics(const QString& name) const {
    QReadLocker locker(&m_pluginsLock);
    auto it = m_plugins.find(name);

    if (it != m_plugins.end() && it->second->instance) {
        return it->second->instance->getMetrics();
    }

    return QJsonObject();
}

// Stub implementations for missing methods
bool PluginManager::validatePluginSignature(const QString& /*filePath*/) const {
    return true; // Stub implementation
}

void PluginManager::handlePluginDestroyed() {
    // Stub implementation
}

void PluginManager::watchPluginFile(const QString& /*filePath*/) {
    // Stub implementation
}

void PluginManager::unwatchPluginFile(const QString& /*filePath*/) {
    // Stub implementation
}

void PluginManager::updatePluginStatus(const QString& /*pluginName*/, PluginStatus /*status*/) {
    // Stub implementation
}

void PluginManager::logPluginActivity(const QString& /*pluginName*/, const QString& /*activity*/) {
    // Stub implementation
}

QJsonObject PluginManager::loadPluginConfiguration(const QString& /*pluginName*/) const {
    return QJsonObject(); // Stub implementation
}

void PluginManager::savePluginConfiguration(const QString& /*pluginName*/, const QJsonObject& /*config*/) {
    // Stub implementation
}

void PluginManager::saveAllConfigurations() {
    // Stub implementation
}
