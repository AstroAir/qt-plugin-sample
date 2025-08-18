/**
 * @file main.cpp
 * @brief Test application for the Advanced Service Plugin
 * 
 * This application demonstrates the comprehensive features of the service plugin:
 * - Plugin loading and initialization
 * - Service lifecycle management
 * - Configuration management
 * - Performance monitoring
 * - Command execution
 * - Error handling
 */

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <iostream>
#include <memory>

// Include QtPlugin headers
#include <qtplugin/qtplugin.hpp>

class ServicePluginDemo : public QObject {
    Q_OBJECT

public:
    explicit ServicePluginDemo(QObject* parent = nullptr) : QObject(parent) {}

    int run() {
        std::cout << "=== Advanced Service Plugin Demo ===" << std::endl;
        
        // Initialize QtPlugin library
        qtplugin::LibraryInitializer init;
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize QtPlugin library" << std::endl;
            return -1;
        }
        
        std::cout << "QtPlugin library initialized, version: " << QTPLUGIN_VERSION << std::endl;
        
        // Create plugin manager
        m_manager = std::make_unique<qtplugin::PluginManager>();
        
        // Load the service plugin
        std::cout << "\n=== Loading Service Plugin ===" << std::endl;
        QString plugin_path = "./plugins/service_plugin.qtplugin";
        
        if (!QDir().exists(plugin_path)) {
            std::cerr << "Plugin file not found: " << plugin_path.toStdString() << std::endl;
            std::cerr << "Make sure to build the plugin first and copy it to the plugins directory" << std::endl;
            return -1;
        }
        
        auto load_result = m_manager->load_plugin(plugin_path.toStdString());
        if (!load_result) {
            std::cerr << "Failed to load plugin: " << load_result.error().message << std::endl;
            return -1;
        }
        
        m_plugin_id = QString::fromStdString(load_result.value());
        std::cout << "Plugin loaded successfully with ID: " << m_plugin_id.toStdString() << std::endl;
        
        // Get plugin instance
        m_plugin = m_manager->get_plugin(m_plugin_id.toStdString());
        if (!m_plugin) {
            std::cerr << "Failed to get plugin instance" << std::endl;
            return -1;
        }
        
        // Get service plugin interface
        m_service_plugin = qobject_cast<qtplugin::IServicePlugin*>(m_plugin.get());
        if (!m_service_plugin) {
            std::cerr << "Plugin does not implement IServicePlugin interface" << std::endl;
            return -1;
        }
        
        // Display plugin information
        std::cout << "Plugin Name: " << m_plugin->name() << std::endl;
        std::cout << "Plugin Version: " << m_plugin->version().to_string() << std::endl;
        std::cout << "Plugin Author: " << m_plugin->author() << std::endl;
        std::cout << "Plugin Description: " << m_plugin->description() << std::endl;
        
        // Connect to plugin signals
        connect_plugin_signals();
        
        // Initialize the plugin
        std::cout << "\n=== Initializing Plugin ===" << std::endl;
        auto init_result = m_plugin->initialize();
        if (!init_result) {
            std::cerr << "Failed to initialize plugin: " << init_result.error().message << std::endl;
            return -1;
        }
        std::cout << "Plugin initialized successfully" << std::endl;
        
        // Demonstrate plugin features
        demonstrate_configuration();
        demonstrate_service_lifecycle();
        demonstrate_commands();
        demonstrate_performance_monitoring();
        
        // Set up demo timer
        m_demo_timer = std::make_unique<QTimer>(this);
        connect(m_demo_timer.get(), &QTimer::timeout, this, &ServicePluginDemo::on_demo_timer);
        m_demo_timer->start(2000); // Demo actions every 2 seconds
        
        // Set up shutdown timer
        QTimer::singleShot(15000, this, &ServicePluginDemo::shutdown_demo); // Run for 15 seconds
        
        return 0;
    }

