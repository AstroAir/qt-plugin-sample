/**
 * @file configuration_example_plugin.hpp
 * @brief Example plugin demonstrating configuration management features
 * @version 3.0.0
 */

#pragma once

#include <qtplugin/qtplugin.hpp>
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

    // IPlugin interface - Metadata
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;

    // IPlugin interface - Lifecycle
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;

    // IPlugin interface - Configuration
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;

    // IPlugin interface - Capabilities and Commands
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;

    // IPlugin interface - Dependencies
    std::vector<std::string> dependencies() const override;

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

    /**
     * @brief Handle configuration change events
     */
    void onConfigurationChanged(const qtplugin::ConfigurationChangeEvent& event);

private slots:
    /**
     * @brief Periodic demonstration timer
     */
    void onDemonstrationTimer();

private:
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;

    std::unique_ptr<QTimer> m_demo_timer;
    int m_demo_step = 0;
    
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
