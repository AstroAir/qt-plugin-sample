#include "service_plugin.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QMutexLocker>
#include <chrono>

AdvancedServicePlugin::AdvancedServicePlugin(QObject* parent)
    : QObject(parent)
    , m_main_timer(std::make_unique<QTimer>(this))
    , m_performance_timer(std::make_unique<QTimer>(this))
    , m_metrics(std::make_unique<PerformanceMetrics>())
{
    // Connect timers
    connect(m_main_timer.get(), &QTimer::timeout, this, &AdvancedServicePlugin::on_timer_timeout);
    connect(m_performance_timer.get(), &QTimer::timeout, this, &AdvancedServicePlugin::on_performance_timer_timeout);
    
    // Initialize performance metrics
    m_metrics->start_time = std::chrono::steady_clock::now();
    
    log_info("AdvancedServicePlugin created");
}

AdvancedServicePlugin::~AdvancedServicePlugin() {
    if (m_plugin_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
    log_info("AdvancedServicePlugin destroyed");
}

qtplugin::expected<void, qtplugin::PluginError> AdvancedServicePlugin::initialize() {
    if (m_plugin_state != qtplugin::PluginState::Unloaded && 
        m_plugin_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError, 
                                         "Plugin is not in a state that allows initialization");
    }
    
    m_plugin_state = qtplugin::PluginState::Initializing;
    
    try {
        // Load default configuration if none exists
        if (m_configuration.isEmpty()) {
            auto default_config = default_configuration();
            if (default_config) {
                m_configuration = default_config.value();
            }
        }
        
        // Configure timers
        int timer_interval = m_configuration.value("timer_interval").toInt(DEFAULT_TIMER_INTERVAL);
        int perf_interval = m_configuration.value("performance_interval").toInt(DEFAULT_PERFORMANCE_INTERVAL);
        
        m_main_timer->setInterval(timer_interval);
        m_performance_timer->setInterval(perf_interval);
        
        // Start performance monitoring
        if (m_configuration.value("enable_monitoring").toBool(true)) {
            m_performance_timer->start();
        }
        
        // Start uptime tracking
        m_uptime_timer.start();
        
        m_plugin_state = qtplugin::PluginState::Running;
        
        log_info("AdvancedServicePlugin initialized successfully");
        
        // Auto-start service if configured
        if (m_configuration.value("auto_start").toBool(false)) {
            auto start_result = start_service();
            if (!start_result) {
                log_warning("Auto-start failed: " + QString::fromStdString(start_result.error().message));
            }
        }
        
        return qtplugin::make_success();
        
    } catch (const std::exception& e) {
        m_plugin_state = qtplugin::PluginState::Error;
        QString error_msg = QString("Initialization failed: %1").arg(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InitializationFailed, error_msg.toStdString());
    }
}

void AdvancedServicePlugin::shutdown() noexcept {
    try {
        log_info("Shutting down AdvancedServicePlugin");
        
        // Stop service if running
        if (is_service_running()) {
            stop_service();
        }
        
        // Stop timers
        if (m_main_timer) {
            m_main_timer->stop();
        }
        if (m_performance_timer) {
            m_performance_timer->stop();
        }
        
        // Clear work queue
        clear_work_queue();
        
        // Reset metrics
        reset_performance_metrics();
        
        m_plugin_state = qtplugin::PluginState::Unloaded;
        m_service_state = qtplugin::ServiceState::Stopped;
        
        log_info("AdvancedServicePlugin shutdown complete");
        
    } catch (...) {
        // Ensure no exceptions escape from shutdown
        m_plugin_state = qtplugin::PluginState::Error;
    }
}