private slots:
    void on_demo_timer() {
        static int demo_step = 0;
        ++demo_step;
        
        switch (demo_step) {
        case 1:
            add_demo_tasks();
            break;
        case 2:
            show_metrics();
            break;
        case 3:
            demonstrate_pause_resume();
            break;
        case 4:
            show_health_status();
            break;
        case 5:
            update_configuration();
            break;
        default:
            show_status();
            break;
        }
    }
    
    void shutdown_demo() {
        std::cout << "\n=== Demo Shutdown ===" << std::endl;
        
        // Stop service
        if (m_service_plugin && m_service_plugin->is_service_running()) {
            auto stop_result = m_service_plugin->stop_service();
            if (stop_result) {
                std::cout << "Service stopped successfully" << std::endl;
            } else {
                std::cerr << "Failed to stop service: " << stop_result.error().message << std::endl;
            }
        }
        
        // Shutdown plugin
        if (m_plugin) {
            m_plugin->shutdown();
            std::cout << "Plugin shutdown complete" << std::endl;
        }
        
        std::cout << "Demo completed successfully!" << std::endl;
        QCoreApplication::quit();
    }
    
    // Plugin signal handlers
    void on_service_started() {
        std::cout << "[EVENT] Service started" << std::endl;
    }
    
    void on_service_stopped() {
        std::cout << "[EVENT] Service stopped" << std::endl;
    }
    
    void on_service_paused() {
        std::cout << "[EVENT] Service paused" << std::endl;
    }
    
    void on_service_resumed() {
        std::cout << "[EVENT] Service resumed" << std::endl;
    }
    
    void on_service_error(const QString& error) {
        std::cout << "[EVENT] Service error: " << error.toStdString() << std::endl;
    }
    
    void on_task_completed(int task_id, qint64 processing_time) {
        std::cout << "[EVENT] Task " << task_id << " completed in " << processing_time << "ms" << std::endl;
    }
    
    void on_task_failed(int task_id, const QString& error) {
        std::cout << "[EVENT] Task " << task_id << " failed: " << error.toStdString() << std::endl;
    }
    
    void on_queue_size_changed(int size) {
        if (size > 0) {
            std::cout << "[EVENT] Queue size changed: " << size << " items" << std::endl;
        }
    }
    
    void on_performance_metrics_updated(const QJsonObject& metrics) {
        std::cout << "[EVENT] Performance metrics updated" << std::endl;
    }

