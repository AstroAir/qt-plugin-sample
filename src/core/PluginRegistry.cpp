
// PluginRegistry.cpp
#include "PluginRegistry.h"
#include "PluginManager.h"
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

PluginRegistry::PluginRegistry(QObject* parent)
    : QAbstractListModel(parent)
    , m_pluginManager(nullptr)
{
    // **Get plugin manager instance**
    m_pluginManager = qobject_cast<PluginManager*>(parent);
    
    if (m_pluginManager) {
        connect(m_pluginManager, &PluginManager::pluginLoaded,
                this, &PluginRegistry::onPluginLoaded);
        connect(m_pluginManager, &PluginManager::pluginUnloaded,
                this, &PluginRegistry::onPluginUnloaded);
        // ... connect other signals
    }
}

int PluginRegistry::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return m_filteredPlugins.size();
}

QVariant PluginRegistry::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_filteredPlugins.size()) {
        return QVariant();
    }

    const auto& plugin = m_filteredPlugins.at(index.row());

    switch (role) {
    case NameRole: return plugin.name;
    case DescriptionRole: return plugin.description;
    case VersionRole: return plugin.version.toString();
    case AuthorRole: return plugin.author;
    case EnabledRole: return plugin.enabled;
    case TypeRole: return plugin.type;
    case StatusRole: return plugin.status;
    case CapabilitiesRole: return plugin.capabilities;
    case SecurityLevelRole: return plugin.securityLevel;
    case MemoryUsageRole: return plugin.memoryUsage;
    case CpuUsageRole: return plugin.cpuUsage;
    case LoadTimeRole: return plugin.loadTime;
    case ErrorCountRole: return plugin.errorCount;
    case UuidRole: return plugin.uuid.toString();
    case FileSizeRole: return plugin.fileSize;
    case LastUpdateRole: return plugin.lastUpdate;
    default: return QVariant();
    }
}

QHash<int, QByteArray> PluginRegistry::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[VersionRole] = "version";
    roles[AuthorRole] = "author";
    roles[EnabledRole] = "enabled";
    roles[TypeRole] = "type";
    roles[StatusRole] = "status";
    roles[CapabilitiesRole] = "capabilities";
    roles[SecurityLevelRole] = "securityLevel";
    roles[MemoryUsageRole] = "memoryUsage";
    roles[CpuUsageRole] = "cpuUsage";
    roles[LoadTimeRole] = "loadTime";
    roles[ErrorCountRole] = "errorCount";
    roles[UuidRole] = "uuid";
    roles[FileSizeRole] = "fileSize";
    roles[LastUpdateRole] = "lastUpdate";
    return roles;
}

bool PluginRegistry::PluginDescriptor::matchesFilter(const QString& filter) const {
    if (filter.isEmpty()) return true;
    
    QString lowerFilter = filter.toLower();
    return name.toLower().contains(lowerFilter) ||
           description.toLower().contains(lowerFilter) ||
           author.toLower().contains(lowerFilter) ||
           type.toLower().contains(lowerFilter) ||
           capabilities.join(" ").toLower().contains(lowerFilter);
}

void PluginRegistry::applyFilter() {
    beginResetModel();
    
    m_filteredPlugins.clear();
    std::copy_if(m_plugins.begin(), m_plugins.end(),
                 std::back_inserter(m_filteredPlugins),
                 [this](const PluginDescriptor& plugin) {
                     return plugin.matchesFilter(m_filterText);
                 });
    
    applySorting();
    endResetModel();
    
    emit countChanged();
}

void PluginRegistry::applySorting() {
    std::sort(m_filteredPlugins.begin(), m_filteredPlugins.end(),
              [this](const PluginDescriptor& a, const PluginDescriptor& b) {
                  bool ascending = (m_sortOrder == Qt::AscendingOrder);
                  
                  if (m_sortRole == "name") {
                      return ascending ? (a.name < b.name) : (a.name > b.name);
                  } else if (m_sortRole == "version") {
                      return ascending ? (a.version < b.version) : (a.version > b.version);
                  } else if (m_sortRole == "loadTime") {
                      return ascending ? (a.loadTime < b.loadTime) : (a.loadTime > b.loadTime);
                  } else if (m_sortRole == "memoryUsage") {
                      return ascending ? (a.memoryUsage < b.memoryUsage) : (a.memoryUsage > b.memoryUsage);
                  }
                  // Default to name sorting
                  return ascending ? (a.name < b.name) : (a.name > b.name);
              });
}

