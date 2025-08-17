/**
 * @file configuration_example_plugin.cpp
 * @brief Implementation of configuration example plugin
 * @version 3.0.0
 */

#include "configuration_example_plugin.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStandardPaths>
#include <filesystem>

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

QString ConfigurationExamplePlugin::id() const
{
    return "configuration_example";
}

QString ConfigurationExamplePlugin::name() const
{
    return "Configuration Example Plugin";
}

QString ConfigurationExamplePlugin::description() const
{
    return "Demonstrates configuration management features including schemas, persistence, and scopes";
}

qtplugin::Version ConfigurationExamplePlugin::version() const
{
    return qtplugin::Version(1, 0, 0);
}

QStringList ConfigurationExamplePlugin::dependencies() const
{
    return {};
}

qtplugin::expected<void, qtplugin::PluginError> 
ConfigurationExamplePlugin::initialize(qtplugin::IPluginManager* manager)
{
    if (!manager) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InvalidArgument, 
                                         "Plugin manager cannot be null");
    }
    
    m_manager = manager;
    m_config_manager = &manager->configuration_manager();
    m_state = qtplugin::PluginState::Initialized;
    
    qDebug() << "ConfigurationExamplePlugin: Initialized with configuration manager";
    
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

qtplugin::expected<void, qtplugin::PluginError> ConfigurationExamplePlugin::start()
{
    if (m_state != qtplugin::PluginState::Initialized) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InvalidState, 
                                         "Plugin must be initialized before starting");
    }
    
    m_state = qtplugin::PluginState::Running;
    
    qDebug() << "ConfigurationExamplePlugin: Started - Beginning configuration demonstrations";
    
    // Subscribe to configuration changes
    m_change_subscription_id = m_config_manager->subscribe_to_changes(
        [this](const qtplugin::ConfigurationChangeEvent& event) {
            onConfigurationChanged(event);
        },
        std::nullopt, // No key filter
        qtplugin::ConfigurationScope::Plugin, // Only plugin scope
        id().toStdString() // Only this plugin
    );
    
    // Start demonstration timer
    m_demo_timer->start();
    
    return qtplugin::make_success();
}

qtplugin::expected<void, qtplugin::PluginError> ConfigurationExamplePlugin::stop()
{
    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InvalidState, 
                                         "Plugin is not running");
    }
    
    m_state = qtplugin::PluginState::Stopped;
    
    // Stop demonstration timer
    m_demo_timer->stop();
    
    // Unsubscribe from configuration changes
    if (!m_change_subscription_id.empty()) {
        m_config_manager->unsubscribe_from_changes(m_change_subscription_id);
        m_change_subscription_id.clear();
    }
    
    qDebug() << "ConfigurationExamplePlugin: Stopped";
    
    return qtplugin::make_success();
}

qtplugin::expected<void, qtplugin::PluginError> ConfigurationExamplePlugin::cleanup()
{
    m_state = qtplugin::PluginState::Unloaded;
    m_manager = nullptr;
    m_config_manager = nullptr;
    
    qDebug() << "ConfigurationExamplePlugin: Cleaned up";
    
    return qtplugin::make_success();
}

qtplugin::PluginState ConfigurationExamplePlugin::state() const
{
    return m_state;
}

QJsonObject ConfigurationExamplePlugin::status() const
{
    QJsonObject status;
    status["state"] = static_cast<int>(m_state);
    status["demo_step"] = m_demo_step;
    status["demo_timer_active"] = m_demo_timer->isActive();
    status["demo_interval"] = m_demo_timer->interval();
    status["has_config_subscription"] = !m_change_subscription_id.empty();
    
    if (m_config_manager) {
        status["config_stats"] = m_config_manager->get_statistics();
    }
    
    return status;
}

