/**
 * @file plugin_composition.cpp
 * @brief Implementation of plugin composition system
 * @version 3.1.0
 */

#include "qtplugin/composition/plugin_composition.hpp"
#include "qtplugin/core/plugin_manager.hpp"
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>
#include <future>

Q_LOGGING_CATEGORY(compositionLog, "qtplugin.composition")

namespace qtplugin::composition {

// === PluginComposition Implementation ===

qtplugin::expected<void, PluginError> PluginComposition::validate() const {
    if (m_id.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration, "Composition ID cannot be empty");
    }
    
    if (m_plugins.empty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration, "Composition must have at least one plugin");
    }
    
    // Validate primary plugin
    if (!m_primary_plugin_id.isEmpty()) {
        auto it = m_plugins.find(m_primary_plugin_id);
        if (it == m_plugins.end() || it->second != PluginRole::Primary) {
            return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                   "Primary plugin not found or not marked as primary");
        }
    }
    
    // Validate bindings
    for (const auto& binding : m_bindings) {
        if (m_plugins.find(binding.source_plugin_id) == m_plugins.end()) {
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                   "Binding source plugin not found: " + binding.source_plugin_id.toStdString());
        }
        
        if (m_plugins.find(binding.target_plugin_id) == m_plugins.end()) {
            return make_error<void>(PluginErrorCode::DependencyMissing,
                                   "Binding target plugin not found: " + binding.target_plugin_id.toStdString());
        }
    }
    
    return make_success();
}

QJsonObject PluginComposition::to_json() const {
    QJsonObject json;
    json["id"] = m_id;
    json["name"] = m_name;
    json["description"] = m_description;
    json["strategy"] = static_cast<int>(m_strategy);
    json["primary_plugin_id"] = m_primary_plugin_id;
    json["configuration"] = m_configuration;
    
    // Serialize plugins
    QJsonObject plugins_json;
    for (const auto& [plugin_id, role] : m_plugins) {
        plugins_json[plugin_id] = static_cast<int>(role);
    }
    json["plugins"] = plugins_json;
    
    // Serialize bindings
    QJsonArray bindings_json;
    for (const auto& binding : m_bindings) {
        QJsonObject binding_json;
        binding_json["source_plugin_id"] = binding.source_plugin_id;
        binding_json["source_method"] = binding.source_method;
        binding_json["target_plugin_id"] = binding.target_plugin_id;
        binding_json["target_method"] = binding.target_method;
        binding_json["parameter_mapping"] = binding.parameter_mapping;
        binding_json["bidirectional"] = binding.bidirectional;
        binding_json["priority"] = binding.priority;
        bindings_json.append(binding_json);
    }
    json["bindings"] = bindings_json;
    
    return json;
}

qtplugin::expected<PluginComposition, PluginError> PluginComposition::from_json(const QJsonObject& json) {
    if (!json.contains("id") || !json["id"].isString()) {
        return make_error<PluginComposition>(PluginErrorCode::InvalidConfiguration, "Missing composition ID");
    }
    
    QString id = json["id"].toString();
    QString name = json.value("name").toString(id);
    
    PluginComposition composition(id, name);
    composition.set_description(json.value("description").toString());
    composition.set_strategy(static_cast<CompositionStrategy>(json.value("strategy").toInt()));
    composition.set_configuration(json.value("configuration").toObject());
    
    QString primary_plugin_id = json.value("primary_plugin_id").toString();
    if (!primary_plugin_id.isEmpty()) {
        composition.set_primary_plugin(primary_plugin_id);
    }
    
    // Parse plugins
    if (json.contains("plugins") && json["plugins"].isObject()) {
        QJsonObject plugins_json = json["plugins"].toObject();
        for (auto it = plugins_json.begin(); it != plugins_json.end(); ++it) {
            const QString& plugin_id = it.key();
            PluginRole role = static_cast<PluginRole>(it.value().toInt());
            composition.add_plugin(plugin_id, role);
        }
    }
    
    // Parse bindings
    if (json.contains("bindings") && json["bindings"].isArray()) {
        QJsonArray bindings_json = json["bindings"].toArray();
        for (const auto& binding_value : bindings_json) {
            QJsonObject binding_json = binding_value.toObject();
            
            CompositionBinding binding;
            binding.source_plugin_id = binding_json["source_plugin_id"].toString();
            binding.source_method = binding_json["source_method"].toString();
            binding.target_plugin_id = binding_json["target_plugin_id"].toString();
            binding.target_method = binding_json["target_method"].toString();
            binding.parameter_mapping = binding_json["parameter_mapping"].toObject();
            binding.bidirectional = binding_json.value("bidirectional").toBool(false);
            binding.priority = binding_json.value("priority").toInt(0);
            
            composition.add_binding(binding);
        }
    }
    
    // Validate the composition
    auto validation_result = composition.validate();
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }
    
    return composition;
}

