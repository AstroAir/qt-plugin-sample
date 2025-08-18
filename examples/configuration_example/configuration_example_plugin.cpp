/**
 * @file configuration_example_plugin.cpp
 * @brief Implementation of configuration example plugin
 * @version 3.0.0
 */

#include "configuration_example_plugin.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>


ConfigurationExamplePlugin::ConfigurationExamplePlugin(QObject* parent)
    : QObject(parent)
    , m_demo_timer(std::make_unique<QTimer>(this))
{
    qDebug() << "ConfigurationExamplePlugin: Created";
    
    // Set up demonstration timer
    m_demo_timer->setSingleShot(false);
    m_demo_timer->setInterval(3000); // 3 seconds between demonstrations
    connect(m_demo_timer.get(), &QTimer::timeout, this, &ConfigurationExamplePlugin::onDemonstrationTimer);
}

ConfigurationExamplePlugin::~ConfigurationExamplePlugin()
{
    qDebug() << "ConfigurationExamplePlugin: Destroyed";
}

std::string ConfigurationExamplePlugin::id() const noexcept
{
    return "configuration_example";
}

std::string_view ConfigurationExamplePlugin::name() const noexcept
{
    return "Configuration Example Plugin";
}

std::string_view ConfigurationExamplePlugin::description() const noexcept
{
    return "Demonstrates configuration management features including schemas, persistence, and scopes";
}

qtplugin::Version ConfigurationExamplePlugin::version() const noexcept
{
    return qtplugin::Version(1, 0, 0);
}

std::string_view ConfigurationExamplePlugin::author() const noexcept
{
    return "QtPlugin Example Team";
}

std::vector<std::string> ConfigurationExamplePlugin::dependencies() const
{
    return {};
}

qtplugin::expected<void, qtplugin::PluginError>
ConfigurationExamplePlugin::initialize()
{
    m_state = qtplugin::PluginState::Running;

    qDebug() << "ConfigurationExamplePlugin: Initialized";

    // Set up configuration schema
    setupConfigurationSchema();

    // Create default configuration
    createDefaultConfiguration();

    return qtplugin::make_success();
}

qtplugin::expected<void, qtplugin::PluginError> 
ConfigurationExamplePlugin::configure(const QJsonObject& config)
{
    qDebug() << "ConfigurationExamplePlugin: Configuring with:" << QJsonDocument(config).toJson(QJsonDocument::Compact);
    
    // Apply configuration settings
    if (config.contains("demo_interval")) {
        int interval = config["demo_interval"].toInt(3000);
        m_demo_timer->setInterval(interval);
        qDebug() << "ConfigurationExamplePlugin: Set demo interval to" << interval << "ms";
    }
    
    if (config.contains("auto_start_demo")) {
        bool autoStart = config["auto_start_demo"].toBool(true);
        if (autoStart && m_state == qtplugin::PluginState::Running) {
            m_demo_timer->start();
            qDebug() << "ConfigurationExamplePlugin: Auto-started demonstration";
        }
    }
    
    return qtplugin::make_success();
}

void ConfigurationExamplePlugin::shutdown() noexcept
{
    m_state = qtplugin::PluginState::Unloaded;

    // Stop demonstration timer
    if (m_demo_timer) {
        m_demo_timer->stop();
    }

    qDebug() << "ConfigurationExamplePlugin: Shutdown";
}

qtplugin::PluginState ConfigurationExamplePlugin::state() const noexcept
{
    return m_state;
}