void ConfigurationExamplePlugin::demonstrateBasicConfiguration()
{
    qDebug() << "\n=== DEMONSTRATING BASIC CONFIGURATION ===";
    
    // Set basic configuration values
    auto result1 = m_config_manager->set_value("app_name", QJsonValue("Configuration Demo App"), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set app_name", result1.has_value());
    
    auto result2 = m_config_manager->set_value("version", QJsonValue("1.0.0"), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set version", result2.has_value());
    
    auto result3 = m_config_manager->set_value("max_connections", QJsonValue(100), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set max_connections", result3.has_value());
    
    auto result4 = m_config_manager->set_value("debug_enabled", QJsonValue(true), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set debug_enabled", result4.has_value());
    
    // Retrieve and display values
    auto app_name = m_config_manager->get_value("app_name", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    if (app_name.has_value()) {
        qDebug() << "Retrieved app_name:" << app_name.value().toString();
    }
    
    auto max_connections = m_config_manager->get_value("max_connections", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    if (max_connections.has_value()) {
        qDebug() << "Retrieved max_connections:" << max_connections.value().toInt();
    }
    
    // Demonstrate default values
    auto timeout = m_config_manager->get_value_or_default("timeout", QJsonValue(30), 
                                                          qtplugin::ConfigurationScope::Plugin, id().toStdString());
    qDebug() << "Timeout (with default):" << timeout.toInt();
}

void ConfigurationExamplePlugin::demonstrateNestedConfiguration()
{
    qDebug() << "\n=== DEMONSTRATING NESTED CONFIGURATION ===";
    
    // Set nested configuration values
    auto result1 = m_config_manager->set_value("database.host", QJsonValue("localhost"), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set database.host", result1.has_value());
    
    auto result2 = m_config_manager->set_value("database.port", QJsonValue(5432), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set database.port", result2.has_value());
    
    auto result3 = m_config_manager->set_value("database.credentials.username", QJsonValue("admin"), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set database.credentials.username", result3.has_value());
    
    auto result4 = m_config_manager->set_value("database.credentials.password", QJsonValue("secret"), 
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set database.credentials.password", result4.has_value());
    
    // Retrieve nested values
    auto host = m_config_manager->get_value("database.host", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    if (host.has_value()) {
        qDebug() << "Database host:" << host.value().toString();
    }
    
    auto username = m_config_manager->get_value("database.credentials.username", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    if (username.has_value()) {
        qDebug() << "Database username:" << username.value().toString();
    }
    
    // Retrieve entire nested object
    auto database_config = m_config_manager->get_value("database", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    if (database_config.has_value() && database_config.value().isObject()) {
        qDebug() << "Complete database config:" << QJsonDocument(database_config.value().toObject()).toJson(QJsonDocument::Compact);
    }
}

void ConfigurationExamplePlugin::onConfigurationChanged(const qtplugin::ConfigurationChangeEvent& event)
{
    qDebug() << "Configuration changed:" << QString::fromStdString(event.key) 
             << "Type:" << static_cast<int>(event.type)
             << "Scope:" << static_cast<int>(event.scope);
}

void ConfigurationExamplePlugin::demonstrateConfigurationValidation()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION VALIDATION ===";

    // Create a configuration schema
    QJsonObject schema = ConfigurationExampleUtils::createSampleSchema();
    qtplugin::ConfigurationSchema config_schema(schema, false);

    // Set the schema
    auto schema_result = m_config_manager->set_schema(config_schema, qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Set configuration schema", schema_result.has_value());

    // Test valid configuration
    QJsonObject valid_config = ConfigurationExampleUtils::createSampleConfiguration();
    auto validation_result = m_config_manager->validate_configuration(valid_config, config_schema);
    qDebug() << "Valid config validation:" << (validation_result.is_valid ? "PASSED" : "FAILED");

    // Test invalid configuration
    QJsonObject invalid_config;
    invalid_config["name"] = 123; // Should be string
    invalid_config["age"] = -5;   // Should be positive

    auto invalid_validation = m_config_manager->validate_configuration(invalid_config, config_schema);
    qDebug() << "Invalid config validation:" << (invalid_validation.is_valid ? "PASSED" : "FAILED");
    if (!invalid_validation.errors.empty()) {
        qDebug() << "Validation errors:";
        for (const auto& error : invalid_validation.errors) {
            qDebug() << "  -" << QString::fromStdString(error);
        }
    }
}

void ConfigurationExamplePlugin::demonstrateConfigurationPersistence()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION PERSISTENCE ===";

    // Set some configuration data
    auto result1 = m_config_manager->set_value("persistent.setting1", QJsonValue("saved_value"),
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    auto result2 = m_config_manager->set_value("persistent.setting2", QJsonValue(42),
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());

    logConfigurationResult("Set persistent settings", result1.has_value() && result2.has_value());

    // Save to file
    std::filesystem::path config_file = std::filesystem::path(QStandardPaths::writableLocation(QStandardPaths::TempLocation).toStdString()) / "demo_config.json";
    auto save_result = m_config_manager->save_to_file(config_file, qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Save to file", save_result.has_value(), QString::fromStdString(config_file.string()));

    // Clear configuration
    auto clear_result = m_config_manager->clear_configuration(qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Clear configuration", clear_result.has_value());

    // Verify cleared
    bool cleared = !m_config_manager->has_key("persistent.setting1", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    qDebug() << "Configuration cleared:" << (cleared ? "YES" : "NO");

    // Load from file
    auto load_result = m_config_manager->load_from_file(config_file, qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Load from file", load_result.has_value());

    // Verify loaded
    auto loaded_value = m_config_manager->get_value("persistent.setting1", qtplugin::ConfigurationScope::Plugin, id().toStdString());
    if (loaded_value.has_value()) {
        qDebug() << "Loaded value:" << loaded_value.value().toString();
    }
}

void ConfigurationExamplePlugin::demonstrateConfigurationScopes()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION SCOPES ===";

    // Set same key in different scopes
    auto global_result = m_config_manager->set_value("scope_test", QJsonValue("global_value"), qtplugin::ConfigurationScope::Global);
    auto user_result = m_config_manager->set_value("scope_test", QJsonValue("user_value"), qtplugin::ConfigurationScope::User);
    auto plugin_result = m_config_manager->set_value("scope_test", QJsonValue("plugin_value"), qtplugin::ConfigurationScope::Plugin, id().toStdString());

    logConfigurationResult("Set values in different scopes",
                          global_result.has_value() && user_result.has_value() && plugin_result.has_value());

    // Retrieve from different scopes
    auto global_value = m_config_manager->get_value("scope_test", qtplugin::ConfigurationScope::Global);
    auto user_value = m_config_manager->get_value("scope_test", qtplugin::ConfigurationScope::User);
    auto plugin_value = m_config_manager->get_value("scope_test", qtplugin::ConfigurationScope::Plugin, id().toStdString());

    if (global_value.has_value()) qDebug() << "Global scope:" << global_value.value().toString();
    if (user_value.has_value()) qDebug() << "User scope:" << user_value.value().toString();
    if (plugin_value.has_value()) qDebug() << "Plugin scope:" << plugin_value.value().toString();
}

void ConfigurationExamplePlugin::demonstrateConfigurationNotifications()
{
    qDebug() << "\n=== DEMONSTRATING CONFIGURATION NOTIFICATIONS ===";

    // The change subscription was set up in start() method
    qDebug() << "Configuration change subscription active:" << (!m_change_subscription_id.empty() ? "YES" : "NO");

    // Make some changes to trigger notifications
    auto result1 = m_config_manager->set_value("notification_test", QJsonValue("initial"),
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    auto result2 = m_config_manager->set_value("notification_test", QJsonValue("modified"),
                                              qtplugin::ConfigurationScope::Plugin, id().toStdString());
    auto result3 = m_config_manager->remove_key("notification_test", qtplugin::ConfigurationScope::Plugin, id().toStdString());

    logConfigurationResult("Configuration change notifications",
                          result1.has_value() && result2.has_value() && result3.has_value(),
                          "Check debug output for change events");
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

    QJsonObject schema = ConfigurationExampleUtils::createSampleSchema();
    qtplugin::ConfigurationSchema config_schema(schema, false);

    auto result = m_config_manager->set_schema(config_schema, qtplugin::ConfigurationScope::Plugin, id().toStdString());
    logConfigurationResult("Setup configuration schema", result.has_value());
}

void ConfigurationExamplePlugin::createDefaultConfiguration()
{
    qDebug() << "ConfigurationExamplePlugin: Creating default configuration";

    QJsonObject default_config = ConfigurationExampleUtils::createSampleConfiguration();

    auto result = m_config_manager->set_configuration(default_config, qtplugin::ConfigurationScope::Plugin, id().toStdString(), true);
    logConfigurationResult("Create default configuration", result.has_value());
}

void ConfigurationExamplePlugin::logConfigurationResult(const QString& operation, bool success, const QString& details)
{
    QString status = success ? "SUCCESS" : "FAILED";
    QString message = QString("ConfigurationExamplePlugin: %1 - %2").arg(operation, status);

    if (!details.isEmpty()) {
        message += QString(" (%1)").arg(details);
    }

    qDebug() << message;
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
