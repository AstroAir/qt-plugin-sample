/**
 * @file main.cpp
 * @brief Configuration example application
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <QStandardPaths>
#include <memory>
#include <filesystem>

// Include the QtPlugin library
#include "qtplugin/qtplugin.hpp"

/**
 * @brief Simple application to demonstrate configuration management
 */
class ConfigurationExampleApp : public QObject
{
    Q_OBJECT

public:
    explicit ConfigurationExampleApp(QObject* parent = nullptr)
        : QObject(parent)
        , m_plugin_manager(std::make_unique<qtplugin::PluginManager>())
    {
        qDebug() << "Configuration Example Application Started";
        qDebug() << "=========================================";
        
        setupPluginDirectories();
        demonstrateConfigurationManager();
    }

private slots:
    void loadAndStartPlugin()
    {
        qDebug() << "\n--- Loading Configuration Example Plugin ---";
        
        // Set plugin search paths
        QStringList plugin_paths;
        plugin_paths << QDir::currentPath() + "/plugins/examples";
        plugin_paths << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/plugins";
        
        for (const QString& path : plugin_paths) {
            qDebug() << "Adding plugin search path:" << path;
            m_plugin_manager->add_plugin_search_path(path);
        }
        
        // Discover plugins
        auto discovery_result = m_plugin_manager->discover_plugins();
        if (!discovery_result) {
            qDebug() << "Failed to discover plugins:" << QString::fromStdString(discovery_result.error().message);
            return;
        }
        
        qDebug() << "Discovered" << discovery_result.value() << "plugins";
        
        // List available plugins
        auto available_plugins = m_plugin_manager->get_available_plugins();
        qDebug() << "Available plugins:";
        for (const auto& plugin_info : available_plugins) {
            qDebug() << "  -" << plugin_info.id << ":" << plugin_info.name;
        }
        
        // Load the configuration example plugin
        auto load_result = m_plugin_manager->load_plugin("configuration_example");
        if (!load_result) {
            qDebug() << "Failed to load configuration example plugin:" << QString::fromStdString(load_result.error().message);
            return;
        }
        
        qDebug() << "Configuration example plugin loaded successfully";
        
        // Configure the plugin
        QJsonObject plugin_config;
        plugin_config["demo_interval"] = 2000; // 2 seconds between demos
        plugin_config["auto_start_demo"] = true;
        
        auto config_result = m_plugin_manager->configure_plugin("configuration_example", plugin_config);
        if (!config_result) {
            qDebug() << "Failed to configure plugin:" << QString::fromStdString(config_result.error().message);
        } else {
            qDebug() << "Plugin configured successfully";
        }
        
        // Start the plugin
        auto start_result = m_plugin_manager->start_plugin("configuration_example");
        if (!start_result) {
            qDebug() << "Failed to start plugin:" << QString::fromStdString(start_result.error().message);
        } else {
            qDebug() << "Plugin started successfully - demonstrations will begin";
        }
        
        // Schedule shutdown after demonstrations
        QTimer::singleShot(30000, this, &ConfigurationExampleApp::shutdown); // 30 seconds
    }
    
    void shutdown()
    {
        qDebug() << "\n--- Shutting Down Application ---";
        
        // Stop all plugins
        auto stop_result = m_plugin_manager->stop_all_plugins();
        if (!stop_result) {
            qDebug() << "Warning: Failed to stop all plugins:" << QString::fromStdString(stop_result.error().message);
        }
        
        // Unload all plugins
        auto unload_result = m_plugin_manager->unload_all_plugins();
        if (!unload_result) {
            qDebug() << "Warning: Failed to unload all plugins:" << QString::fromStdString(unload_result.error().message);
        }
        
        qDebug() << "Application shutdown complete";
        QCoreApplication::quit();
    }

private:
    void setupPluginDirectories()
    {
        // Create plugin directories if they don't exist
        QStringList dirs = {
            QDir::currentPath() + "/plugins",
            QDir::currentPath() + "/plugins/examples",
            QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/plugins"
        };
        
        for (const QString& dir : dirs) {
            QDir().mkpath(dir);
        }
    }
    
    void demonstrateConfigurationManager()
    {
        qDebug() << "\n--- Configuration Manager Demonstration ---";
        
        // Get the configuration manager
        auto& config_manager = m_plugin_manager->configuration_manager();
        
        // Demonstrate basic configuration operations
        qDebug() << "Setting global application configuration...";
        config_manager.set_value("app.name", QJsonValue("Configuration Demo App"));
        config_manager.set_value("app.version", QJsonValue("1.0.0"));
        config_manager.set_value("app.debug", QJsonValue(true));
        
        // Demonstrate nested configuration
        config_manager.set_value("database.host", QJsonValue("localhost"));
        config_manager.set_value("database.port", QJsonValue(5432));
        config_manager.set_value("database.name", QJsonValue("demo_db"));
        
        // Retrieve and display configuration
        auto app_name = config_manager.get_value("app.name");
        if (app_name.has_value()) {
            qDebug() << "Application name:" << app_name.value().toString();
        }
        
        auto db_config = config_manager.get_value("database");
        if (db_config.has_value() && db_config.value().isObject()) {
            qDebug() << "Database configuration:" << QJsonDocument(db_config.value().toObject()).toJson(QJsonDocument::Compact);
        }
        
        // Show configuration statistics
        auto stats = config_manager.get_statistics();
        qDebug() << "Configuration statistics:" << QJsonDocument(stats).toJson(QJsonDocument::Compact);
        
        // Schedule plugin loading
        QTimer::singleShot(1000, this, &ConfigurationExampleApp::loadAndStartPlugin);
    }
    
    std::unique_ptr<qtplugin::PluginManager> m_plugin_manager;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Configuration Example");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("QtPlugin Framework");
    
    // Create and run the example application
    ConfigurationExampleApp example_app;
    
    return app.exec();
}

#include "main.moc"