// === CompositePlugin Implementation ===

CompositePlugin::CompositePlugin(const PluginComposition& composition, QObject* parent)
    : QObject(parent), m_composition(composition) {
    
    m_id = composition.id().toStdString();
    m_name = composition.name().toStdString();
    m_description = composition.description().toStdString();
    m_configuration = composition.configuration();
    
    qCDebug(compositionLog) << "Created composite plugin:" << QString::fromStdString(m_id);
}

CompositePlugin::~CompositePlugin() {
    if (m_state != PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> CompositePlugin::initialize() {
    if (m_state != PluginState::Unloaded) {
        return make_error<void>(PluginErrorCode::InvalidState, "Plugin already initialized");
    }
    
    m_state = PluginState::Loading;
    
    // Load component plugins
    auto load_result = load_component_plugins();
    if (!load_result) {
        m_state = PluginState::Error;
        return load_result;
    }
    
    // Initialize component plugins
    for (auto& [plugin_id, plugin] : m_component_plugins) {
        auto init_result = plugin->initialize();
        if (!init_result) {
            qCWarning(compositionLog) << "Failed to initialize component plugin:" << plugin_id;
            m_state = PluginState::Error;
            return init_result;
        }
    }
    
    // Setup bindings
    auto binding_result = setup_bindings();
    if (!binding_result) {
        m_state = PluginState::Error;
        return binding_result;
    }
    
    // Calculate combined capabilities
    m_capabilities = 0;
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        m_capabilities |= plugin->capabilities();
    }
    
    m_state = PluginState::Running;
    
    qCDebug(compositionLog) << "Composite plugin initialized:" << QString::fromStdString(m_id);
    
    return make_success();
}

void CompositePlugin::shutdown() noexcept {
    if (m_state == PluginState::Unloaded) {
        return;
    }
    
    try {
        m_state = PluginState::Stopping;
        
        // Shutdown component plugins in reverse order
        for (auto it = m_component_plugins.rbegin(); it != m_component_plugins.rend(); ++it) {
            try {
                it->second->shutdown();
            } catch (const std::exception& e) {
                qCWarning(compositionLog) << "Exception during component plugin shutdown:" << e.what();
            }
        }
        
        m_component_plugins.clear();
        m_active_bindings.clear();
        
        m_state = PluginState::Unloaded;
        
        qCDebug(compositionLog) << "Composite plugin shutdown:" << QString::fromStdString(m_id);
    } catch (const std::exception& e) {
        qCWarning(compositionLog) << "Exception during composite plugin shutdown:" << e.what();
        m_state = PluginState::Error;
    } catch (...) {
        qCWarning(compositionLog) << "Unknown exception during composite plugin shutdown";
        m_state = PluginState::Error;
    }
}

qtplugin::expected<void, PluginError> CompositePlugin::configure(const QJsonObject& config) {
    m_configuration = config;
    
    // Configure component plugins
    for (auto& [plugin_id, plugin] : m_component_plugins) {
        if (config.contains(plugin_id)) {
            auto plugin_config = config[plugin_id].toObject();
            auto config_result = plugin->configure(plugin_config);
            if (!config_result) {
                qCWarning(compositionLog) << "Failed to configure component plugin:" << plugin_id;
                return config_result;
            }
        }
    }
    
    return make_success();
}

PluginMetadata CompositePlugin::metadata() const {
    PluginMetadata meta;
    meta.id = m_id;
    meta.name = m_name;
    meta.description = m_description;
    meta.version = m_version;
    meta.author = m_author;
    meta.plugin_type = "composite";
    meta.capabilities = m_capabilities;

    // Add component plugin information as custom data
    QJsonObject custom_data;
    custom_data["strategy"] = static_cast<int>(m_composition.strategy());

    QJsonArray components;
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        QJsonObject component_info;
        component_info["id"] = plugin_id;
        component_info["name"] = QString::fromStdString(std::string(plugin->name()));
        component_info["state"] = static_cast<int>(plugin->state());
        components.append(component_info);
    }
    custom_data["components"] = components;
    meta.custom_data = custom_data;

    return meta;
}