private:
    void connect_plugin_signals() {
        // Connect service lifecycle signals
        connect(m_service_plugin, &qtplugin::IServicePlugin::service_started,
                this, &ServicePluginDemo::on_service_started);
        connect(m_service_plugin, &qtplugin::IServicePlugin::service_stopped,
                this, &ServicePluginDemo::on_service_stopped);
        connect(m_service_plugin, &qtplugin::IServicePlugin::service_paused,
                this, &ServicePluginDemo::on_service_paused);
        connect(m_service_plugin, &qtplugin::IServicePlugin::service_resumed,
                this, &ServicePluginDemo::on_service_resumed);
        connect(m_service_plugin, &qtplugin::IServicePlugin::service_error,
                this, &ServicePluginDemo::on_service_error);
        
        // Connect task processing signals (if available)
        if (auto advanced_plugin = qobject_cast<QObject*>(m_service_plugin)) {
            connect(advanced_plugin, SIGNAL(task_completed(int, qint64)),
                    this, SLOT(on_task_completed(int, qint64)));
            connect(advanced_plugin, SIGNAL(task_failed(int, QString)),
                    this, SLOT(on_task_failed(int, QString)));
            connect(advanced_plugin, SIGNAL(queue_size_changed(int)),
                    this, SLOT(on_queue_size_changed(int)));
            connect(advanced_plugin, SIGNAL(performance_metrics_updated(QJsonObject)),
                    this, SLOT(on_performance_metrics_updated(QJsonObject)));
        }
    }
    
    void demonstrate_configuration() {
        std::cout << "\n=== Configuration Management ===" << std::endl;
        
        // Get default configuration
        auto default_config = m_plugin->default_configuration();
        if (default_config) {
            std::cout << "Default configuration loaded" << std::endl;
            QJsonDocument doc(default_config.value());
            std::cout << "Config: " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
        
        // Update configuration
        QJsonObject new_config;
        new_config["timer_interval"] = 800;
        new_config["enable_monitoring"] = true;
        new_config["auto_start"] = true;
        
        auto config_result = m_plugin->configure(new_config);
        if (config_result) {
            std::cout << "Configuration updated successfully" << std::endl;
        } else {
            std::cerr << "Configuration update failed: " << config_result.error().message << std::endl;
        }
    }
    
    void demonstrate_service_lifecycle() {
        std::cout << "\n=== Service Lifecycle Management ===" << std::endl;
        
        // Start service
        auto start_result = m_service_plugin->start_service();
        if (start_result) {
            std::cout << "Service started successfully" << std::endl;
        } else {
            std::cerr << "Failed to start service: " << start_result.error().message << std::endl;
        }
    }
    
    void demonstrate_commands() {
        std::cout << "\n=== Command Execution ===" << std::endl;
        
        // List available commands
        auto commands = m_plugin->available_commands();
        std::cout << "Available commands: ";
        for (const auto& cmd : commands) {
            std::cout << cmd << " ";
        }
        std::cout << std::endl;
        
        // Execute status command
        auto status_result = m_plugin->execute_command("status");
        if (status_result) {
            QJsonDocument doc(status_result.value());
            std::cout << "Status: " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
    }
    
    void demonstrate_performance_monitoring() {
        std::cout << "\n=== Performance Monitoring ===" << std::endl;
        
        auto metrics_result = m_plugin->execute_command("metrics");
        if (metrics_result) {
            QJsonDocument doc(metrics_result.value());
            std::cout << "Metrics: " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
    }
    
    void add_demo_tasks() {
        std::cout << "\n=== Adding Demo Tasks ===" << std::endl;
        
        for (int i = 0; i < 3; ++i) {
            QJsonObject params;
            params["type"] = "demo_task";
            
            QJsonObject task_data;
            task_data["task_number"] = i + 1;
            task_data["description"] = QString("Demo task %1").arg(i + 1);
            params["data"] = task_data;
            
            auto result = m_plugin->execute_command("add_task", params);
            if (result) {
                std::cout << "Added demo task " << (i + 1) << std::endl;
            }
        }
    }
    
    void show_metrics() {
        auto metrics_result = m_plugin->execute_command("metrics");
        if (metrics_result) {
            QJsonDocument doc(metrics_result.value());
            std::cout << "\n[METRICS] " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
    }
    
    void demonstrate_pause_resume() {
        std::cout << "\n=== Pause/Resume Demo ===" << std::endl;
        
        // Pause service
        auto pause_result = m_service_plugin->pause_service();
        if (pause_result) {
            std::cout << "Service paused" << std::endl;
            
            // Resume after a short delay
            QTimer::singleShot(1000, [this]() {
                auto resume_result = m_service_plugin->resume_service();
                if (resume_result) {
                    std::cout << "Service resumed" << std::endl;
                }
            });
        }
    }
    
    void show_health_status() {
        auto health_result = m_plugin->execute_command("health");
        if (health_result) {
            QJsonDocument doc(health_result.value());
            std::cout << "\n[HEALTH] " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
    }
    
    void update_configuration() {
        std::cout << "\n=== Configuration Update ===" << std::endl;
        
        QJsonObject config_update;
        config_update["timer_interval"] = 1200;
        config_update["max_queue_size"] = 150;
        
        auto result = m_plugin->execute_command("configure", config_update);
        if (result) {
            std::cout << "Configuration updated during runtime" << std::endl;
        }
    }
    
    void show_status() {
        auto status_result = m_plugin->execute_command("status");
        if (status_result) {
            QJsonDocument doc(status_result.value());
            std::cout << "\n[STATUS] " << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
        }
    }

private:
    std::unique_ptr<qtplugin::PluginManager> m_manager;
    std::shared_ptr<qtplugin::IPlugin> m_plugin;
    qtplugin::IServicePlugin* m_service_plugin = nullptr;
    QString m_plugin_id;
    std::unique_ptr<QTimer> m_demo_timer;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    ServicePluginDemo demo;
    int result = demo.run();
    
    if (result != 0) {
        return result;
    }
    
    return app.exec();
}

#include "main.moc"
