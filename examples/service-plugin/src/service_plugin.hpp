#pragma once

#include <qtplugin/qtplugin.hpp>
#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include <QQueue>
#include <QElapsedTimer>
#include <memory>
#include <atomic>
#include <chrono>

/**
 * @brief Advanced Service Plugin demonstrating comprehensive service functionality
 * 
 * This plugin showcases:
 * - Complete service lifecycle management
 * - Configuration management with validation
 * - Background task processing with work queue
 * - Performance monitoring and metrics collection
 * - Error handling and recovery mechanisms
 * - Inter-plugin communication capabilities
 * - Resource management and cleanup
 */
class AdvancedServicePlugin : public QObject, public qtplugin::IServicePlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(AdvancedServicePlugin, "com.example.AdvancedService/1.0", "metadata.json", qtplugin::IServicePlugin)

public:
    explicit AdvancedServicePlugin(QObject* parent = nullptr);
    ~AdvancedServicePlugin() override;

    // === IPlugin Interface ===
    
    std::string_view name() const noexcept override { return "Advanced Service Plugin"; }
    std::string_view description() const noexcept override { 
        return "A comprehensive service plugin demonstrating advanced QtPlugin features"; 
    }
    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }
    std::string_view author() const noexcept override { return "QtPlugin Team"; }
    std::string_view license() const noexcept override { return "MIT"; }
    std::string_view homepage() const noexcept override { return "https://github.com/example/qtplugin"; }
    std::string_view category() const noexcept override { return "Service"; }
    std::string id() const noexcept override { return "com.example.advanced_service"; }

    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return qtplugin::PluginCapability::Service | 
               qtplugin::PluginCapability::Configuration |
               qtplugin::PluginCapability::Monitoring |
               qtplugin::PluginCapability::Threading |
               qtplugin::PluginCapability::AsyncInit;
    }

    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override { return m_plugin_state; }

    // Configuration management
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;

    // Command execution
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;

    // Error handling
    std::string last_error() const override { return m_last_error; }
    std::vector<std::string> error_log() const override;
    void clear_errors() override;

    // === IServicePlugin Interface ===
    
    qtplugin::expected<void, qtplugin::PluginError> start_service() override;
    qtplugin::expected<void, qtplugin::PluginError> stop_service() override;
    qtplugin::expected<void, qtplugin::PluginError> pause_service() override;
    qtplugin::expected<void, qtplugin::PluginError> resume_service() override;
    
    qtplugin::ServiceState service_state() const noexcept override { return m_service_state; }
    bool is_service_running() const noexcept override { 
        return m_service_state == qtplugin::ServiceState::Running; 
    }

    // Service configuration
    QJsonObject service_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> 
    configure_service(const QJsonObject& config) override;

    // Service monitoring
    qtplugin::ServiceHealth service_health() const override;
    QJsonObject service_metrics() const override;

signals:
    // Service lifecycle signals
    void service_started();
    void service_stopped();
    void service_paused();
    void service_resumed();
    void service_error(const QString& error);

    // Task processing signals
    void task_completed(int task_id, qint64 processing_time);
    void task_failed(int task_id, const QString& error);
    void queue_size_changed(int size);

    // Performance signals
    void performance_metrics_updated(const QJsonObject& metrics);

private slots:
    void on_timer_timeout();
    void on_performance_timer_timeout();
    void process_work_queue();

private:
    // === Internal Types ===
    
    struct WorkItem {
        int id;
        QString type;
        QJsonObject data;
        std::chrono::steady_clock::time_point created_at;
        int retry_count = 0;
    };

    struct PerformanceMetrics {
        std::atomic<int> tasks_processed{0};
        std::atomic<int> tasks_failed{0};
        std::atomic<qint64> total_processing_time{0};
        std::atomic<qint64> peak_memory_usage{0};
        std::chrono::steady_clock::time_point start_time;
        QQueue<qint64> processing_times; // For calculating averages
        mutable QMutex metrics_mutex;
    };

    // === Helper Methods ===
    
    void log_info(const QString& message);
    void log_warning(const QString& message);
    void log_error(const QString& message);
    
    void update_performance_metrics();
    void reset_performance_metrics();
    qint64 get_memory_usage() const;
    double get_cpu_usage() const;
    
    bool process_single_task(const WorkItem& item);
    void add_work_item(const QString& type, const QJsonObject& data);
    void clear_work_queue();
    
    void transition_to_state(qtplugin::ServiceState new_state);
    void handle_service_error(const QString& error);
    
    QJsonObject create_status_response() const;
    QJsonObject create_metrics_response() const;
    QJsonObject create_health_response() const;

    // === Member Variables ===
    
    // State management
    std::atomic<qtplugin::PluginState> m_plugin_state{qtplugin::PluginState::Unloaded};
    std::atomic<qtplugin::ServiceState> m_service_state{qtplugin::ServiceState::Stopped};
    
    // Configuration
    QJsonObject m_configuration;
    mutable QMutex m_config_mutex;
    
    // Timers and processing
    std::unique_ptr<QTimer> m_main_timer;
    std::unique_ptr<QTimer> m_performance_timer;
    QElapsedTimer m_uptime_timer;
    
    // Work queue management
    QQueue<WorkItem> m_work_queue;
    mutable QMutex m_queue_mutex;
    std::atomic<int> m_next_task_id{1};
    
    // Performance monitoring
    std::unique_ptr<PerformanceMetrics> m_metrics;
    
    // Error handling
    QString m_last_error;
    QStringList m_error_log;
    mutable QMutex m_error_mutex;
    
    // Configuration defaults
    static constexpr int DEFAULT_TIMER_INTERVAL = 1000;
    static constexpr int DEFAULT_MAX_QUEUE_SIZE = 100;
    static constexpr int DEFAULT_PERFORMANCE_INTERVAL = 5000;
    static constexpr int MAX_ERROR_LOG_SIZE = 100;
    static constexpr int MAX_PROCESSING_TIMES_HISTORY = 1000;
};

#include "service_plugin.moc"
};