std::optional<QJsonObject> AdvancedServicePlugin::default_configuration() const {
    QJsonObject config;
    config["timer_interval"] = DEFAULT_TIMER_INTERVAL;
    config["max_queue_size"] = DEFAULT_MAX_QUEUE_SIZE;
    config["performance_interval"] = DEFAULT_PERFORMANCE_INTERVAL;
    config["enable_monitoring"] = true;
    config["auto_start"] = false;
    config["log_level"] = "info";
    
    QJsonObject performance;
    performance["enabled"] = true;
    performance["sample_rate"] = 1.0;
    performance["history_size"] = MAX_PROCESSING_TIMES_HISTORY;
    config["performance_tracking"] = performance;
    
    QJsonObject retry_policy;
    retry_policy["max_retries"] = 3;
    retry_policy["retry_delay"] = 1000;
    retry_policy["exponential_backoff"] = true;
    config["retry_policy"] = retry_policy;
    
    return config;
}

qtplugin::expected<void, qtplugin::PluginError> AdvancedServicePlugin::configure(const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InvalidConfiguration, 
                                         "Configuration validation failed");
    }
    
    QMutexLocker locker(&m_config_mutex);
    
    // Merge with existing configuration
    QJsonObject new_config = m_configuration;
    for (auto it = config.begin(); it != config.end(); ++it) {
        new_config[it.key()] = it.value();
    }
    
    // Apply configuration changes
    bool timer_changed = false;
    if (config.contains("timer_interval")) {
        int new_interval = config["timer_interval"].toInt();
        if (new_interval != m_main_timer->interval()) {
            m_main_timer->setInterval(new_interval);
            timer_changed = true;
        }
    }
    
    if (config.contains("performance_interval")) {
        int new_interval = config["performance_interval"].toInt();
        if (new_interval != m_performance_timer->interval()) {
            m_performance_timer->setInterval(new_interval);
        }
    }
    
    if (config.contains("enable_monitoring")) {
        bool enable = config["enable_monitoring"].toBool();
        if (enable && !m_performance_timer->isActive()) {
            m_performance_timer->start();
        } else if (!enable && m_performance_timer->isActive()) {
            m_performance_timer->stop();
        }
    }
    
    m_configuration = new_config;
    
    log_info(QString("Configuration updated%1").arg(timer_changed ? " (timer interval changed)" : ""));
    
    return qtplugin::make_success();
}

QJsonObject AdvancedServicePlugin::current_configuration() const {
    QMutexLocker locker(&m_config_mutex);
    return m_configuration;
}

