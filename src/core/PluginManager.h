// PluginManager.h - Enhanced plugin management system
#pragma once
#include <QObject>
#include <QDir>
#include <QPluginLoader>
#include <QJsonObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QThreadPool>
#include <QReadWriteLock>
#include <QMutex>
#include <QHash>
#include <QStringList>
#include <QLoggingCategory>
#include <unordered_map>
#include <memory>
#include <atomic>
#include "PluginInterface.h"

Q_DECLARE_LOGGING_CATEGORY(pluginManager)

class PluginSecurityManager;
class PluginCommunicationBus;
class PluginMetricsCollector;

class PluginManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(int loadedPluginCount READ loadedPluginCount NOTIFY pluginCountChanged)
    Q_PROPERTY(bool hotReloadEnabled READ hotReloadEnabled WRITE setHotReloadEnabled NOTIFY hotReloadEnabledChanged)
    Q_PROPERTY(bool autoLoadEnabled READ autoLoadEnabled WRITE setAutoLoadEnabled NOTIFY autoLoadEnabledChanged)

public:
    enum class LoadResult {
        Success,
        FileNotFound,
        InvalidPlugin,
        DependencyMissing,
        InitializationFailed,
        VersionMismatch,
        SecurityViolation,
        AlreadyLoaded,
        IncompatibleArchitecture,
        LicenseViolation,
        ResourceConflict
    };

    enum class UnloadResult {
        Success,
        PluginNotFound,
        HasDependents,
        CleanupFailed,
        ForceUnloaded
    };

    struct PluginInfo {
        QString filePath;
        QString name;
        QUuid uuid;
        QJsonObject metadata;
        std::unique_ptr<QPluginLoader> loader;
        IPlugin* instance = nullptr;
        PluginStatus status = PluginStatus::Discovered;
        QStringList unresolvedDependencies;
        QDateTime loadTime;
        QDateTime lastAccess;
        qint64 memoryUsage = 0;
        double cpuUsage = 0.0;
        QStringList errorLog;
        SecurityLevel securityLevel = SecurityLevel::Sandbox;
        PluginCapabilities capabilities = PluginCapability::None;
        bool autoStart = false;
        int priority = 0;
        QJsonObject configuration;
        std::atomic<bool> isUnloading{false};
    };

    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    // **Plugin loading and management**
    [[nodiscard]] LoadResult loadPlugin(const QString& filePath);
    [[nodiscard]] UnloadResult unloadPlugin(const QString& pluginName, bool force = false);
    
    // **Plugin discovery**
    void scanDirectory(const QString& directory, bool recursive = false);
    void scanDirectoryAsync(const QString& directory, bool recursive = false);
    void addPluginSearchPath(const QString& path);
    void removePluginSearchPath(const QString& path);
    QStringList pluginSearchPaths() const;
    
    // **Plugin lifecycle**
    void loadAllPlugins();
    void unloadAllPlugins();
    void reloadPlugin(const QString& pluginName);
    void reloadAllPlugins();
    
    // **Plugin querying**
    template<typename PluginType>
    [[nodiscard]] QList<PluginType*> getPluginsOfType() const {
        QReadLocker locker(&m_pluginsLock);
        QList<PluginType*> result;
        for (const auto& [name, info] : m_plugins) {
            if (info->status == PluginStatus::Running) {
                if (auto* plugin = qobject_cast<PluginType*>(info->instance)) {
                    result.append(plugin);
                }
            }
        }
        return result;
    }
    
    [[nodiscard]] IPlugin* getPlugin(const QString& name) const;
    [[nodiscard]] IPlugin* getPluginByUuid(const QUuid& uuid) const;
    [[nodiscard]] bool isPluginLoaded(const QString& name) const;
    [[nodiscard]] QStringList availablePlugins() const;
    [[nodiscard]] QStringList loadedPlugins() const;
    [[nodiscard]] QStringList runningPlugins() const;
    [[nodiscard]] QList<PluginInfo*> getPluginsByCapability(PluginCapabilities capabilities) const;
    [[nodiscard]] QList<PluginInfo*> getPluginsByCategory(const QString& category) const;
    
    // **Plugin status and metrics**
    [[nodiscard]] PluginStatus getPluginStatus(const QString& name) const;
    [[nodiscard]] QJsonObject getPluginMetrics(const QString& name) const;
    [[nodiscard]] QJsonObject getSystemMetrics() const;
    [[nodiscard]] int loadedPluginCount() const;
    
    // **Configuration management**
    void savePluginConfiguration(const QString& pluginName, const QJsonObject& config);
    [[nodiscard]] QJsonObject loadPluginConfiguration(const QString& pluginName) const;
    void saveAllConfigurations();
    void loadAllConfigurations();
    
    // **Plugin communication**
    [[nodiscard]] QVariant sendCommand(const QString& pluginName, const QString& command, 
                                      const QVariantMap& params = {});
    void broadcastMessage(const QString& message, const QVariantMap& data = {});
    
    // **Hot reload support**
    [[nodiscard]] bool hotReloadEnabled() const { return m_hotReloadEnabled; }
    void setHotReloadEnabled(bool enabled);
    
    // **Auto-loading**
    [[nodiscard]] bool autoLoadEnabled() const { return m_autoLoadEnabled; }
    void setAutoLoadEnabled(bool enabled);
    
    // **Plugin updates**
    void checkForUpdates();
    void updatePlugin(const QString& pluginName);
    void updateAllPlugins();
    
    // **Security**
    void setSecurityLevel(SecurityLevel level);
    SecurityLevel securityLevel() const { return m_securityLevel; }
    bool validatePluginSecurity(const QString& pluginName) const;
    
    // **Performance monitoring**
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    bool isPerformanceMonitoringEnabled() const { return m_performanceMonitoringEnabled; }
    
    // **Plugin sandboxing**
    void enableSandboxMode(bool enabled);
    bool isSandboxModeEnabled() const { return m_sandboxEnabled; }

