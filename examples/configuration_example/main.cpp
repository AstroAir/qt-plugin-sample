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
#include <qtplugin/qtplugin.hpp>

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
            m_plugin_manager->add_search_path(std::filesystem::path(path.toStdString()));
        }

        // Discover plugins in current directory
        auto discovered_plugins = m_plugin_manager->discover_plugins(std::filesystem::current_path() / "plugins" / "examples", true);
        qDebug() << "Discovered" << discovered_plugins.size() << "plugins";
        
        // List available plugins
        auto available_plugins = m_plugin_manager->all_plugin_info();
        qDebug() << "Available plugins:";
        for (const auto& plugin_info : available_plugins) {
            qDebug() << "  -" << QString::fromStdString(plugin_info.id) << ":" << QString::fromStdString(plugin_info.metadata.name);
        }

        // Try to load plugins from discovered paths
        if (!discovered_plugins.empty()) {
            auto load_result = m_plugin_manager->load_plugin(discovered_plugins[0]);
            if (!load_result) {
                qDebug() << "Failed to load plugin:" << QString::fromStdString(load_result.error().message);
                return;
            }

            qDebug() << "Plugin loaded successfully:" << QString::fromStdString(load_result.value());
        } else {
            qDebug() << "No plugins found to load";
        }
        
        // Schedule shutdown after demonstrations
        QTimer::singleShot(30000, this, &ConfigurationExampleApp::shutdown); // 30 seconds
    }
    
    void shutdown()
    {
        qDebug() << "\n--- Shutting Down Application ---";

        // Stop all services
        int stopped = m_plugin_manager->stop_all_services();
        qDebug() << "Stopped" << stopped << "services";

        // Shutdown all plugins
        m_plugin_manager->shutdown_all_plugins();

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
        qDebug() << "This would demonstrate configuration management features";

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
