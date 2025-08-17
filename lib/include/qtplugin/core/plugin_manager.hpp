/**
 * @file plugin_manager.hpp
 * @brief Enhanced plugin manager using modern C++ features
 * @version 3.0.0
 */

#pragma once

#include "plugin_interface.hpp"
#include "plugin_loader.hpp"
#include "../communication/message_bus.hpp"
#include "../security/security_manager.hpp"
#include "../utils/error_handling.hpp"
#include "../utils/concepts.hpp"
#include <QObject>
#include <QString>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <future>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>

namespace qtplugin {

// Forward declarations
class IPluginLoader;
class IMessageBus;
class ISecurityManager;

/**
 * @brief Plugin loading options
 */
struct PluginLoadOptions {
    bool validate_signature = true;        ///< Validate plugin signature
    bool check_dependencies = true;        ///< Check plugin dependencies
    bool initialize_immediately = true;    ///< Initialize plugin after loading
    bool enable_hot_reload = false;        ///< Enable hot reloading for this plugin
    SecurityLevel security_level = SecurityLevel::Basic;  ///< Security level to apply
    std::chrono::milliseconds timeout = std::chrono::seconds{30};  ///< Loading timeout
    QJsonObject configuration;             ///< Initial plugin configuration
};

/**
 * @brief Plugin information structure
 */
struct PluginInfo {
    std::string id;
    std::filesystem::path file_path;
    PluginMetadata metadata;
    PluginState state = PluginState::Unloaded;
    std::chrono::system_clock::time_point load_time;
    std::chrono::system_clock::time_point last_activity;
    std::shared_ptr<IPlugin> instance;
    std::unique_ptr<QPluginLoader> loader;
    QJsonObject configuration;
    std::vector<std::string> error_log;
    QJsonObject metrics;
    bool hot_reload_enabled = false;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin dependency graph node
 */
struct DependencyNode {
    std::string plugin_id;
    std::unordered_set<std::string> dependencies;
    std::unordered_set<std::string> dependents;
    int load_order = 0;
};

/**
 * @brief Enhanced plugin manager
 * 
 * This class provides comprehensive plugin management functionality including
 * loading, unloading, dependency resolution, hot reloading, and monitoring.
 */
class PluginManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Constructor with dependency injection
     * @param loader Custom plugin loader (optional)
     * @param message_bus Custom message bus (optional)
     * @param security_manager Custom security manager (optional)
     * @param parent Parent QObject (optional)
     */
    explicit PluginManager(std::unique_ptr<IPluginLoader> loader = nullptr,
                          std::unique_ptr<IMessageBus> message_bus = nullptr,
                          std::unique_ptr<ISecurityManager> security_manager = nullptr,
                          QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~PluginManager() override;
    
    // === Plugin Loading ===
    
    /**
     * @brief Load a plugin from file
     * @param file_path Path to the plugin file
     * @param options Loading options
     * @return Plugin ID or error information
     */
    qtplugin::expected<std::string, PluginError>
    load_plugin(const std::filesystem::path& file_path,
               const PluginLoadOptions& options = {});

    /**
     * @brief Load plugin asynchronously
     * @param file_path Path to the plugin file
     * @param options Loading options
     * @return Future with plugin ID or error information
     */
    std::future<qtplugin::expected<std::string, PluginError>>
    load_plugin_async(const std::filesystem::path& file_path,
                     const PluginLoadOptions& options = {});

    /**
     * @brief Unload a plugin
     * @param plugin_id Plugin identifier
     * @param force Force unload even if other plugins depend on it
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> unload_plugin(std::string_view plugin_id,
                                                        bool force = false);

    /**
     * @brief Reload a plugin
     * @param plugin_id Plugin identifier
     * @param preserve_state Whether to preserve plugin state
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> reload_plugin(std::string_view plugin_id,
                                                        bool preserve_state = true);
    
    // === Plugin Discovery ===
    
    /**
     * @brief Discover plugins in a directory
     * @param directory Directory to search
     * @param recursive Whether to search recursively
     * @return Vector of discovered plugin file paths
     */
    std::vector<std::filesystem::path> discover_plugins(const std::filesystem::path& directory,
                                                       bool recursive = false) const;
    
    /**
     * @brief Add plugin search path
     * @param path Path to add to search paths
     */
    void add_search_path(const std::filesystem::path& path);
    
    /**
     * @brief Remove plugin search path
     * @param path Path to remove from search paths
     */
    void remove_search_path(const std::filesystem::path& path);
    
    /**
     * @brief Get all plugin search paths
     * @return Vector of search paths
     */
    std::vector<std::filesystem::path> search_paths() const;
    
    /**
     * @brief Load all plugins from search paths
     * @param options Loading options to apply to all plugins
     * @return Number of successfully loaded plugins
     */
    int load_all_plugins(const PluginLoadOptions& options = {});
    
    // === Plugin Access ===
    
    /**
     * @brief Get plugin by ID
     * @param plugin_id Plugin identifier
     * @return Shared pointer to plugin, or nullptr if not found
     */
    std::shared_ptr<IPlugin> get_plugin(std::string_view plugin_id) const;
    
    /**
     * @brief Get plugin with specific interface type
     * @tparam PluginType Plugin interface type
     * @param plugin_id Plugin identifier
     * @return Shared pointer to plugin with specified type, or nullptr if not found or wrong type
     */
    template<concepts::Plugin PluginType>
    std::shared_ptr<PluginType> get_plugin(std::string_view plugin_id) const {
        auto plugin = get_plugin(plugin_id);
        return std::dynamic_pointer_cast<PluginType>(plugin);
    }
    
    /**
     * @brief Get all loaded plugins
     * @return Vector of plugin IDs
     */
    std::vector<std::string> loaded_plugins() const;
    
    /**
     * @brief Get plugins by capability
     * @param capability Capability to filter by
     * @return Vector of plugin IDs that have the specified capability
     */
    std::vector<std::string> plugins_with_capability(PluginCapability capability) const;
    
    /**
     * @brief Get plugins by category
     * @param category Category to filter by
     * @return Vector of plugin IDs in the specified category
     */
    std::vector<std::string> plugins_in_category(std::string_view category) const;
    
    /**
     * @brief Get plugin information
     * @param plugin_id Plugin identifier
     * @return Plugin information, or nullopt if not found
     */
    std::optional<PluginInfo> get_plugin_info(std::string_view plugin_id) const;
    
    /**
     * @brief Get all plugin information
     * @return Vector of plugin information for all loaded plugins
     */
    std::vector<PluginInfo> all_plugin_info() const;
    
    // === Plugin State Management ===
    
    /**
     * @brief Initialize all loaded plugins
     * @return Number of successfully initialized plugins
     */
    int initialize_all_plugins();
    
    /**
     * @brief Shutdown all plugins
     */
    void shutdown_all_plugins();
    
    /**
     * @brief Start all service plugins
     * @return Number of successfully started services
     */
    int start_all_services();
    
    /**
     * @brief Stop all service plugins
     * @return Number of successfully stopped services
     */
    int stop_all_services();
    
    // === Dependency Management ===
    
    /**
     * @brief Resolve plugin dependencies
     * @return Success or error information with details about unresolved dependencies
     */
    qtplugin::expected<void, PluginError> resolve_dependencies();
    
    /**
     * @brief Get dependency graph
     * @return Map of plugin IDs to their dependency information
     */
    std::unordered_map<std::string, DependencyNode> dependency_graph() const;
    
    /**
     * @brief Get load order for plugins based on dependencies
     * @return Vector of plugin IDs in load order
     */
    std::vector<std::string> get_load_order() const;
    
    /**
     * @brief Check if plugin can be safely unloaded
     * @param plugin_id Plugin identifier
     * @return true if plugin can be unloaded without breaking dependencies
     */
    bool can_unload_safely(std::string_view plugin_id) const;
    
    // === Hot Reloading ===
    
    /**
     * @brief Enable hot reloading for a plugin
     * @param plugin_id Plugin identifier
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> enable_hot_reload(std::string_view plugin_id);

    /**
     * @brief Disable hot reloading for a plugin
     * @param plugin_id Plugin identifier
     */
    void disable_hot_reload(std::string_view plugin_id);
    
    /**
     * @brief Check if hot reloading is enabled for a plugin
     * @param plugin_id Plugin identifier
     * @return true if hot reloading is enabled
     */
    bool is_hot_reload_enabled(std::string_view plugin_id) const;
    
    /**
     * @brief Enable global hot reloading
     * @param watch_directories Directories to watch for changes
     */
    void enable_global_hot_reload(const std::vector<std::filesystem::path>& watch_directories = {});
    
    /**
     * @brief Disable global hot reloading
     */
    void disable_global_hot_reload();
    
    // === Configuration Management ===
    
    /**
     * @brief Configure a plugin
     * @param plugin_id Plugin identifier
     * @param configuration Configuration data
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> configure_plugin(std::string_view plugin_id,
                                                           const QJsonObject& configuration);
    
    /**
     * @brief Get plugin configuration
     * @param plugin_id Plugin identifier
     * @return Plugin configuration, or empty object if not found
     */
    QJsonObject get_plugin_configuration(std::string_view plugin_id) const;
    
    /**
     * @brief Save all plugin configurations
     * @param file_path File to save configurations to
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> save_configurations(const std::filesystem::path& file_path) const;

    /**
     * @brief Load plugin configurations
     * @param file_path File to load configurations from
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> load_configurations(const std::filesystem::path& file_path);
    
    // === Communication ===
    
    /**
     * @brief Send command to a plugin
     * @param plugin_id Plugin identifier
     * @param command Command name
     * @param parameters Command parameters
     * @return Command result or error information
     */
    qtplugin::expected<QJsonObject, PluginError> send_command(std::string_view plugin_id,
                                                              std::string_view command,
                                                              const QJsonObject& parameters = {});
    
    /**
     * @brief Broadcast message to all plugins
     * @tparam MessageType Type of message to broadcast
     * @param message Message to broadcast
     */
    template<typename MessageType>
    void broadcast_message(const MessageType& message);
    
    /**
     * @brief Get message bus
     * @return Reference to the message bus
     */
    IMessageBus& message_bus() const { return *m_message_bus; }
    
    // === Monitoring and Metrics ===
    
    /**
     * @brief Get system metrics
     * @return System-wide plugin metrics
     */
    QJsonObject system_metrics() const;
    
    /**
     * @brief Get plugin metrics
     * @param plugin_id Plugin identifier
     * @return Plugin-specific metrics
     */
    QJsonObject plugin_metrics(std::string_view plugin_id) const;
    
    /**
     * @brief Start performance monitoring
     * @param interval Monitoring interval
     */
    void start_monitoring(std::chrono::milliseconds interval = std::chrono::seconds{60});
    
    /**
     * @brief Stop performance monitoring
     */
    void stop_monitoring();
    
    /**
     * @brief Check if monitoring is active
     * @return true if monitoring is running
     */
    bool is_monitoring_active() const noexcept { return m_monitoring_active; }
    
    // === Security ===
    
    /**
     * @brief Set global security level
     * @param level Security level to apply
     */
    void set_security_level(SecurityLevel level);
    
    /**
     * @brief Get current security level
     * @return Current security level
     */
    SecurityLevel security_level() const;
    
    /**
     * @brief Validate plugin security
     * @param plugin_id Plugin identifier
     * @return Success or error information with security validation details
     */
    qtplugin::expected<void, PluginError> validate_plugin_security(std::string_view plugin_id) const;
    
signals:
    /**
     * @brief Emitted when a plugin is loaded
     * @param plugin_id Plugin identifier
     */
    void plugin_loaded(const QString& plugin_id);
    
    /**
     * @brief Emitted when a plugin is unloaded
     * @param plugin_id Plugin identifier
     */
    void plugin_unloaded(const QString& plugin_id);
    
    /**
     * @brief Emitted when a plugin state changes
     * @param plugin_id Plugin identifier
     * @param old_state Previous state
     * @param new_state New state
     */
    void plugin_state_changed(const QString& plugin_id, PluginState old_state, PluginState new_state);
    
    /**
     * @brief Emitted when a plugin error occurs
     * @param plugin_id Plugin identifier
     * @param error Error information
     */
    void plugin_error(const QString& plugin_id, const QString& error);
    
    /**
     * @brief Emitted when plugin metrics are updated
     * @param plugin_id Plugin identifier
     * @param metrics Updated metrics
     */
    void plugin_metrics_updated(const QString& plugin_id, const QJsonObject& metrics);

private slots:
    void on_file_changed(const QString& path);
    void on_monitoring_timer();

private:
    // Core components
    std::unique_ptr<IPluginLoader> m_loader;
    std::unique_ptr<IMessageBus> m_message_bus;
    std::unique_ptr<ISecurityManager> m_security_manager;
    
    // Plugin storage
    mutable std::shared_mutex m_plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<PluginInfo>> m_plugins;
    std::unordered_map<std::string, DependencyNode> m_dependency_graph;
    
    // Search paths
    mutable std::shared_mutex m_search_paths_mutex;
    std::unordered_set<std::filesystem::path> m_search_paths;
    
    // Hot reloading
    std::unique_ptr<QFileSystemWatcher> m_file_watcher;
    std::unordered_map<std::string, std::filesystem::path> m_watched_files;
    
    // Monitoring
    std::atomic<bool> m_monitoring_active{false};
    std::unique_ptr<QTimer> m_monitoring_timer;
    
    // Security
    SecurityLevel m_security_level = SecurityLevel::Basic;
    
    // Helper methods
    qtplugin::expected<void, PluginError> validate_plugin_file(const std::filesystem::path& file_path) const;
    qtplugin::expected<void, PluginError> check_plugin_dependencies(const PluginInfo& info) const;
    void update_dependency_graph();
    std::vector<std::string> topological_sort() const;
    void cleanup_plugin(const std::string& plugin_id);
    void update_plugin_metrics(const std::string& plugin_id);
};

} // namespace qtplugin