public slots:
    void pausePlugin(const QString& pluginName);
    void resumePlugin(const QString& pluginName);
    void restartPlugin(const QString& pluginName);
    void refreshPluginList();

signals:
    void pluginLoaded(const QString& pluginName);
    void pluginUnloaded(const QString& pluginName);
    void pluginStatusChanged(const QString& pluginName, PluginStatus status);
    void pluginError(const QString& pluginName, const QString& error);
    void pluginCountChanged(int count);
    void hotReloadEnabledChanged(bool enabled);
    void autoLoadEnabledChanged(bool enabled);
    void pluginUpdated(const QString& pluginName, const QVersionNumber& newVersion);
    void securityViolation(const QString& pluginName, const QString& violation);
    void performanceAlert(const QString& pluginName, const QString& alert);
    void pluginCommunication(const QString& from, const QString& to, const QVariantMap& data);

private slots:
    void handlePluginDestroyed();
    void handleFileChanged(const QString& path);
    void performanceCheck();
    void cleanupUnusedPlugins();
    void saveMetrics();

private:
    // **Plugin management internals**
    [[nodiscard]] bool resolveDependencies(const QString& pluginName);
    [[nodiscard]] bool validatePlugin(IPlugin* plugin) const;
    [[nodiscard]] bool checkPluginConflicts(const QString& pluginName) const;
    void registerPluginMetadata(const QString& name, const QJsonObject& metadata);
    void updatePluginStatus(const QString& name, PluginStatus status);
    void logPluginActivity(const QString& pluginName, const QString& activity);
    
    // **Security and validation**
    [[nodiscard]] bool validatePluginSignature(const QString& filePath) const;
    [[nodiscard]] bool checkPluginPermissions(IPlugin* plugin) const;
    void applySandboxRestrictions(const QString& pluginName);
    
    // **Performance monitoring**
    void collectPluginMetrics();
    void analyzePluginPerformance();
    void optimizePluginLoading();
    
    // **File system monitoring**
    void setupFileWatcher();
    void watchPluginFile(const QString& filePath);
    void unwatchPluginFile(const QString& filePath);

    // **Member variables**
    mutable QReadWriteLock m_pluginsLock;
    std::unordered_map<QString, std::unique_ptr<PluginInfo>> m_plugins;
    QHash<QUuid, QString> m_pluginUuidMap;
    
    QStringList m_searchPaths;
    QString m_configPath;
    
    std::unique_ptr<QFileSystemWatcher> m_fileWatcher;
    std::unique_ptr<QTimer> m_performanceTimer;
    std::unique_ptr<QTimer> m_cleanupTimer;
    std::unique_ptr<QTimer> m_metricsTimer;
    
    std::unique_ptr<PluginSecurityManager> m_securityManager;
    std::unique_ptr<PluginCommunicationBus> m_communicationBus;
    std::unique_ptr<PluginMetricsCollector> m_metricsCollector;
    
    SecurityLevel m_securityLevel = SecurityLevel::Sandbox;
    bool m_hotReloadEnabled = false;
    bool m_autoLoadEnabled = true;
    bool m_performanceMonitoringEnabled = false;
    bool m_sandboxEnabled = true;
    
    std::atomic<int> m_loadedPluginCount{0};
    QMutex m_configMutex;
    QThreadPool* m_threadPool;
};