// **Public slot implementations**
void PluginRegistry::refreshPlugins() {
    if (m_pluginManager) {
        m_pluginManager->refreshPluginList();
        // Refresh the plugin list by reapplying filters
        applyFilter();
    }
}

void PluginRegistry::sortPlugins() {
    beginResetModel();
    applySorting();
    endResetModel();
}

void PluginRegistry::onPluginLoaded(const QString& name) {
    updatePluginDescriptor(name);
    applyFilter();
}

void PluginRegistry::onPluginUnloaded(const QString& name) {
    // Remove plugin from registry
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                          [&name](const PluginDescriptor& desc) {
                              return desc.name == name;
                          });

    if (it != m_plugins.end()) {
        beginResetModel();
        m_plugins.erase(it);
        applyFilter();
        endResetModel();
    }
}

void PluginRegistry::onPluginStatusChanged(const QString& name, PluginStatus status) {
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                          [&name](PluginDescriptor& desc) {
                              return desc.name == name;
                          });

    if (it != m_plugins.end()) {
        // Convert PluginStatus enum to string
        QString statusStr;
        switch (status) {
        case PluginStatus::Unknown: statusStr = "Unknown"; break;
        case PluginStatus::Discovered: statusStr = "Discovered"; break;
        case PluginStatus::Loading: statusStr = "Loading"; break;
        case PluginStatus::Loaded: statusStr = "Loaded"; break;
        case PluginStatus::Initializing: statusStr = "Initializing"; break;
        case PluginStatus::Running: statusStr = "Running"; break;
        case PluginStatus::Paused: statusStr = "Paused"; break;
        case PluginStatus::Stopping: statusStr = "Stopping"; break;
        case PluginStatus::Stopped: statusStr = "Stopped"; break;
        case PluginStatus::Error: statusStr = "Error"; break;
        case PluginStatus::Unloading: statusStr = "Unloading"; break;
        }
        it->status = statusStr;

        // Find index in filtered list and emit data changed
        auto filteredIt = std::find_if(m_filteredPlugins.begin(), m_filteredPlugins.end(),
                                      [&name](const PluginDescriptor& desc) {
                                          return desc.name == name;
                                      });

        if (filteredIt != m_filteredPlugins.end()) {
            int row = std::distance(m_filteredPlugins.begin(), filteredIt);
            QModelIndex idx = index(row);
            emit dataChanged(idx, idx, {StatusRole});
        }
    }
}

bool PluginRegistry::enablePlugin(const QString& name) {
    if (m_pluginManager) {
        // Enable plugin through manager
        auto result = m_pluginManager->loadPlugin(name);
        if (result == PluginManager::LoadResult::Success) {
            updatePluginDescriptor(name);
            return true;
        }
    }
    return false;
}

bool PluginRegistry::disablePlugin(const QString& name) {
    if (m_pluginManager) {
        // Disable plugin through manager
        auto result = m_pluginManager->unloadPlugin(name);
        if (result == PluginManager::UnloadResult::Success) {
            updatePluginDescriptor(name);
            return true;
        }
    }
    return false;
}

// **Query methods**
QJsonObject PluginRegistry::getPluginInfo(const QString& name) const {
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                          [&name](const PluginDescriptor& desc) {
                              return desc.name == name;
                          });

    if (it != m_plugins.end()) {
        QJsonObject info;
        info["name"] = it->name;
        info["description"] = it->description;
        info["version"] = it->version.toString();
        info["author"] = it->author;
        info["enabled"] = it->enabled;
        info["type"] = it->type;
        info["status"] = it->status;
        info["capabilities"] = QJsonArray::fromStringList(it->capabilities);
        info["uuid"] = it->uuid.toString();
        info["securityLevel"] = it->securityLevel;
        info["memoryUsage"] = it->memoryUsage;
        info["cpuUsage"] = it->cpuUsage;
        info["errorCount"] = it->errorCount;
        return info;
    }

    return QJsonObject(); // Return empty object if not found
}

QStringList PluginRegistry::getPluginsByType(const QString& type) const {
    QStringList result;
    for (const auto& plugin : m_plugins) {
        if (plugin.type == type) {
            result << plugin.name;
        }
    }
    return result;
}

