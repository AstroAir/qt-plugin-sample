/**
 * @file plugin_loader.cpp
 * @brief Implementation of plugin loader
 * @version 3.0.0
 */

#include <qtplugin/core/plugin_loader.hpp>
#include <QPluginLoader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>
#include <QLibrary>
#include <shared_mutex>
#include <unordered_map>
#include <mutex>

namespace qtplugin {

// Static members for PluginLoaderFactory
std::unordered_map<std::string, std::function<std::unique_ptr<IPluginLoader>()>> PluginLoaderFactory::s_loader_factories;
std::mutex PluginLoaderFactory::s_factory_mutex;

QtPluginLoader::QtPluginLoader() = default;

QtPluginLoader::~QtPluginLoader() {
    // Unload all plugins
    std::unique_lock lock(m_plugins_mutex);
    for (auto& [id, plugin] : m_loaded_plugins) {
        if (plugin->qt_loader && plugin->qt_loader->isLoaded()) {
            plugin->qt_loader->unload();
        }
    }
    m_loaded_plugins.clear();
}

bool QtPluginLoader::can_load(const std::filesystem::path& file_path) const {
    if (!is_valid_plugin_file(file_path)) {
        return false;
    }
    
    // Try to read metadata to verify it's a valid plugin
    auto metadata_result = read_metadata(file_path);
    return metadata_result.has_value();
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError>
QtPluginLoader::load(const std::filesystem::path& file_path) {
    if (!std::filesystem::exists(file_path)) {
        return make_error<std::shared_ptr<IPlugin>>(PluginErrorCode::FileNotFound, 
                                                   "Plugin file not found: " + file_path.string());
    }
    
    if (!can_load(file_path)) {
        return make_error<std::shared_ptr<IPlugin>>(PluginErrorCode::InvalidFormat,
                                                   "Invalid plugin file: " + file_path.string());
    }
    
    // Read metadata to get plugin ID
    auto metadata_result = read_metadata(file_path);
    if (!metadata_result) {
        return qtplugin::unexpected<PluginError>{metadata_result.error()};
    }
    
    auto plugin_id_result = extract_plugin_id(metadata_result.value());
    if (!plugin_id_result) {
        return qtplugin::unexpected<PluginError>{plugin_id_result.error()};
    }
    
    const std::string plugin_id = plugin_id_result.value();
    
    // Check if already loaded
    {
        std::shared_lock lock(m_plugins_mutex);
        if (m_loaded_plugins.find(plugin_id) != m_loaded_plugins.end()) {
            return make_error<std::shared_ptr<IPlugin>>(PluginErrorCode::LoadFailed,
                                                       "Plugin already loaded: " + plugin_id);
        }
    }
    
    // Create Qt plugin loader
    auto qt_loader = std::make_unique<QPluginLoader>(QString::fromStdString(file_path.string()));
    
    // Load the plugin
    QObject* instance = qt_loader->instance();
    if (!instance) {
        return make_error<std::shared_ptr<IPlugin>>(PluginErrorCode::LoadFailed,
                                                   "Failed to load plugin: " + qt_loader->errorString().toStdString());
    }
    
    // Cast to IPlugin interface
    IPlugin* plugin_interface = qobject_cast<IPlugin*>(instance);
    if (!plugin_interface) {
        qt_loader->unload();
        return make_error<std::shared_ptr<IPlugin>>(PluginErrorCode::LoadFailed,
                                                   "Plugin does not implement IPlugin interface");
    }
    
    // Create shared pointer with custom deleter that doesn't delete the object
    // (Qt manages the lifetime)
    std::shared_ptr<IPlugin> plugin_ptr(plugin_interface, [](IPlugin*) {
        // Don't delete - Qt manages the lifetime
    });
    
    // Create loaded plugin info
    auto loaded_plugin = std::make_unique<LoadedPlugin>();
    loaded_plugin->id = plugin_id;
    loaded_plugin->file_path = file_path;
    loaded_plugin->qt_loader = std::move(qt_loader);
    loaded_plugin->instance = plugin_ptr;
    
    // Store the loaded plugin
    {
        std::unique_lock lock(m_plugins_mutex);
        m_loaded_plugins[plugin_id] = std::move(loaded_plugin);
    }
    
    return plugin_ptr;
}

qtplugin::expected<void, PluginError> QtPluginLoader::unload(std::string_view plugin_id) {
    std::unique_lock lock(m_plugins_mutex);
    
    auto it = m_loaded_plugins.find(std::string(plugin_id));
    if (it == m_loaded_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed, "Plugin not found: " + std::string(plugin_id));
    }
    
    auto& loaded_plugin = it->second;
    
    // Unload the Qt plugin
    if (loaded_plugin->qt_loader && loaded_plugin->qt_loader->isLoaded()) {
        if (!loaded_plugin->qt_loader->unload()) {
            return make_error<void>(PluginErrorCode::LoadFailed,
                                   "Failed to unload plugin: " + loaded_plugin->qt_loader->errorString().toStdString());
        }
    }
    
    // Remove from loaded plugins
    m_loaded_plugins.erase(it);
    
    return make_success();
}

std::vector<std::string> QtPluginLoader::supported_extensions() const {
    return {".dll", ".so", ".dylib", ".qtplugin"};
}

std::string_view QtPluginLoader::name() const noexcept {
    return "QtPluginLoader";
}

bool QtPluginLoader::supports_hot_reload() const noexcept {
    return true;
}

size_t QtPluginLoader::loaded_plugin_count() const {
    std::shared_lock lock(m_plugins_mutex);
    return m_loaded_plugins.size();
}

std::vector<std::string> QtPluginLoader::loaded_plugins() const {
    std::shared_lock lock(m_plugins_mutex);
    std::vector<std::string> plugin_ids;
    plugin_ids.reserve(m_loaded_plugins.size());
    
    for (const auto& [id, plugin] : m_loaded_plugins) {
        plugin_ids.push_back(id);
    }
    
    return plugin_ids;
}

bool QtPluginLoader::is_loaded(std::string_view plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    return m_loaded_plugins.find(std::string(plugin_id)) != m_loaded_plugins.end();
}

qtplugin::expected<QJsonObject, PluginError> QtPluginLoader::read_metadata(const std::filesystem::path& file_path) const {
    QPluginLoader temp_loader(QString::fromStdString(file_path.string()));
    QJsonObject metadata = temp_loader.metaData();
    
    if (metadata.isEmpty()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidFormat, "No metadata found in plugin file");
    }
    
