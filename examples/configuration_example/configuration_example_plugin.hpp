/**
 * @file configuration_example_plugin.hpp
 * @brief Example plugin demonstrating configuration management features
 * @version 3.0.0
 */

#pragma once

#include "qtplugin/qtplugin.hpp"
#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <memory>

/**
 * @brief Example plugin demonstrating configuration management
 * 
 * This plugin showcases various configuration management features:
 * - Basic configuration setting and retrieval
 * - Nested configuration structures
 * - Configuration validation with schemas
 * - Configuration persistence and loading
 * - Plugin-specific configuration scopes
 * - Configuration change notifications
 */
class ConfigurationExamplePlugin : public QObject, public qtplugin::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "configuration_example_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit ConfigurationExamplePlugin(QObject* parent = nullptr);
    ~ConfigurationExamplePlugin() override;

    // IPlugin interface
    QString id() const override;
    QString name() const override;
    QString description() const override;
    qtplugin::Version version() const override;
    QStringList dependencies() const override;

    qtplugin::expected<void, qtplugin::PluginError> initialize(qtplugin::IPluginManager* manager) override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    qtplugin::expected<void, qtplugin::PluginError> start() override;
    qtplugin::expected<void, qtplugin::PluginError> stop() override;
    qtplugin::expected<void, qtplugin::PluginError> cleanup() override;

    qtplugin::PluginState state() const override;
    QJsonObject status() const override;

public slots:
    /**
     * @brief Demonstrate basic configuration operations
     */
    void demonstrateBasicConfiguration();
    
    /**
     * @brief Demonstrate nested configuration structures
     */
    void demonstrateNestedConfiguration();
    
    /**
     * @brief Demonstrate configuration validation
     */
    void demonstrateConfigurationValidation();
    
    /**
     * @brief Demonstrate configuration persistence
     */
    void demonstrateConfigurationPersistence();
    
    /**
     * @brief Demonstrate different configuration scopes
     */
    void demonstrateConfigurationScopes();
    
    /**
     * @brief Demonstrate configuration change notifications
     */
    void demonstrateConfigurationNotifications();

private slots:
    /**
     * @brief Handle configuration change events
     */
    void onConfigurationChanged(const qtplugin::ConfigurationChangeEvent& event);
    
    /**
     * @brief Periodic demonstration timer
     */
    void onDemonstrationTimer();

private:
    qtplugin::IPluginManager* m_manager = nullptr;
    qtplugin::IConfigurationManager* m_config_manager = nullptr;
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
    
    std::unique_ptr<QTimer> m_demo_timer;
    int m_demo_step = 0;
    std::string m_change_subscription_id;
    
    /**
     * @brief Set up configuration schema for validation
     */
    void setupConfigurationSchema();
    
    /**
     * @brief Create default configuration
     */
    void createDefaultConfiguration();
    
    /**
     * @brief Log configuration operation result
     */
    void logConfigurationResult(const QString& operation, bool success, const QString& details = {});
};

/**
 * @brief Configuration example utility functions
 */
namespace ConfigurationExampleUtils {
    /**
     * @brief Create a sample configuration schema
     * @return JSON schema object
     */
    QJsonObject createSampleSchema();
    
    /**
     * @brief Create sample configuration data
     * @return Configuration object
     */
    QJsonObject createSampleConfiguration();
    
    /**
     * @brief Validate configuration against schema
     * @param config Configuration to validate
     * @param schema Schema to validate against
     * @return Validation result with details
     */
    QString validateAndReport(const QJsonObject& config, const QJsonObject& schema);
}
