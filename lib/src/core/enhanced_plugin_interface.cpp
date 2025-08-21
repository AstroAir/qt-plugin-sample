/**
 * @file enhanced_plugin_interface.cpp
 * @brief Implementation of enhanced plugin interface
 * @version 3.1.0
 */

#include "qtplugin/core/enhanced_plugin_interface.hpp"
#include "qtplugin/communication/request_response_system.hpp"
#include <QLoggingCategory>
#include <QDateTime>
#include <future>

Q_LOGGING_CATEGORY(enhancedPluginLog, "qtplugin.enhanced")

namespace qtplugin {

// === EnhancedPluginBase Implementation ===

EnhancedPluginBase::EnhancedPluginBase(QObject* parent)
    : QObject(parent), m_state(PluginState::Unloaded) {
    m_start_time = std::chrono::system_clock::now();
}

EnhancedPluginBase::~EnhancedPluginBase() {
    if (m_state != PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> EnhancedPluginBase::initialize() {
    if (m_state != PluginState::Unloaded) {
        return make_error<void>(PluginErrorCode::InvalidState, 
                               "Plugin already initialized");
    }
    
    set_state(PluginState::Loading);
    
    // Register services first
    auto register_result = register_services();
    if (!register_result) {
        set_state(PluginState::Error);
        return register_result;
    }
    
    // Initialize plugin-specific functionality
    auto init_result = do_initialize();
    if (!init_result) {
        set_state(PluginState::Error);
        unregister_services();
        return init_result;
    }
    
    set_state(PluginState::Running);
    
    qCDebug(enhancedPluginLog) << "Enhanced plugin initialized:" << QString::fromStdString(id());
    
    return make_success();
}

void EnhancedPluginBase::shutdown() noexcept {
    if (m_state == PluginState::Unloaded) {
        return;
    }
    
    try {
        set_state(PluginState::Stopping);
        
        // Unregister services
        unregister_services();
        
        // Shutdown plugin-specific functionality
        do_shutdown();
        
        set_state(PluginState::Unloaded);
        
        qCDebug(enhancedPluginLog) << "Enhanced plugin shutdown:" << QString::fromStdString(id());
    } catch (const std::exception& e) {
        qCWarning(enhancedPluginLog) << "Exception during plugin shutdown:" << e.what();
        set_state(PluginState::Error);
    } catch (...) {
        qCWarning(enhancedPluginLog) << "Unknown exception during plugin shutdown";
        set_state(PluginState::Error);
    }
}

qtplugin::expected<QJsonObject, PluginError> EnhancedPluginBase::call_service(
    const QString& service_name,
    const QString& method_name,
    const QJsonObject& parameters,
    std::chrono::milliseconds timeout) {
    
    // Get the service contract
    auto& registry = contracts::ServiceContractRegistry::instance();
    auto contract_result = registry.get_contract(service_name);
    if (!contract_result) {
        return qtplugin::unexpected<PluginError>(contract_result.error());
    }
    
    const auto& contract = contract_result.value();
    
    // Validate the method call
    auto validation_result = contract.validate_method_call(method_name, parameters);
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }
    
    // Find the provider
    auto provider_result = registry.find_provider(service_name);
    if (!provider_result) {
        return qtplugin::unexpected<PluginError>(provider_result.error());
    }
    
    const QString& provider_id = provider_result.value();
    
    // Use request-response system to make the call
    auto& request_system = communication::RequestResponseSystem::instance();
    
    communication::RequestInfo request;
    request.sender_id = QString::fromStdString(id());
    request.receiver_id = provider_id;
    request.method = method_name;
    request.parameters = parameters;
    request.timeout = timeout;
    request.type = communication::RequestType::Query;
    request.priority = communication::RequestPriority::Normal;
    
    auto response_result = request_system.send_request(request);
    if (!response_result) {
        return qtplugin::unexpected<PluginError>(response_result.error());
    }
    
    const auto& response = response_result.value();
    if (response.status != communication::ResponseStatus::Success) {
        return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed,
                                      "Service call failed: " + response.error_message.toStdString());
    }
    
    return response.data;
}

std::future<qtplugin::expected<QJsonObject, PluginError>> EnhancedPluginBase::call_service_async(
    const QString& service_name,
    const QString& method_name,
    const QJsonObject& parameters,
    std::chrono::milliseconds timeout) {
    
    return std::async(std::launch::async, [this, service_name, method_name, parameters, timeout]() {
        return call_service(service_name, method_name, parameters, timeout);
    });
}

qtplugin::expected<QJsonObject, PluginError> EnhancedPluginBase::handle_service_call(
    const QString& service_name,
    const QString& method_name,
    const QJsonObject& parameters) {
    
    // Find the service contract
    auto contracts = get_service_contracts();
    auto contract_it = std::find_if(contracts.begin(), contracts.end(),
        [&service_name](const contracts::ServiceContract& contract) {
            return contract.service_name() == service_name;
        });
    
    if (contract_it == contracts.end()) {
        return make_error<QJsonObject>(PluginErrorCode::CommandNotFound,
                                      "Service not provided: " + service_name.toStdString());
    }
    
    // Validate the method call
    auto validation_result = contract_it->validate_method_call(method_name, parameters);
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }
    
    // Delegate to the standard execute_command method
    return execute_command(method_name.toStdString(), parameters);
}

QJsonObject EnhancedPluginBase::get_health_status() const {
    QJsonObject health;
    health["status"] = (m_state == PluginState::Running) ? "healthy" : "unhealthy";
    health["state"] = static_cast<int>(m_state);
    
    // Calculate uptime
    auto now = std::chrono::system_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - m_start_time);
    health["uptime"] = static_cast<int>(uptime.count());
    
    health["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Add service information
    auto contracts = get_service_contracts();
    QJsonArray services;
    for (const auto& contract : contracts) {
        QJsonObject service_info;
        service_info["name"] = contract.service_name();
        service_info["version"] = QString::fromStdString(contract.version().to_string());
        service_info["methods"] = static_cast<int>(contract.methods().size());
        services.append(service_info);
    }
    health["services"] = services;
    
    return health;
}

} // namespace qtplugin

#include "enhanced_plugin_interface.moc"