bool AdvancedServicePlugin::validate_configuration(const QJsonObject& config) const {
    // Validate timer_interval
    if (config.contains("timer_interval")) {
        int interval = config["timer_interval"].toInt(-1);
        if (interval < 100 || interval > 60000) {
            log_error("Invalid timer_interval: must be between 100 and 60000 ms");
            return false;
        }
    }
    
    // Validate max_queue_size
    if (config.contains("max_queue_size")) {
        int size = config["max_queue_size"].toInt(-1);
        if (size < 1 || size > 10000) {
            log_error("Invalid max_queue_size: must be between 1 and 10000");
            return false;
        }
    }
    
    // Validate performance_interval
    if (config.contains("performance_interval")) {
        int interval = config["performance_interval"].toInt(-1);
        if (interval < 1000 || interval > 300000) {
            log_error("Invalid performance_interval: must be between 1000 and 300000 ms");
            return false;
        }
    }
    
    // Validate log_level
    if (config.contains("log_level")) {
        QString level = config["log_level"].toString();
        QStringList valid_levels = {"debug", "info", "warning", "error"};
        if (!valid_levels.contains(level)) {
            log_error("Invalid log_level: must be one of debug, info, warning, error");
            return false;
        }
    }
    
    return true;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
AdvancedServicePlugin::execute_command(std::string_view command, const QJsonObject& params) {
    QString cmd = QString::fromUtf8(command.data(), command.size());
    
    try {
        if (cmd == "status") {
            return create_status_response();
        }
        else if (cmd == "metrics") {
            return create_metrics_response();
        }
        else if (cmd == "health") {
            return create_health_response();
        }
        else if (cmd == "start") {
            auto result = start_service();
            QJsonObject response;
            response["success"] = result.has_value();
            if (!result) {
                response["error"] = QString::fromStdString(result.error().message);
            }
            return response;
        }
        else if (cmd == "stop") {
            auto result = stop_service();
            QJsonObject response;
            response["success"] = result.has_value();
            if (!result) {
                response["error"] = QString::fromStdString(result.error().message);
            }
            return response;
        }
        else if (cmd == "pause") {
            auto result = pause_service();
            QJsonObject response;
            response["success"] = result.has_value();
            if (!result) {
                response["error"] = QString::fromStdString(result.error().message);
            }
            return response;
        }
        else if (cmd == "resume") {
            auto result = resume_service();
            QJsonObject response;
            response["success"] = result.has_value();
            if (!result) {
                response["error"] = QString::fromStdString(result.error().message);
            }
            return response;
        }
        else if (cmd == "configure") {
            auto result = configure(params);
            QJsonObject response;
            response["success"] = result.has_value();
            if (!result) {
                response["error"] = QString::fromStdString(result.error().message);
            }
            return response;
        }
        else if (cmd == "add_task") {
            QString task_type = params.value("type").toString("default");
            QJsonObject task_data = params.value("data").toObject();
            add_work_item(task_type, task_data);
            
            QJsonObject response;
            response["success"] = true;
            response["task_id"] = m_next_task_id.load() - 1;
            return response;
        }
        else if (cmd == "clear_queue") {
            clear_work_queue();
            QJsonObject response;
            response["success"] = true;
            response["message"] = "Work queue cleared";
            return response;
        }
        else if (cmd == "reset_metrics") {
            reset_performance_metrics();
            QJsonObject response;
            response["success"] = true;
            response["message"] = "Performance metrics reset";
            return response;
        }
        else {
            return qtplugin::make_error<QJsonObject>(
                qtplugin::PluginErrorCode::CommandNotFound,
                "Unknown command: " + cmd.toStdString()
            );
        }
        
    } catch (const std::exception& e) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::ExecutionFailed,
            "Command execution failed: " + std::string(e.what())
        );
    }
}

std::vector<std::string> AdvancedServicePlugin::available_commands() const {
    return {
        "status", "metrics", "health", "start", "stop", "pause", "resume",
        "configure", "add_task", "clear_queue", "reset_metrics"
    };
}

std::vector<std::string> AdvancedServicePlugin::error_log() const {
    QMutexLocker locker(&m_error_mutex);
    std::vector<std::string> log;
    log.reserve(m_error_log.size());
    for (const QString& error : m_error_log) {
        log.push_back(error.toStdString());
    }
    return log;
}

void AdvancedServicePlugin::clear_errors() {
    QMutexLocker locker(&m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
}

// === IServicePlugin Implementation ===

qtplugin::expected<void, qtplugin::PluginError> AdvancedServicePlugin::start_service() {
    if (m_service_state == qtplugin::ServiceState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                         "Service is already running");
    }

    if (m_plugin_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                         "Plugin must be initialized before starting service");
    }

    try {
        transition_to_state(qtplugin::ServiceState::Starting);

        // Start main timer
        m_main_timer->start();

        // Reset performance metrics
        reset_performance_metrics();

        transition_to_state(qtplugin::ServiceState::Running);

        log_info("Service started successfully");
        emit service_started();

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        QString error_msg = QString("Failed to start service: %1").arg(e.what());
        handle_service_error(error_msg);
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::ServiceStartFailed, error_msg.toStdString());
    }
}

qtplugin::expected<void, qtplugin::PluginError> AdvancedServicePlugin::stop_service() {
    if (m_service_state == qtplugin::ServiceState::Stopped) {
        return qtplugin::make_success(); // Already stopped
    }

    try {
        transition_to_state(qtplugin::ServiceState::Stopping);

        // Stop main timer
        m_main_timer->stop();

        // Process remaining work items
        process_work_queue();

        // Clear work queue
        clear_work_queue();

        transition_to_state(qtplugin::ServiceState::Stopped);

        log_info("Service stopped successfully");
        emit service_stopped();

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        QString error_msg = QString("Failed to stop service: %1").arg(e.what());
        handle_service_error(error_msg);
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::ServiceStopFailed, error_msg.toStdString());
    }
}