    return metadata;
}

qtplugin::expected<std::string, PluginError> QtPluginLoader::extract_plugin_id(const QJsonObject& metadata) const {
    // Try to get plugin ID from metadata
    if (metadata.contains("MetaData")) {
        QJsonObject meta_data = metadata["MetaData"].toObject();
        if (meta_data.contains("id") && meta_data["id"].isString()) {
            return meta_data["id"].toString().toStdString();
        }
        if (meta_data.contains("name") && meta_data["name"].isString()) {
            return meta_data["name"].toString().toStdString();
        }
    }
    
    // Fallback to IID
    if (metadata.contains("IID") && metadata["IID"].isString()) {
        return metadata["IID"].toString().toStdString();
    }
    
    return make_error<std::string>(PluginErrorCode::InvalidFormat, "No plugin ID found in metadata");
}

bool QtPluginLoader::is_valid_plugin_file(const std::filesystem::path& file_path) const {
    QFileInfo file_info(QString::fromStdString(file_path.string()));
    
    if (!file_info.exists() || !file_info.isFile()) {
        return false;
    }
    
    // Check file extension
    QString suffix = file_info.suffix().toLower();
    auto extensions = supported_extensions();
    
    for (const auto& ext : extensions) {
        QString ext_without_dot = QString::fromStdString(ext).mid(1); // Remove the dot
        if (suffix == ext_without_dot) {
            return true;
        }
    }
    
    return false;
}

// PluginLoaderFactory implementation

std::unique_ptr<IPluginLoader> PluginLoaderFactory::create_default_loader() {
    return std::make_unique<QtPluginLoader>();
}

std::unique_ptr<QtPluginLoader> PluginLoaderFactory::create_qt_loader() {
    return std::make_unique<QtPluginLoader>();
}

void PluginLoaderFactory::register_loader_type(std::string_view name, 
                                              std::function<std::unique_ptr<IPluginLoader>()> factory) {
    std::lock_guard lock(s_factory_mutex);
    s_loader_factories[std::string(name)] = std::move(factory);
}

std::unique_ptr<IPluginLoader> PluginLoaderFactory::create_loader(std::string_view name) {
    std::lock_guard lock(s_factory_mutex);
    auto it = s_loader_factories.find(std::string(name));
    if (it != s_loader_factories.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> PluginLoaderFactory::available_loaders() {
    std::lock_guard lock(s_factory_mutex);
    std::vector<std::string> loaders;
    loaders.reserve(s_loader_factories.size());
    
    for (const auto& [name, factory] : s_loader_factories) {
        loaders.push_back(name);
    }
    
    return loaders;
}

} // namespace qtplugin