QStringList PluginRegistry::getPluginsByCapability(const QString& capability) const {
    QStringList result;
    for (const auto& plugin : m_plugins) {
        if (plugin.capabilities.contains(capability)) {
            result << plugin.name;
        }
    }
    return result;
}

void PluginRegistry::installPlugin(const QString& filePath) {
    if (m_pluginManager) {
        auto result = m_pluginManager->loadPlugin(filePath);
        if (result == PluginManager::LoadResult::Success) {
            applyFilter(); // Refresh the list
        }
    }
}

void PluginRegistry::uninstallPlugin(const QString& name) {
    if (m_pluginManager) {
        auto result = m_pluginManager->unloadPlugin(name);
        if (result == PluginManager::UnloadResult::Success) {
            onPluginUnloaded(name);
        }
    }
}

void PluginRegistry::updatePlugin(const QString& name) {
    // Unload and reload the plugin
    if (m_pluginManager) {
        auto unloadResult = m_pluginManager->unloadPlugin(name);
        if (unloadResult == PluginManager::UnloadResult::Success) {
            // Try to reload from the same location
            // Note: In a real implementation, you'd store the file path
            onPluginUnloaded(name);
        }
    }
}

QJsonObject PluginRegistry::getPluginMetrics(const QString& name) const {
    if (m_pluginManager) {
        return m_pluginManager->getPluginMetrics(name);
    }
    return QJsonObject();
}

void PluginRegistry::configurePlugin(const QString& name, const QJsonObject& config) {
    if (m_pluginManager) {
        m_pluginManager->savePluginConfiguration(name, config);
        updatePluginDescriptor(name);
    }
}

QStringList PluginRegistry::getPluginErrors(const QString& name) const {
    // Return empty list for now - in a real implementation,
    // you'd store errors in the plugin descriptor
    Q_UNUSED(name)
    return QStringList();
}

void PluginRegistry::clearPluginErrors(const QString& name) {
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                          [&name](PluginDescriptor& desc) {
                              return desc.name == name;
                          });

    if (it != m_plugins.end()) {
        it->errorCount = 0;
        updatePluginDescriptor(name);
    }
}

// **Property setters**
void PluginRegistry::setFilterText(const QString& text) {
    if (m_filterText != text) {
        m_filterText = text;
        applyFilter();
        emit filterTextChanged();
    }
}

void PluginRegistry::setSortRole(const QString& role) {
    if (m_sortRole != role) {
        m_sortRole = role;
        beginResetModel();
        applySorting();
        endResetModel();
        emit sortRoleChanged();
    }
}

void PluginRegistry::setSortOrder(Qt::SortOrder order) {
    if (m_sortOrder != order) {
        m_sortOrder = order;
        beginResetModel();
        applySorting();
        endResetModel();
        emit sortOrderChanged();
    }
}

// **Private helper methods**
void PluginRegistry::updatePluginDescriptor(const QString& name) {
    if (!m_pluginManager) return;

    // Find existing descriptor or create new one
    auto it = std::find_if(m_plugins.begin(), m_plugins.end(),
                          [&name](const PluginDescriptor& desc) {
                              return desc.name == name;
                          });

    PluginDescriptor descriptor;
    descriptor.name = name;

    // Get plugin information from manager
    if (m_pluginManager->isPluginLoaded(name)) {
        descriptor.enabled = true;
        descriptor.status = "Loaded";

        // Get metrics if available
        QJsonObject metrics = m_pluginManager->getPluginMetrics(name);
        if (!metrics.isEmpty()) {
            descriptor.memoryUsage = metrics.value("memoryUsage").toInt();
            descriptor.cpuUsage = metrics.value("cpuUsage").toDouble();
        }

        // Get configuration if available
        QJsonObject config = m_pluginManager->loadPluginConfiguration(name);
        if (!config.isEmpty()) {
            descriptor.description = config.value("description").toString();
            descriptor.author = config.value("author").toString();
            descriptor.type = config.value("type").toString();
            descriptor.version = QVersionNumber::fromString(config.value("version").toString());
            descriptor.capabilities = config.value("capabilities").toVariant().toStringList();
        }
    } else {
        descriptor.enabled = false;
        descriptor.status = "Unloaded";
    }

    descriptor.loadTime = QDateTime::currentDateTime();
    descriptor.uuid = QUuid::createUuid();

    if (it != m_plugins.end()) {
        *it = descriptor;
    } else {
        m_plugins.append(descriptor);
    }

    applyFilter();
}