qtplugin::expected<QJsonObject, PluginError> CompositePlugin::execute_command(
    std::string_view command, 
    const QJsonObject& params) {
    
    if (m_state != PluginState::Running) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState, "Plugin not running");
    }
    
    // Execute based on composition strategy
    switch (m_composition.strategy()) {
        case CompositionStrategy::Aggregation:
            return execute_aggregation_command(command, params);
        case CompositionStrategy::Pipeline:
            return execute_pipeline_command(command, params);
        case CompositionStrategy::Facade:
            return execute_facade_command(command, params);
        default:
            return execute_aggregation_command(command, params);
    }
}

std::vector<std::string> CompositePlugin::available_commands() const {
    std::set<std::string> all_commands;
    
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        auto commands = plugin->available_commands();
        all_commands.insert(commands.begin(), commands.end());
    }
    
    return std::vector<std::string>(all_commands.begin(), all_commands.end());
}

qtplugin::expected<void, PluginError> CompositePlugin::load_component_plugins() {
    auto* plugin_manager = PluginManager::instance();
    if (!plugin_manager) {
        return make_error<void>(PluginErrorCode::SystemError, "Plugin manager not available");
    }
    
    for (const auto& [plugin_id, role] : m_composition.plugins()) {
        auto plugin = plugin_manager->get_plugin(plugin_id.toStdString());
        if (!plugin) {
            return make_error<void>(PluginErrorCode::PluginNotFound,
                                   "Component plugin not found: " + plugin_id.toStdString());
        }
        
        m_component_plugins[plugin_id] = plugin;
        
        qCDebug(compositionLog) << "Loaded component plugin:" << plugin_id 
                               << "role:" << static_cast<int>(role);
    }
    
    return make_success();
}

qtplugin::expected<void, PluginError> CompositePlugin::setup_bindings() {
    m_active_bindings = m_composition.bindings();

    // Sort bindings by priority
    std::sort(m_active_bindings.begin(), m_active_bindings.end(),
        [](const CompositionBinding& a, const CompositionBinding& b) {
            return a.priority > b.priority;
        });

    qCDebug(compositionLog) << "Setup" << m_active_bindings.size() << "bindings for composite plugin";

    return make_success();
}

qtplugin::expected<QJsonObject, PluginError> CompositePlugin::execute_aggregation_command(
    std::string_view command,
    const QJsonObject& params) {

    QJsonObject aggregated_result;
    bool any_success = false;
    QString last_error;

    // Execute command on all component plugins
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        auto commands = plugin->available_commands();
        if (std::find(commands.begin(), commands.end(), std::string(command)) != commands.end()) {
            auto result = plugin->execute_command(command, params);
            if (result) {
                aggregated_result[plugin_id] = result.value();
                any_success = true;
            } else {
                last_error = QString::fromStdString(result.error().message);
                qCWarning(compositionLog) << "Component plugin" << plugin_id
                                         << "failed to execute command:" << QString::fromStdString(std::string(command));
            }
        }
    }

    if (!any_success) {
        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed,
                                      "No component plugin could execute command: " + last_error.toStdString());
    }

    return aggregated_result;
}