qtplugin::expected<void, qtplugin::PluginError> AdvancedServicePlugin::pause_service() {
    if (m_service_state != qtplugin::ServiceState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                         "Service must be running to pause");
    }

    try {
        transition_to_state(qtplugin::ServiceState::Pausing);

        // Stop main timer but keep performance monitoring
        m_main_timer->stop();

        transition_to_state(qtplugin::ServiceState::Paused);

        log_info("Service paused successfully");
        emit service_paused();

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        QString error_msg = QString("Failed to pause service: %1").arg(e.what());
        handle_service_error(error_msg);
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::ServicePauseFailed, error_msg.toStdString());
    }
}

qtplugin::expected<void, qtplugin::PluginError> AdvancedServicePlugin::resume_service() {
    if (m_service_state != qtplugin::ServiceState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                         "Service must be paused to resume");
    }

    try {
        transition_to_state(qtplugin::ServiceState::Resuming);

        // Restart main timer
        m_main_timer->start();

        transition_to_state(qtplugin::ServiceState::Running);

        log_info("Service resumed successfully");
        emit service_resumed();

        return qtplugin::make_success();

    } catch (const std::exception& e) {
        QString error_msg = QString("Failed to resume service: %1").arg(e.what());
        handle_service_error(error_msg);
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::ServiceResumeFailed, error_msg.toStdString());
    }
}

QJsonObject AdvancedServicePlugin::service_configuration() const {
    QMutexLocker locker(&m_config_mutex);
    QJsonObject service_config;
    service_config["timer_interval"] = m_configuration.value("timer_interval");
    service_config["max_queue_size"] = m_configuration.value("max_queue_size");
    service_config["enable_monitoring"] = m_configuration.value("enable_monitoring");
    service_config["auto_start"] = m_configuration.value("auto_start");
    return service_config;
}

qtplugin::expected<void, qtplugin::PluginError>
AdvancedServicePlugin::configure_service(const QJsonObject& config) {
    return configure(config);
}

qtplugin::ServiceHealth AdvancedServicePlugin::service_health() const {
    qtplugin::ServiceHealth health;
    health.is_healthy = (m_service_state == qtplugin::ServiceState::Running ||
                        m_service_state == qtplugin::ServiceState::Paused);
    health.last_check = std::chrono::system_clock::now();

    // Calculate health score based on various factors
    double score = 1.0;

    // Reduce score based on error rate
    int total_tasks = m_metrics->tasks_processed.load();
    int failed_tasks = m_metrics->tasks_failed.load();
    if (total_tasks > 0) {
        double error_rate = static_cast<double>(failed_tasks) / total_tasks;
        score *= (1.0 - error_rate);
    }

    // Reduce score based on queue size
    {
        QMutexLocker locker(&m_queue_mutex);
        int queue_size = m_work_queue.size();
        int max_size = m_configuration.value("max_queue_size").toInt(DEFAULT_MAX_QUEUE_SIZE);
        if (queue_size > max_size * 0.8) {
            score *= 0.7; // Reduce score if queue is getting full
        }
    }

    health.health_score = score;
    health.status_message = health.is_healthy ? "Service is healthy" : "Service has issues";

    return health;
}

QJsonObject AdvancedServicePlugin::service_metrics() const {
    return create_metrics_response();
}

// === Private Slots ===

void AdvancedServicePlugin::on_timer_timeout() {
    if (m_service_state == qtplugin::ServiceState::Running) {
        process_work_queue();

        // Add some sample work items for demonstration
        static int demo_counter = 0;
        if (++demo_counter % 5 == 0) { // Every 5th timer tick
            QJsonObject demo_data;
            demo_data["demo_id"] = demo_counter;
            demo_data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            add_work_item("demo_task", demo_data);
        }
    }
}

void AdvancedServicePlugin::on_performance_timer_timeout() {
    update_performance_metrics();

    QJsonObject metrics = create_metrics_response();
    emit performance_metrics_updated(metrics);
}