qtplugin::PluginCapabilities ConfigurationExamplePlugin::capabilities() const noexcept
{
    return qtplugin::PluginCapability::Configuration | qtplugin::PluginCapability::Service;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
ConfigurationExamplePlugin::execute_command(std::string_view command, const QJsonObject& params)
{
    QJsonObject result;

    if (command == "demo_basic") {
        demonstrateBasicConfiguration();
        result["status"] = "Basic configuration demo executed";
    } else if (command == "demo_nested") {
        demonstrateNestedConfiguration();
        result["status"] = "Nested configuration demo executed";
    } else if (command == "demo_validation") {
        demonstrateConfigurationValidation();
        result["status"] = "Configuration validation demo executed";
    } else if (command == "demo_persistence") {
        demonstrateConfigurationPersistence();
        result["status"] = "Configuration persistence demo executed";
    } else if (command == "demo_scopes") {
        demonstrateConfigurationScopes();
        result["status"] = "Configuration scopes demo executed";
    } else if (command == "demo_notifications") {
        demonstrateConfigurationNotifications();
        result["status"] = "Configuration notifications demo executed";
    } else if (command == "get_status") {
        result["state"] = static_cast<int>(m_state);
        result["demo_step"] = m_demo_step;
        result["demo_timer_active"] = m_demo_timer ? m_demo_timer->isActive() : false;
        result["demo_interval"] = m_demo_timer ? m_demo_timer->interval() : 0;
    } else {
        return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound,
                                                "Unknown command: " + std::string(command));
    }

    return result;
}

std::vector<std::string> ConfigurationExamplePlugin::available_commands() const
{
    return {
        "demo_basic",
        "demo_nested",
        "demo_validation",
        "demo_persistence",
        "demo_scopes",
        "demo_notifications",
        "get_status"
    };
}

void ConfigurationExamplePlugin::demonstrateBasicConfiguration()
{
    qDebug() << "\n=== DEMONSTRATING BASIC CONFIGURATION ===";
    qDebug() << "This would demonstrate basic configuration operations";
    qDebug() << "Setting app_name, version, max_connections, debug_enabled";
    qDebug() << "Retrieving values and using defaults";
}

void ConfigurationExamplePlugin::demonstrateNestedConfiguration()
{
    qDebug() << "\n=== DEMONSTRATING NESTED CONFIGURATION ===";
    qDebug() << "This would demonstrate nested configuration structures";
    qDebug() << "Setting database.host, database.port, database.credentials.username";
    qDebug() << "Retrieving nested values and complete objects";
}

void ConfigurationExamplePlugin::onConfigurationChanged(const qtplugin::ConfigurationChangeEvent& event)
{
    Q_UNUSED(event)
    qDebug() << "Configuration changed event received";
}

void ConfigurationExamplePlugin::demonstrateConfigurationValidation()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION VALIDATION ===";
    qDebug() << "This would demonstrate configuration validation with schemas";
    qDebug() << "Testing valid and invalid configurations against schemas";
}

void ConfigurationExamplePlugin::demonstrateConfigurationPersistence()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION PERSISTENCE ===";
    qDebug() << "This would demonstrate configuration persistence";
    qDebug() << "Saving to and loading from files";
}

void ConfigurationExamplePlugin::demonstrateConfigurationScopes()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION SCOPES ===";
    qDebug() << "This would demonstrate different configuration scopes";
    qDebug() << "Global, User, and Plugin-specific scopes";
}

void ConfigurationExamplePlugin::demonstrateConfigurationNotifications()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION NOTIFICATIONS ===";
    qDebug() << "This would demonstrate configuration change notifications";
    qDebug() << "Subscribing to and receiving change events";
}

void ConfigurationExamplePlugin::onDemonstrationTimer()
{
    switch (m_demo_step % 6) {
        case 0:
            demonstrateBasicConfiguration();
            break;
        case 1:
            demonstrateNestedConfiguration();
            break;
        case 2:
            demonstrateConfigurationValidation();
            break;
        case 3:
            demonstrateConfigurationPersistence();
            break;
        case 4:
            demonstrateConfigurationScopes();
            break;
        case 5:
            demonstrateConfigurationNotifications();
            break;
    }

    m_demo_step++;

    // Stop after a few cycles to avoid spam
    if (m_demo_step >= 12) {
        m_demo_timer->stop();
        qDebug() << "\n=== CONFIGURATION DEMONSTRATION COMPLETE ===";
    }
}

