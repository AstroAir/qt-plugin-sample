/**
 * @file plugin_loader.hpp
 * @brief Plugin loader interface and implementation
 * @version 3.0.0
 */

#pragma once

#include "plugin_interface.hpp"
#include "../utils/error_handling.hpp"
#include "../utils/concepts.hpp"
#include <QPluginLoader>
#include <QJsonObject>
#include <memory>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <shared_mutex>
#include <mutex>
#include <unordered_map>
#include <functional>

namespace qtplugin {

/**
 * @brief Plugin loader interface
 */
class IPluginLoader {
public:
    virtual ~IPluginLoader() = default;
    
    /**
     * @brief Check if a file can be loaded as a plugin
     * @param file_path Path to the plugin file
     * @return true if the file can be loaded
     */
    virtual bool can_load(const std::filesystem::path& file_path) const = 0;
    
    /**
     * @brief Load a plugin from file
     * @param file_path Path to the plugin file
     * @return Plugin instance or error information
     */
    virtual qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load(const std::filesystem::path& file_path) = 0;

    /**
     * @brief Unload a plugin
     * @param plugin_id Plugin identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unload(std::string_view plugin_id) = 0;
    
    /**
     * @brief Get supported file extensions
     * @return Vector of supported extensions (including the dot)
     */
    virtual std::vector<std::string> supported_extensions() const = 0;
    
    /**
     * @brief Get loader name/type
     * @return Loader identifier
     */
    virtual std::string_view name() const noexcept = 0;
    
    /**
     * @brief Check if loader supports hot reloading
     * @return true if hot reloading is supported
     */
    virtual bool supports_hot_reload() const noexcept = 0;
};

/**
 * @brief Default Qt plugin loader implementation
 */
class QtPluginLoader : public IPluginLoader {
public:
    QtPluginLoader();
    ~QtPluginLoader() override;
    
    // IPluginLoader implementation
    bool can_load(const std::filesystem::path& file_path) const override;
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
    load(const std::filesystem::path& file_path) override;
    qtplugin::expected<void, PluginError> unload(std::string_view plugin_id) override;
    std::vector<std::string> supported_extensions() const override;
    std::string_view name() const noexcept override;
    bool supports_hot_reload() const noexcept override;
    
    /**
     * @brief Get loaded plugin count
     * @return Number of currently loaded plugins
     */
    size_t loaded_plugin_count() const;
    
    /**
     * @brief Get loaded plugin IDs
     * @return Vector of loaded plugin identifiers
     */
    std::vector<std::string> loaded_plugins() const;
    
    /**
     * @brief Check if a plugin is loaded
     * @param plugin_id Plugin identifier
     * @return true if plugin is loaded
     */
    bool is_loaded(std::string_view plugin_id) const;

private:
    struct LoadedPlugin {
        std::string id;
        std::filesystem::path file_path;
        std::unique_ptr<QPluginLoader> qt_loader;
        std::shared_ptr<IPlugin> instance;
    };
    
    std::unordered_map<std::string, std::unique_ptr<LoadedPlugin>> m_loaded_plugins;
    mutable std::shared_mutex m_plugins_mutex;
    
    // Helper methods
    qtplugin::expected<QJsonObject, PluginError> read_metadata(const std::filesystem::path& file_path) const;
    qtplugin::expected<std::string, PluginError> extract_plugin_id(const QJsonObject& metadata) const;
    bool is_valid_plugin_file(const std::filesystem::path& file_path) const;
};

/**
 * @brief Plugin loader factory
 */
class PluginLoaderFactory {
public:
    /**
     * @brief Create default plugin loader
     * @return Unique pointer to plugin loader
     */
    static std::unique_ptr<IPluginLoader> create_default_loader();
    
    /**
     * @brief Create Qt plugin loader
     * @return Unique pointer to Qt plugin loader
     */
    static std::unique_ptr<QtPluginLoader> create_qt_loader();
    
    /**
     * @brief Register custom loader type
     * @param name Loader type name
     * @param factory Factory function for creating the loader
     */
    static void register_loader_type(std::string_view name, 
                                   std::function<std::unique_ptr<IPluginLoader>()> factory);
    
    /**
     * @brief Create loader by name
     * @param name Loader type name
     * @return Unique pointer to plugin loader, or nullptr if not found
     */
    static std::unique_ptr<IPluginLoader> create_loader(std::string_view name);
    
    /**
     * @brief Get available loader types
     * @return Vector of available loader type names
     */
    static std::vector<std::string> available_loaders();

private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<IPluginLoader>()>> s_loader_factories;
    static std::mutex s_factory_mutex;
};

} // namespace qtplugin