qtplugin::expected<QJsonObject, PluginError> CompositePlugin::execute_pipeline_command(
    std::string_view command,
    const QJsonObject& params) {

    QJsonObject current_data = params;

    // Get plugins in execution order (primary first, then others)
    std::vector<QString> execution_order;

    // Add primary plugin first
    if (!m_composition.primary_plugin_id().isEmpty()) {
        execution_order.push_back(m_composition.primary_plugin_id());
    }

    // Add other plugins
    for (const auto& [plugin_id, role] : m_composition.plugins()) {
        if (plugin_id != m_composition.primary_plugin_id()) {
            execution_order.push_back(plugin_id);
        }
    }

    // Execute through pipeline
    for (const QString& plugin_id : execution_order) {
        auto plugin_it = m_component_plugins.find(plugin_id);
        if (plugin_it == m_component_plugins.end()) {
            continue;
        }

        auto plugin = plugin_it->second;
        auto commands = plugin->available_commands();

        if (std::find(commands.begin(), commands.end(), std::string(command)) != commands.end()) {
            auto result = plugin->execute_command(command, current_data);
            if (result) {
                current_data = result.value();
            } else {
                return qtplugin::unexpected<PluginError>(result.error());
            }
        }
    }

    return current_data;
}

qtplugin::expected<QJsonObject, PluginError> CompositePlugin::execute_facade_command(
    std::string_view command,
    const QJsonObject& params) {

    // Use primary plugin if available, otherwise first available plugin
    std::shared_ptr<IPlugin> target_plugin = find_primary_plugin();

    if (!target_plugin) {
        // Find first plugin that supports the command
        for (const auto& [plugin_id, plugin] : m_component_plugins) {
            auto commands = plugin->available_commands();
            if (std::find(commands.begin(), commands.end(), std::string(command)) != commands.end()) {
                target_plugin = plugin;
                break;
            }
        }
    }

    if (!target_plugin) {
        return make_error<QJsonObject>(PluginErrorCode::CommandNotFound,
                                      "No component plugin supports command: " + std::string(command));
    }

    return target_plugin->execute_command(command, params);
}

std::shared_ptr<IPlugin> CompositePlugin::find_primary_plugin() const {
    if (m_composition.primary_plugin_id().isEmpty()) {
        return nullptr;
    }

    auto it = m_component_plugins.find(m_composition.primary_plugin_id());
    return it != m_component_plugins.end() ? it->second : nullptr;
}

std::vector<contracts::ServiceContract> CompositePlugin::get_service_contracts() const {
    std::vector<contracts::ServiceContract> all_contracts;

    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        // Try to cast to enhanced plugin
        if (auto enhanced_plugin = std::dynamic_pointer_cast<IEnhancedPlugin>(plugin)) {
            auto contracts = enhanced_plugin->get_service_contracts();
            all_contracts.insert(all_contracts.end(), contracts.begin(), contracts.end());
        }
    }

    return all_contracts;
}

QJsonObject CompositePlugin::get_health_status() const {
    QJsonObject health;
    health["status"] = (m_state == PluginState::Running) ? "healthy" : "unhealthy";
    health["state"] = static_cast<int>(m_state);
    health["type"] = "composite";
    health["strategy"] = static_cast<int>(m_composition.strategy());

    // Add component health status
    QJsonArray components_health;
    for (const auto& [plugin_id, plugin] : m_component_plugins) {
        QJsonObject component_health;
        component_health["plugin_id"] = plugin_id;
        component_health["state"] = static_cast<int>(plugin->state());

        if (auto enhanced_plugin = std::dynamic_pointer_cast<IEnhancedPlugin>(plugin)) {
            component_health["health"] = enhanced_plugin->get_health_status();
        }

        components_health.append(component_health);
    }
    health["components"] = components_health;

    return health;
}

} // namespace qtplugin::composition

#include "plugin_composition.moc"