void AdvancedServicePlugin::process_work_queue() {
    QMutexLocker locker(&m_queue_mutex);

    while (!m_work_queue.isEmpty()) {
        WorkItem item = m_work_queue.dequeue();
        locker.unlock();

        auto start_time = std::chrono::steady_clock::now();
        bool success = process_single_task(item);
        auto end_time = std::chrono::steady_clock::now();

        auto processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        if (success) {
            m_metrics->tasks_processed++;
            m_metrics->total_processing_time += processing_time;

            // Store processing time for average calculation
            {
                QMutexLocker metrics_locker(&m_metrics->metrics_mutex);
                m_metrics->processing_times.enqueue(processing_time);
                if (m_metrics->processing_times.size() > MAX_PROCESSING_TIMES_HISTORY) {
                    m_metrics->processing_times.dequeue();
                }
            }

            emit task_completed(item.id, processing_time);
        } else {
            m_metrics->tasks_failed++;
            emit task_failed(item.id, QString("Task processing failed"));
        }

        locker.relock();
        emit queue_size_changed(m_work_queue.size());
    }
}

// === Private Helper Methods ===

void AdvancedServicePlugin::log_info(const QString& message) {
    qDebug() << "[AdvancedServicePlugin]" << message;
}

void AdvancedServicePlugin::log_warning(const QString& message) {
    qWarning() << "[AdvancedServicePlugin]" << message;

    QMutexLocker locker(&m_error_mutex);
    m_error_log.append(QString("WARNING: %1").arg(message));
    if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
        m_error_log.removeFirst();
    }
}

void AdvancedServicePlugin::log_error(const QString& message) {
    qCritical() << "[AdvancedServicePlugin]" << message;

    QMutexLocker locker(&m_error_mutex);
    m_last_error = message;
    m_error_log.append(QString("ERROR: %1").arg(message));
    if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
        m_error_log.removeFirst();
    }
}

void AdvancedServicePlugin::update_performance_metrics() {
    // Update memory usage
    qint64 current_memory = get_memory_usage();
    qint64 peak_memory = m_metrics->peak_memory_usage.load();
    if (current_memory > peak_memory) {
        m_metrics->peak_memory_usage = current_memory;
    }
}

void AdvancedServicePlugin::reset_performance_metrics() {
    m_metrics->tasks_processed = 0;
    m_metrics->tasks_failed = 0;
    m_metrics->total_processing_time = 0;
    m_metrics->peak_memory_usage = 0;
    m_metrics->start_time = std::chrono::steady_clock::now();

    QMutexLocker locker(&m_metrics->metrics_mutex);
    m_metrics->processing_times.clear();
}

qint64 AdvancedServicePlugin::get_memory_usage() const {
    // Simple memory usage estimation
    // In a real implementation, you might use platform-specific APIs
    return QCoreApplication::instance()->property("memory_usage").toLongLong();
}

double AdvancedServicePlugin::get_cpu_usage() const {
    // Simple CPU usage estimation
    // In a real implementation, you might use platform-specific APIs
    return 0.5; // Placeholder value
}

bool AdvancedServicePlugin::process_single_task(const WorkItem& item) {
    try {
        // Simulate task processing
        QThread::msleep(10 + (item.id % 20)); // Variable processing time

        // Simulate occasional failures
        if (item.retry_count > 0 && (item.id % 10 == 0)) {
            return false; // Simulate failure
        }

        log_info(QString("Processed task %1 of type '%2'").arg(item.id).arg(item.type));
        return true;

    } catch (const std::exception& e) {
        log_error(QString("Task %1 failed: %2").arg(item.id).arg(e.what()));
        return false;
    }
}

