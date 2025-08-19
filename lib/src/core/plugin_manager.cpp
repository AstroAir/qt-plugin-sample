/**
 * @file plugin_manager.cpp
 * @brief Implementation of enhanced plugin manager
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_manager.hpp"
#include "../../include/qtplugin/core/plugin_loader.hpp"
#include "../../include/qtplugin/core/plugin_registry.hpp"
#include "../../include/qtplugin/core/plugin_dependency_resolver.hpp"
#include "../../include/qtplugin/monitoring/plugin_hot_reload_manager.hpp"
#include "../../include/qtplugin/monitoring/plugin_metrics_collector.hpp"
#include "../../include/qtplugin/core/service_plugin_interface.hpp"
#include "../../include/qtplugin/communication/message_bus.hpp"
#include "../../include/qtplugin/security/security_manager.hpp"
#include "../../include/qtplugin/managers/configuration_manager_impl.hpp"
#include "../../include/qtplugin/managers/logging_manager_impl.hpp"
#include "../../include/qtplugin/managers/resource_manager_impl.hpp"
#include "../../include/qtplugin/managers/resource_lifecycle_impl.hpp"
#include "../../include/qtplugin/managers/resource_monitor_impl.hpp"
#include <QTimer>
#include <QFileSystemWatcher>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QLoggingCategory>
#include <QDebug>
#include <algorithm>
#include <fstream>

Q_LOGGING_CATEGORY(pluginLog, "qtplugin.manager")

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
                           std::unique_ptr<IConfigurationManager> configuration_manager,
                           std::unique_ptr<ILoggingManager> logging_manager,
                           std::unique_ptr<IResourceManager> resource_manager,
                           std::unique_ptr<IResourceLifecycleManager> resource_lifecycle_manager,
                           std::unique_ptr<IResourceMonitor> resource_monitor,
                           std::unique_ptr<IPluginRegistry> plugin_registry,
                           std::unique_ptr<IPluginDependencyResolver> dependency_resolver,
                           std::unique_ptr<IPluginHotReloadManager> hot_reload_manager,
                           std::unique_ptr<IPluginMetricsCollector> metrics_collector,
                           QObject* parent)
    : QObject(parent)
    , m_loader(loader ? std::move(loader) : PluginLoaderFactory::create_default_loader())
    , m_message_bus(message_bus ? std::move(message_bus) : std::make_unique<MessageBus>())
    , m_security_manager(security_manager ? std::move(security_manager) : SecurityManagerFactory::create_default())
    , m_configuration_manager(configuration_manager ? std::move(configuration_manager) : create_configuration_manager(this))
    , m_logging_manager(logging_manager ? std::move(logging_manager) : create_logging_manager(this))
    , m_resource_manager(resource_manager ? std::move(resource_manager) : create_resource_manager(this))
    , m_resource_lifecycle_manager(resource_lifecycle_manager ? std::move(resource_lifecycle_manager) : create_resource_lifecycle_manager(this))
    , m_resource_monitor(resource_monitor ? std::move(resource_monitor) : create_resource_monitor(this))
    , m_plugin_registry(plugin_registry ? std::move(plugin_registry) : std::make_unique<PluginRegistry>(this))
    , m_dependency_resolver(dependency_resolver ? std::move(dependency_resolver) : std::make_unique<PluginDependencyResolver>(this))
    , m_hot_reload_manager(hot_reload_manager ? std::move(hot_reload_manager) : std::make_unique<PluginHotReloadManager>(this))
    , m_metrics_collector(metrics_collector ? std::move(metrics_collector) : std::make_unique<PluginMetricsCollector>(this))
    , m_monitoring_timer(std::make_unique<QTimer>(this))
{
    // Set up hot reload callback
    m_hot_reload_manager->set_reload_callback([this](const std::string& plugin_id) {
        reload_plugin(plugin_id, true);
    });

    // Connect monitoring timer (legacy - will be removed)
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
    if (m_plugin_registry->is_plugin_registered(plugin_id)) {
        return make_error<std::string>(PluginErrorCode::LoadFailed, "Plugin already loaded: " + plugin_id);
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
        auto dep_result = m_dependency_resolver->check_plugin_dependencies(*plugin_info);
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
        m_hot_reload_manager->enable_hot_reload(plugin_id, file_path);
    }
    
    // Store plugin info in registry
    auto register_result = m_plugin_registry->register_plugin(plugin_id, std::move(plugin_info));
    if (!register_result) {
        return qtplugin::unexpected<PluginError>{register_result.error()};
    }
    
    // Update dependency graph
    m_dependency_resolver->update_dependency_graph(m_plugin_registry.get());
    
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
    // Get plugin info from registry
    auto plugin_info_opt = m_plugin_registry->get_plugin_info(std::string(plugin_id));
    if (!plugin_info_opt) {
        return make_error<void>(PluginErrorCode::LoadFailed, "Plugin not found: " + std::string(plugin_id));
    }

    auto plugin_info = plugin_info_opt.value();
    
    // Check if plugin can be safely unloaded
    if (!force && !m_dependency_resolver->can_unload_safely(std::string(plugin_id))) {
        return make_error<void>(PluginErrorCode::DependencyMissing,
                               "Plugin has dependents and cannot be safely unloaded");
    }
    
    // Shutdown plugin if running
    if (plugin_info.instance && plugin_info.state == PluginState::Running) {
        plugin_info.instance->shutdown();
    }
    
    // Disable hot reload
    m_hot_reload_manager->disable_hot_reload(std::string(plugin_id));
    
    // Unload from loader
    auto unload_result = m_loader->unload(plugin_id);
    if (!unload_result) {
        return unload_result;
    }
    
    // Remove from registry
    auto unregister_result = m_plugin_registry->unregister_plugin(std::string(plugin_id));
    if (!unregister_result) {
        return unregister_result;
    }
    
    // Update dependency graph
    m_dependency_resolver->update_dependency_graph(m_plugin_registry.get());
    
    emit plugin_unloaded(QString::fromStdString(std::string(plugin_id)));
    
    return make_success();
}

std::shared_ptr<IPlugin> PluginManager::get_plugin(std::string_view plugin_id) const {
    return m_plugin_registry->get_plugin(std::string(plugin_id));
}

std::vector<std::string> PluginManager::loaded_plugins() const {
    return m_plugin_registry->get_all_plugin_ids();
}

std::vector<PluginInfo> PluginManager::all_plugin_info() const {
    return m_plugin_registry->get_all_plugin_info();
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
            auto future = std::async(std::launch::async, [this, id]() {
                reload_plugin(id, true);
            });
            // Future will be destroyed when it goes out of scope, which is fine for fire-and-forget
            (void)future; // Suppress unused variable warning
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
    (void)info; // Suppress unused parameter warning
    return make_success();
}

void PluginManager::update_dependency_graph() {
    std::unique_lock lock(m_plugins_mutex);

    // Clear existing dependency graph
    m_dependency_graph.clear();

    // Build dependency graph from loaded plugins
    for (const auto& [plugin_id, plugin_info] : m_plugins) {
        if (!plugin_info || !plugin_info->instance) {
            continue;
        }

        DependencyNode node;
        node.plugin_id = plugin_id;

        // Convert vector to unordered_set
        for (const auto& dep : plugin_info->metadata.dependencies) {
            node.dependencies.insert(dep);
        }
        node.dependents.clear();

        // Set load order based on dependency count (will be refined later)
        node.load_order = static_cast<int>(plugin_info->metadata.dependencies.size());

        m_dependency_graph[plugin_id] = std::move(node);
    }

    // Build reverse dependencies (dependents)
    for (auto& [plugin_id, node] : m_dependency_graph) {
        for (const auto& dependency : node.dependencies) {
            auto dep_it = m_dependency_graph.find(dependency);
            if (dep_it != m_dependency_graph.end()) {
                dep_it->second.dependents.insert(plugin_id);
            }
        }
    }

    // Validate for circular dependencies
    detect_circular_dependencies();
}

void PluginManager::update_plugin_metrics(const std::string& plugin_id) {
    std::unique_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(plugin_id);
    if (it == m_plugins.end() || !it->second || !it->second->instance) {
        return;
    }

    auto& plugin_info = it->second;
    auto& metrics = plugin_info->metrics;

    // Update basic metrics
    auto now = std::chrono::system_clock::now();
    plugin_info->last_activity = now;

    // Calculate uptime
    auto uptime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - plugin_info->load_time).count();
    metrics["uptime_ms"] = static_cast<qint64>(uptime_ms);

    // Update activity timestamp
    metrics["last_activity"] = QString::number(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count());

    // Get plugin-specific metrics if available
    try {
        if (plugin_info->instance->capabilities() & static_cast<PluginCapabilities>(PluginCapability::Monitoring)) {
            // Try to get metrics from plugin
            auto plugin_metrics_result = plugin_info->instance->execute_command("get_metrics");
            if (plugin_metrics_result) {
                metrics["plugin_metrics"] = plugin_metrics_result.value();
            }
        }
    } catch (...) {
        // Ignore errors in metrics collection
    }

    // Update error count
    metrics["error_count"] = static_cast<int>(plugin_info->error_log.size());

    // Update state information
    metrics["state"] = static_cast<int>(plugin_info->state);
    metrics["state_name"] = QString::fromStdString(plugin_state_to_string(plugin_info->state));
}

// === Missing Method Implementations ===

QJsonObject PluginManager::system_metrics() const {
    std::shared_lock lock(m_plugins_mutex);

    QJsonObject metrics;

    // Count plugins by state
    int total_plugins = 0;
    int loaded_plugins = 0;
    int failed_plugins = 0;
    int unloaded_plugins = 0;
    int initializing_plugins = 0;

    for (const auto& [id, plugin_info] : m_plugins) {
        total_plugins++;
        if (plugin_info) {
            switch (plugin_info->state) {
                case PluginState::Loaded:
                case PluginState::Running:
                    loaded_plugins++;
                    break;
                case PluginState::Error:
                    failed_plugins++;
                    break;
                case PluginState::Unloaded:
                case PluginState::Stopped:
                    unloaded_plugins++;
                    break;
                case PluginState::Initializing:
                case PluginState::Loading:
                    initializing_plugins++;
                    break;
                case PluginState::Paused:
                case PluginState::Stopping:
                case PluginState::Reloading:
                    // Count as loaded but not fully operational
                    loaded_plugins++;
                    break;
            }
        }
    }

    metrics["total_plugins"] = total_plugins;
    metrics["loaded_plugins"] = loaded_plugins;
    metrics["failed_plugins"] = failed_plugins;
    metrics["unloaded_plugins"] = unloaded_plugins;
    metrics["initializing_plugins"] = initializing_plugins;

    // Calculate memory usage (basic estimation)
    size_t estimated_memory = 0;
    for (const auto& [id, plugin_info] : m_plugins) {
        if (plugin_info) {
            // Basic estimation: plugin info + metadata + configuration
            estimated_memory += sizeof(PluginInfo);
            estimated_memory += plugin_info->metadata.name.size();
            estimated_memory += plugin_info->metadata.description.size();
            estimated_memory += plugin_info->error_log.size() * 100; // Rough estimate
        }
    }
    metrics["estimated_memory_bytes"] = static_cast<qint64>(estimated_memory);

    // System uptime (time since first plugin was loaded)
    if (!m_plugins.empty()) {
        auto earliest_load_time = std::chrono::system_clock::now();
        for (const auto& [id, plugin_info] : m_plugins) {
            if (plugin_info && plugin_info->load_time < earliest_load_time) {
                earliest_load_time = plugin_info->load_time;
            }
        }

        auto uptime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - earliest_load_time).count();
        metrics["system_uptime_ms"] = static_cast<qint64>(uptime_ms);
    } else {
        metrics["system_uptime_ms"] = 0;
    }

    // Monitoring status
    metrics["monitoring_active"] = m_monitoring_active.load();

    // Security level
    metrics["security_level"] = static_cast<int>(m_security_level);

    // Dependency graph stats
    metrics["dependency_nodes"] = static_cast<int>(m_dependency_graph.size());

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

int PluginManager::start_all_services() {
    std::shared_lock lock(m_plugins_mutex);
    int started_count = 0;

    for (auto& [id, info] : m_plugins) {
        if (info && info->instance) {
            // Check if plugin has Service capability
            auto capabilities = info->metadata.capabilities;
            if (capabilities & static_cast<uint32_t>(PluginCapability::Service)) {
                // Try to cast to service plugin and start it
                auto service_plugin = std::dynamic_pointer_cast<IServicePlugin>(info->instance);
                if (service_plugin) {
                    try {
                        auto result = service_plugin->start_service();
                        if (result) {
                            started_count++;
                        }
                    } catch (...) {
                        // Log error but continue with other services
                    }
                }
            }
        }
    }

    return started_count;
}

int PluginManager::stop_all_services() {
    std::shared_lock lock(m_plugins_mutex);
    int stopped_count = 0;

    for (auto& [id, info] : m_plugins) {
        if (info && info->instance) {
            // Check if plugin has Service capability
            auto capabilities = info->metadata.capabilities;
            if (capabilities & static_cast<uint32_t>(PluginCapability::Service)) {
                // Try to cast to service plugin and stop it
                auto service_plugin = std::dynamic_pointer_cast<IServicePlugin>(info->instance);
                if (service_plugin) {
                    try {
                        auto result = service_plugin->stop_service();
                        if (result) {
                            stopped_count++;
                        }
                    } catch (...) {
                        // Log error but continue with other services
                    }
                }
            }
        }
    }

    return stopped_count;
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
        try {
            // Try to get state from plugin using standard command
            auto state_result = it->second->instance->execute_command("save_state");
            if (state_result) {
                saved_state = state_result.value();
            } else {
                // Fallback: save current configuration as state
                saved_state = it->second->configuration;
                saved_state["_fallback_state"] = true;
            }

            // Also save plugin metrics and runtime information
            saved_state["_runtime_info"] = QJsonObject{
                {"load_time", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                    it->second->load_time.time_since_epoch()).count())},
                {"last_activity", QString::number(std::chrono::duration_cast<std::chrono::milliseconds>(
                    it->second->last_activity.time_since_epoch()).count())},
                {"error_count", static_cast<int>(it->second->error_log.size())}
            };
        } catch (...) {
            qCWarning(pluginLog) << "Failed to save state for plugin:"
                                << QString::fromStdString(std::string(plugin_id));
        }
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
        try {
            // Check if this was a fallback state save
            bool is_fallback = saved_state.contains("_fallback_state") &&
                             saved_state["_fallback_state"].toBool();

            if (is_fallback) {
                // Restore configuration
                QJsonObject config = saved_state;
                config.remove("_fallback_state");
                config.remove("_runtime_info");

                auto config_result = it->second->instance->configure(config);
                if (!config_result) {
                    qCWarning(pluginLog) << "Failed to restore configuration for plugin:"
                                        << QString::fromStdString(std::string(plugin_id));
                }
            } else {
                // Try to restore state using standard command
                auto restore_result = it->second->instance->execute_command("restore_state", saved_state);
                if (!restore_result) {
                    qCWarning(pluginLog) << "Failed to restore state for plugin:"
                                        << QString::fromStdString(std::string(plugin_id));

                    // Fallback: try to restore as configuration
                    auto config_result = it->second->instance->configure(saved_state);
                    if (!config_result) {
                        qCWarning(pluginLog) << "Failed to restore state as configuration for plugin:"
                                            << QString::fromStdString(std::string(plugin_id));
                    }
                }
            }

            // Update plugin info with restored state
            it->second->configuration = saved_state;

        } catch (...) {
            qCWarning(pluginLog) << "Exception during state restoration for plugin:"
                                << QString::fromStdString(std::string(plugin_id));
        }
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

IConfigurationManager& PluginManager::configuration_manager() const {
    return *m_configuration_manager;
}

ILoggingManager& PluginManager::logging_manager() const {
    return *m_logging_manager;
}

IResourceManager& PluginManager::resource_manager() const {
    return *m_resource_manager;
}

IResourceLifecycleManager& PluginManager::resource_lifecycle_manager() const {
    return *m_resource_lifecycle_manager;
}

IResourceMonitor& PluginManager::resource_monitor() const {
    return *m_resource_monitor;
}

// === Helper Methods ===

int PluginManager::calculate_dependency_level(const std::string& plugin_id,
                                            const std::vector<std::string>& dependencies) const {
    if (dependencies.empty()) {
        return 0;
    }

    int max_level = 0;
    for (const auto& dep : dependencies) {
        auto it = m_plugins.find(dep);
        if (it != m_plugins.end() && it->second) {
            int dep_level = calculate_dependency_level(dep, it->second->metadata.dependencies);
            max_level = std::max(max_level, dep_level + 1);
        }
    }

    return max_level;
}

void PluginManager::detect_circular_dependencies() const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;

    for (const auto& [plugin_id, node] : m_dependency_graph) {
        if (visited.find(plugin_id) == visited.end()) {
            if (has_circular_dependency(plugin_id, visited, recursion_stack)) {
                qCWarning(pluginLog) << "Circular dependency detected involving plugin:"
                                    << QString::fromStdString(plugin_id);
            }
        }
    }
}

bool PluginManager::has_circular_dependency(const std::string& plugin_id,
                                          std::unordered_set<std::string>& visited,
                                          std::unordered_set<std::string>& recursion_stack) const {
    visited.insert(plugin_id);
    recursion_stack.insert(plugin_id);

    auto it = m_dependency_graph.find(plugin_id);
    if (it != m_dependency_graph.end()) {
        for (const auto& dep : it->second.dependencies) {
            if (recursion_stack.find(dep) != recursion_stack.end()) {
                return true; // Circular dependency found
            }

            if (visited.find(dep) == visited.end()) {
                if (has_circular_dependency(dep, visited, recursion_stack)) {
                    return true;
                }
            }
        }
    }

    recursion_stack.erase(plugin_id);
    return false;
}

std::string PluginManager::plugin_state_to_string(PluginState state) const {
    switch (state) {
        case PluginState::Unloaded: return "Unloaded";
        case PluginState::Loading: return "Loading";
        case PluginState::Loaded: return "Loaded";
        case PluginState::Initializing: return "Initializing";
        case PluginState::Running: return "Running";
        case PluginState::Stopping: return "Stopping";
        case PluginState::Error: return "Error";
        default: return "Unknown";
    }
}

} // namespace qtplugin