void ConfigurationExamplePlugin::setupConfigurationSchema()
{
    qDebug() << "ConfigurationExamplePlugin: Setting up configuration schema";
}

void ConfigurationExamplePlugin::createDefaultConfiguration()
{
    qDebug() << "ConfigurationExamplePlugin: Creating default configuration";
}

void ConfigurationExamplePlugin::logConfigurationResult(const QString& operation, bool success, const QString& details)
{
    Q_UNUSED(details)
    QString status = success ? "SUCCESS" : "FAILED";
    qDebug() << "ConfigurationExamplePlugin:" << operation << "-" << status;
}

// Utility functions implementation
namespace ConfigurationExampleUtils {

QJsonObject createSampleSchema()
{
    QJsonObject schema;
    schema["type"] = "object";

    QJsonObject properties;

    // Name property
    QJsonObject nameProperty;
    nameProperty["type"] = "string";
    nameProperty["minLength"] = 1;
    nameProperty["maxLength"] = 100;
    properties["name"] = nameProperty;

    // Age property
    QJsonObject ageProperty;
    ageProperty["type"] = "number";
    ageProperty["minimum"] = 0;
    ageProperty["maximum"] = 150;
    properties["age"] = ageProperty;

    // Email property
    QJsonObject emailProperty;
    emailProperty["type"] = "string";
    emailProperty["pattern"] = "^[\\w\\.-]+@[\\w\\.-]+\\.[a-zA-Z]{2,}$";
    properties["email"] = emailProperty;

    // Active property
    QJsonObject activeProperty;
    activeProperty["type"] = "boolean";
    properties["active"] = activeProperty;

    schema["properties"] = properties;

    QJsonArray required;
    required.append("name");
    required.append("age");
    schema["required"] = required;

    return schema;
}

QJsonObject createSampleConfiguration()
{
    QJsonObject config;
    config["name"] = "Configuration Demo User";
    config["age"] = 25;
    config["email"] = "demo@example.com";
    config["active"] = true;
    config["demo_interval"] = 3000;
    config["auto_start_demo"] = true;

    // Nested configuration
    QJsonObject preferences;
    preferences["theme"] = "dark";
    preferences["language"] = "en";
    preferences["notifications"] = true;
    config["preferences"] = preferences;

    return config;
}

QString validateAndReport(const QJsonObject& config, const QJsonObject& schema)
{
    // This is a simplified validation report
    // In a real implementation, you would use a proper JSON schema validator

    QStringList report;
    report << "Configuration Validation Report:";
    report << "================================";

    // Check required fields
    if (schema.contains("required") && schema["required"].isArray()) {
        QJsonArray required = schema["required"].toArray();
        for (const auto& req : required) {
            QString key = req.toString();
            if (config.contains(key)) {
                report << QString("✓ Required field '%1' present").arg(key);
            } else {
                report << QString("✗ Required field '%1' missing").arg(key);
            }
        }
    }

    // Check data types
    if (schema.contains("properties") && schema["properties"].isObject()) {
        QJsonObject properties = schema["properties"].toObject();
        for (auto it = config.begin(); it != config.end(); ++it) {
            QString key = it.key();
            if (properties.contains(key)) {
                QJsonObject propSchema = properties[key].toObject();
                QString expectedType = propSchema["type"].toString();
                QString actualType = "unknown";

                QJsonValue value = it.value();
                if (value.isString()) actualType = "string";
                else if (value.isDouble()) actualType = "number";
                else if (value.isBool()) actualType = "boolean";
                else if (value.isObject()) actualType = "object";
                else if (value.isArray()) actualType = "array";

                if (expectedType == actualType) {
                    report << QString("✓ Field '%1' has correct type (%2)").arg(key, actualType);
                } else {
                    report << QString("✗ Field '%1' has wrong type (expected %2, got %3)").arg(key, expectedType, actualType);
                }
            }
        }
    }

    return report.join("\n");
}

} // namespace ConfigurationExampleUtils

#include "configuration_example_plugin.moc"