void AdvancedServicePlugin::add_work_item(const QString& type, const QJsonObject& data) {
    QMutexLocker locker(&m_queue_mutex);

    int max_size = m_configuration.value("max_queue_size").toInt(DEFAULT_MAX_QUEUE_SIZE);
    if (m_work_queue.size() >= max_size) {
        log_warning(QString("Work queue is full (%1 items), dropping oldest item").arg(max_size));
        m_work_queue.dequeue();
    }

    WorkItem item;
    item.id = m_next_task_id++;
    item.type = type;
    item.data = data;
    item.created_at = std::chrono::steady_clock::now();
    item.retry_count = 0;

    m_work_queue.enqueue(item);
    emit queue_size_changed(m_work_queue.size());

    log_info(QString("Added work item %1 of type '%2'").arg(item.id).arg(type));
}

void AdvancedServicePlugin::clear_work_queue() {
    QMutexLocker locker(&m_queue_mutex);
    int cleared_count = m_work_queue.size();
    m_work_queue.clear();

    if (cleared_count > 0) {
        log_info(QString("Cleared %1 items from work queue").arg(cleared_count));
        emit queue_size_changed(0);
    }
}

void AdvancedServicePlugin::transition_to_state(qtplugin::ServiceState new_state) {
    qtplugin::ServiceState old_state = m_service_state.load();
    m_service_state = new_state;

    log_info(QString("Service state transition: %1 -> %2")
             .arg(static_cast<int>(old_state))
             .arg(static_cast<int>(new_state)));
}

void AdvancedServicePlugin::handle_service_error(const QString& error) {
    log_error(error);
    m_service_state = qtplugin::ServiceState::Error;
    emit service_error(error);
}

QJsonObject AdvancedServicePlugin::create_status_response() const {
    QJsonObject status;
    status["plugin_state"] = static_cast<int>(m_plugin_state.load());
    status["service_state"] = static_cast<int>(m_service_state.load());
    status["uptime_ms"] = m_uptime_timer.elapsed();
    status["is_running"] = is_service_running();

    {
        QMutexLocker locker(&m_queue_mutex);
        status["queue_size"] = m_work_queue.size();
    }

    status["last_error"] = m_last_error;
    status["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    return status;
}

QJsonObject AdvancedServicePlugin::create_metrics_response() const {
    QJsonObject metrics;

    // Basic metrics
    metrics["tasks_processed"] = m_metrics->tasks_processed.load();
    metrics["tasks_failed"] = m_metrics->tasks_failed.load();
    metrics["total_processing_time_ms"] = m_metrics->total_processing_time.load();

    // Calculate average processing time
    int processed = m_metrics->tasks_processed.load();
    if (processed > 0) {
        metrics["average_processing_time_ms"] =
            static_cast<double>(m_metrics->total_processing_time.load()) / processed;
    } else {
        metrics["average_processing_time_ms"] = 0.0;
    }

    // Memory and performance
    metrics["current_memory_usage"] = get_memory_usage();
    metrics["peak_memory_usage"] = m_metrics->peak_memory_usage.load();
    metrics["cpu_usage_percent"] = get_cpu_usage();

    // Uptime
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_metrics->start_time);
    metrics["uptime_ms"] = uptime.count();

    // Queue metrics
    {
        QMutexLocker locker(&m_queue_mutex);
        metrics["current_queue_size"] = m_work_queue.size();
        metrics["max_queue_size"] = m_configuration.value("max_queue_size").toInt(DEFAULT_MAX_QUEUE_SIZE);
    }

    // Error rate
    int total_tasks = processed + m_metrics->tasks_failed.load();
    if (total_tasks > 0) {
        metrics["error_rate"] = static_cast<double>(m_metrics->tasks_failed.load()) / total_tasks;
    } else {
        metrics["error_rate"] = 0.0;
    }

    return metrics;
}

QJsonObject AdvancedServicePlugin::create_health_response() const {
    auto health = service_health();

    QJsonObject response;
    response["is_healthy"] = health.is_healthy;
    response["health_score"] = health.health_score;
    response["status_message"] = QString::fromStdString(health.status_message);

    auto time_t = std::chrono::system_clock::to_time_t(health.last_check);
    response["last_check"] = QDateTime::fromSecsSinceEpoch(time_t).toString(Qt::ISODate);

    return response;
}

#include "service_plugin.moc"
