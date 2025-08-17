/**
 * @file plugin_manager.cpp
 * @brief Implementation of enhanced plugin manager
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_manager.hpp"
#include "../../include/qtplugin/core/plugin_loader.hpp"
#include "../../include/qtplugin/communication/message_bus.hpp"
#include "../../include/qtplugin/security/security_manager.hpp"
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <algorithm>
#include <fstream>

namespace qtplugin {

QJsonObject PluginInfo::to_json() const {
    QJsonObject json;
    json["id"] = QString::fromStdString(id);
    json["file_path"] = QString::fromStdString(file_path.string());
    json["state"] = static_cast<int>(state);
    json["load_time"] = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
        load_time.time_since_epoch()).count());
    json["last_activity"] = QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
        last_activity.time_since_epoch()).count());
    json["hot_reload_enabled"] = hot_reload_enabled;
    json["configuration"] = configuration;
    json["metrics"] = metrics;
    
    // Add metadata
    if (instance) {
        auto plugin_metadata = instance->metadata();
        json["metadata"] = plugin_metadata.to_json();
    }
    
    // Add error log
    QJsonArray errors_array;
    for (const auto& error : error_log) {
        errors_array.append(QString::fromStdString(error));
    }
    json["error_log"] = errors_array;
    
    return json;
}

PluginManager::PluginManager(std::unique_ptr<IPluginLoader> loader,
                           std::unique_ptr<IMessageBus> message_bus,
                           std::unique_ptr<ISecurityManager> security_manager,
                           QObject* parent)
    : QObject(parent)
    , m_loader(loader ? std::move(loader) : PluginLoaderFactory::create_default_loader())
    , m_message_bus(message_bus ? std::move(message_bus) : std::make_unique<MessageBus>())
    , m_security_manager(security_manager ? std::move(security_manager) : SecurityManagerFactory::create_default())
    , m_file_watcher(std::make_unique<QFileSystemWatcher>(this))
    , m_monitoring_timer(std::make_unique<QTimer>(this))
{
    // Connect file watcher
    connect(m_file_watcher.get(), &QFileSystemWatcher::fileChanged,
            this, &PluginManager::on_file_changed);
    
    // Connect monitoring timer
    connect(m_monitoring_timer.get(), &QTimer::timeout,
            this, &PluginManager::on_monitoring_timer);
}

PluginManager::~PluginManager() {
    shutdown_all_plugins();
}

qtplugin::expected<std::string, PluginError>
PluginManager::load_plugin(const std::filesystem::path& file_path, 
                          const PluginLoadOptions& options) {
    // Validate plugin file
    auto validation_result = validate_plugin_file(file_path);
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>{validation_result.error()};
    }
    
    // Security validation
    if (options.validate_signature) {
        auto security_result = m_security_manager->validate_plugin(file_path, options.security_level);
        if (!security_result.is_valid) {
            std::string error_msg = "Security validation failed: ";
            for (const auto& error : security_result.errors) {
                error_msg += error + "; ";
            }
            return make_error<std::string>(PluginErrorCode::SecurityViolation, error_msg);
        }
    }
    
    // Load the plugin
    auto plugin_result = m_loader->load(file_path);
    if (!plugin_result) {
        return qtplugin::unexpected<PluginError>{plugin_result.error()};
    }
    
    auto plugin = plugin_result.value();
    std::string plugin_id = plugin->id();
    
    // Check if already loaded
    {
        std::shared_lock lock(m_plugins_mutex);
        if (m_plugins.find(plugin_id) != m_plugins.end()) {
            return make_error<std::string>(PluginErrorCode::LoadFailed, "Plugin already loaded: " + plugin_id);
        }
    }
    
    // Create plugin info
    auto plugin_info = std::make_unique<PluginInfo>();
    plugin_info->id = plugin_id;
    plugin_info->file_path = file_path;
    plugin_info->metadata = plugin->metadata();
    plugin_info->state = PluginState::Loaded;
    plugin_info->load_time = std::chrono::system_clock::now();
    plugin_info->last_activity = plugin_info->load_time;
    plugin_info->instance = plugin;
    plugin_info->configuration = options.configuration;
    plugin_info->hot_reload_enabled = options.enable_hot_reload;
    
    // Check dependencies if requested
    if (options.check_dependencies) {
        auto dep_result = check_plugin_dependencies(*plugin_info);
        if (!dep_result) {
            return qtplugin::unexpected<PluginError>{dep_result.error()};
        }
    }
    
    // Configure plugin if configuration provided
    if (!options.configuration.isEmpty()) {
        auto config_result = plugin->configure(options.configuration);
        if (!config_result) {
            return qtplugin::unexpected<PluginError>{config_result.error()};
        }
    }
    
    // Initialize plugin if requested
    if (options.initialize_immediately) {
        plugin_info->state = PluginState::Initializing;
        auto init_result = plugin->initialize();
        if (!init_result) {
            plugin_info->state = PluginState::Error;
            plugin_info->error_log.push_back(init_result.error().message);
            return qtplugin::unexpected<PluginError>{init_result.error()};
        }
        plugin_info->state = PluginState::Running;
    }
    
    // Enable hot reload if requested
    if (options.enable_hot_reload) {
        enable_hot_reload(plugin_id);
    }
    
    // Store plugin info
    {
        std::unique_lock lock(m_plugins_mutex);
        m_plugins[plugin_id] = std::move(plugin_info);
    }
    
    // Update dependency graph
    update_dependency_graph();
    
    emit plugin_loaded(QString::fromStdString(plugin_id));
    
    return plugin_id;
}

std::future<qtplugin::expected<std::string, PluginError>>
PluginManager::load_plugin_async(const std::filesystem::path& file_path,
                                const PluginLoadOptions& options) {
    return std::async(std::launch::async, [this, file_path, options]() {
        return load_plugin(file_path, options);
    });
}

qtplugin::expected<void, PluginError> PluginManager::unload_plugin(std::string_view plugin_id, bool force) {
    std::unique_lock lock(m_plugins_mutex);
    
    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed, "Plugin not found: " + std::string(plugin_id));
    }
    
    auto& plugin_info = it->second;
    
    // Check if plugin can be safely unloaded
    if (!force && !can_unload_safely(plugin_id)) {
        return make_error<void>(PluginErrorCode::DependencyMissing, 
                               "Plugin has dependents and cannot be safely unloaded");
    }
    
    // Shutdown plugin if running
    if (plugin_info->instance && plugin_info->state == PluginState::Running) {
        plugin_info->state = PluginState::Stopping;
        plugin_info->instance->shutdown();
        plugin_info->state = PluginState::Stopped;
    }
    
    // Disable hot reload
    disable_hot_reload(plugin_id);
    
    // Unload from loader
    auto unload_result = m_loader->unload(plugin_id);
    if (!unload_result) {
        return unload_result;
    }
    
    // Remove from plugins map
    m_plugins.erase(it);
    
    // Update dependency graph
    update_dependency_graph();
    
    emit plugin_unloaded(QString::fromStdString(std::string(plugin_id)));
    
    return make_success();
}

std::shared_ptr<IPlugin> PluginManager::get_plugin(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    
    auto it = m_plugins.find(std::string(plugin_id));
    if (it != m_plugins.end()) {
        return it->second->instance;
    }
    
    return nullptr;
}

std::vector<std::string> PluginManager::loaded_plugins() const {
    std::shared_lock lock(m_plugins_mutex);
    std::vector<std::string> plugin_ids;
    plugin_ids.reserve(m_plugins.size());
    
    for (const auto& [id, info] : m_plugins) {
        plugin_ids.push_back(id);
    }
    
    return plugin_ids;
}

std::vector<std::filesystem::path> PluginManager::discover_plugins(const std::filesystem::path& directory,
                                                                  bool recursive) const {
    std::vector<std::filesystem::path> discovered_plugins;
    
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return discovered_plugins;
    }
    
    auto extensions = m_loader->supported_extensions();
    
    try {
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file() && m_loader->can_load(entry.path())) {
                    discovered_plugins.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file() && m_loader->can_load(entry.path())) {
                    discovered_plugins.push_back(entry.path());
                }
            }
        }

    } catch (const std::filesystem::filesystem_error&) {
        // Ignore filesystem errors during discovery
    }
    
    return discovered_plugins;
}

void PluginManager::add_search_path(const std::filesystem::path& path) {
    std::unique_lock lock(m_search_paths_mutex);
    m_search_paths.insert(path);
}

void PluginManager::remove_search_path(const std::filesystem::path& path) {
    std::unique_lock lock(m_search_paths_mutex);
    m_search_paths.erase(path);
}

std::vector<std::filesystem::path> PluginManager::search_paths() const {
    std::shared_lock lock(m_search_paths_mutex);
    return std::vector<std::filesystem::path>(m_search_paths.begin(), m_search_paths.end());
}

int PluginManager::load_all_plugins(const PluginLoadOptions& options) {
    int loaded_count = 0;
    
    auto paths = search_paths();
    for (const auto& search_path : paths) {
        auto discovered = discover_plugins(search_path, true);
        for (const auto& plugin_path : discovered) {
            auto result = load_plugin(plugin_path, options);
            if (result) {
                ++loaded_count;
            }
        }
    }
    
    return loaded_count;
}

void PluginManager::on_file_changed(const QString& path) {
    std::string file_path = path.toStdString();
    
    // Find plugin by file path
    std::shared_lock lock(m_plugins_mutex);
    for (const auto& [id, info] : m_plugins) {
        if (info->file_path.string() == file_path && info->hot_reload_enabled) {
            // Reload plugin asynchronously
            std::async(std::launch::async, [this, id]() {
                reload_plugin(id, true);
            });
            break;
        }
    }
}

void PluginManager::on_monitoring_timer() {
    std::shared_lock lock(m_plugins_mutex);
    for (const auto& [id, info] : m_plugins) {
        update_plugin_metrics(id);
    }
}

qtplugin::expected<void, PluginError> PluginManager::validate_plugin_file(const std::filesystem::path& file_path) const {
    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(PluginErrorCode::FileNotFound, "Plugin file not found");
    }
    
    if (!m_loader->can_load(file_path)) {
        return make_error<void>(PluginErrorCode::InvalidFormat, "Invalid plugin file format");
    }
    
    return make_success();
}

qtplugin::expected<void, PluginError> PluginManager::check_plugin_dependencies(const PluginInfo& info) const {
    // This is a simplified implementation
    // In a real system, you would check if all dependencies are loaded and compatible
    return make_success();
}

void PluginManager::update_dependency_graph() {
    // This is a placeholder for dependency graph updates
    // In a real implementation, you would analyze plugin dependencies
    // and create a proper dependency graph
}

void PluginManager::update_plugin_metrics(const std::string& plugin_id) {
    // This is a placeholder for metrics updates
    // In a real implementation, you would collect performance metrics
    Q_UNUSED(plugin_id)
}

// === Missing Method Implementations ===

QJsonObject PluginManager::system_metrics() const {
    std::shared_lock lock(m_plugins_mutex);

    QJsonObject metrics;
    metrics["total_plugins"] = static_cast<int>(m_plugins.size());
    metrics["loaded_plugins"] = static_cast<int>(m_plugins.size());
    metrics["failed_plugins"] = 0; // TODO: Track failed plugins
    metrics["memory_usage"] = 0; // TODO: Calculate memory usage
    metrics["uptime"] = 0; // TODO: Track uptime

    return metrics;
}

void PluginManager::shutdown_all_plugins() {
    std::unique_lock lock(m_plugins_mutex);

    // Shutdown all plugins (order doesn't matter for shutdown)
    for (auto& [id, info] : m_plugins) {
        if (info && info->instance) {
            try {
                info->instance->shutdown();
            } catch (...) {
                // Log error but continue shutdown
            }
        }
    }

    m_plugins.clear();
}

qtplugin::expected<void, PluginError> PluginManager::enable_hot_reload(std::string_view plugin_id) {
    std::unique_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed, "Plugin not found");
    }

    // Add to file watcher if not already watching
    if (m_file_watcher && it->second && !it->second->file_path.empty()) {
        m_file_watcher->addPath(QString::fromStdString(it->second->file_path.string()));
        it->second->hot_reload_enabled = true;
    }

    return make_success();
}

bool PluginManager::can_unload_safely(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);

    // Check if any other plugins depend on this one
    for (const auto& [id, info] : m_plugins) {
        if (id != plugin_id && info) {
            // Check metadata dependencies
            for (const auto& dep : info->metadata.dependencies) {
                if (dep == plugin_id) {
                    return false; // Another plugin depends on this one
                }
            }
        }
    }

    return true;
}

void PluginManager::disable_hot_reload(std::string_view plugin_id) {
    std::unique_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(std::string(plugin_id));
    if (it != m_plugins.end() && it->second) {
        if (m_file_watcher && !it->second->file_path.empty()) {
            m_file_watcher->removePath(QString::fromStdString(it->second->file_path.string()));
        }
        it->second->hot_reload_enabled = false;
    }
}

qtplugin::expected<void, PluginError> PluginManager::reload_plugin(std::string_view plugin_id, bool preserve_state) {
    std::unique_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed, "Plugin not found");
    }

    if (!it->second) {
        return make_error<void>(PluginErrorCode::LoadFailed, "Plugin info is null");
    }

    // Save state if requested
    QJsonObject saved_state;
    if (preserve_state && it->second->instance) {
        // TODO: Implement state preservation
    }

    // Unload current plugin
    if (it->second->instance) {
        it->second->instance->shutdown();
    }

    // Reload plugin
    auto plugin_result = m_loader->load(it->second->file_path);
    if (!plugin_result) {
        return make_error<void>(plugin_result.error().code, "Failed to reload plugin");
    }

    it->second->instance = plugin_result.value();

    // Initialize plugin
    auto init_result = it->second->instance->initialize();
    if (!init_result) {
        return make_error<void>(init_result.error().code, "Failed to initialize reloaded plugin");
    }

    // Restore state if requested
    if (preserve_state && !saved_state.isEmpty()) {
        // TODO: Implement state restoration
    }

    return make_success();
}

qtplugin::expected<void, PluginError> PluginManager::configure_plugin(std::string_view plugin_id,
                                                                       const QJsonObject& configuration) {
    std::unique_lock lock(m_plugins_mutex);
    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return make_error<void>(PluginErrorCode::StateError, "Plugin not found");
    }

    // Store configuration
    it->second->configuration = configuration;

    // Apply configuration to plugin if it's loaded
    if (it->second->instance) {
        auto result = it->second->instance->configure(configuration);
        if (!result) {
            return make_error<void>(result.error().code, "Failed to configure plugin");
        }
    }

    return make_success();
}

QJsonObject PluginManager::plugin_metrics(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return QJsonObject();
    }

    return it->second->metrics;
}

void PluginManager::start_monitoring(std::chrono::milliseconds interval) {
    if (m_monitoring_active) {
        return;
    }

    m_monitoring_active = true;

    if (!m_monitoring_timer) {
        m_monitoring_timer = std::make_unique<QTimer>(this);
        connect(m_monitoring_timer.get(), &QTimer::timeout, this, &PluginManager::on_monitoring_timer);
    }

    m_monitoring_timer->start(static_cast<int>(interval.count()));
}

std::optional<PluginInfo> PluginManager::get_plugin_info(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return std::nullopt;
    }

    // Create a copy without the unique_ptr members
    PluginInfo info;
    info.id = it->second->id;
    info.file_path = it->second->file_path;
    info.metadata = it->second->metadata;
    info.state = it->second->state;
    info.load_time = it->second->load_time;
    info.last_activity = it->second->last_activity;
    info.instance = it->second->instance;
    // Skip loader (unique_ptr)
    info.configuration = it->second->configuration;
    info.error_log = it->second->error_log;
    info.metrics = it->second->metrics;
    info.hot_reload_enabled = it->second->hot_reload_enabled;

    return info;
}

QJsonObject PluginManager::get_plugin_configuration(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    auto it = m_plugins.find(std::string(plugin_id));
    if (it == m_plugins.end()) {
        return QJsonObject();
    }

    return it->second->configuration;
}

} // namespace qtplugin
