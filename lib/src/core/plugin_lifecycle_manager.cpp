/**
 * @file plugin_lifecycle_manager.cpp
 * @brief Implementation of advanced plugin lifecycle management
 * @version 3.0.0
 */

#include "qtplugin/core/plugin_lifecycle_manager.hpp"
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QTimer>
#include <QDateTime>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QUuid>
#include <algorithm>
#include <unordered_map>
#include <memory>

Q_LOGGING_CATEGORY(lifecycleLog, "qtplugin.lifecycle")

namespace qtplugin {

// === PluginLifecycleConfig Implementation ===

QJsonObject PluginLifecycleConfig::to_json() const {
    QJsonObject json;
    json["initialization_timeout"] = static_cast<qint64>(initialization_timeout.count());
    json["shutdown_timeout"] = static_cast<qint64>(shutdown_timeout.count());
    json["pause_timeout"] = static_cast<qint64>(pause_timeout.count());
    json["resume_timeout"] = static_cast<qint64>(resume_timeout.count());
    json["health_check_interval"] = static_cast<qint64>(health_check_interval.count());
    json["enable_graceful_shutdown"] = enable_graceful_shutdown;
    json["enable_health_monitoring"] = enable_health_monitoring;
    json["enable_resource_monitoring"] = enable_resource_monitoring;
    json["auto_restart_on_failure"] = auto_restart_on_failure;
    json["max_restart_attempts"] = max_restart_attempts;
    json["restart_delay"] = static_cast<qint64>(restart_delay.count());
    json["custom_config"] = custom_config;
    return json;
}

PluginLifecycleConfig PluginLifecycleConfig::from_json(const QJsonObject& json) {
    PluginLifecycleConfig config;
    config.initialization_timeout = std::chrono::milliseconds(json["initialization_timeout"].toInt(30000));
    config.shutdown_timeout = std::chrono::milliseconds(json["shutdown_timeout"].toInt(10000));
    config.pause_timeout = std::chrono::milliseconds(json["pause_timeout"].toInt(5000));
    config.resume_timeout = std::chrono::milliseconds(json["resume_timeout"].toInt(5000));
    config.health_check_interval = std::chrono::milliseconds(json["health_check_interval"].toInt(60000));
    config.enable_graceful_shutdown = json["enable_graceful_shutdown"].toBool(true);
    config.enable_health_monitoring = json["enable_health_monitoring"].toBool(true);
    config.enable_resource_monitoring = json["enable_resource_monitoring"].toBool(true);
    config.auto_restart_on_failure = json["auto_restart_on_failure"].toBool(false);
    config.max_restart_attempts = json["max_restart_attempts"].toInt(3);
    config.restart_delay = std::chrono::milliseconds(json["restart_delay"].toInt(5000));
    config.custom_config = json["custom_config"].toObject();
    return config;
}

// === PluginLifecycleEventData Implementation ===

QJsonObject PluginLifecycleEventData::to_json() const {
    QJsonObject json;
    json["plugin_id"] = plugin_id;
    json["event_type"] = static_cast<int>(event_type);
    json["old_state"] = static_cast<int>(old_state);
    json["new_state"] = static_cast<int>(new_state);
    json["timestamp"] = QDateTime::fromSecsSinceEpoch(
        std::chrono::duration_cast<std::chrono::seconds>(timestamp.time_since_epoch()).count())
        .toString(Qt::ISODate);
    json["message"] = message;
    json["metadata"] = metadata;
    
    if (error.has_value()) {
        QJsonObject error_json;
        error_json["code"] = static_cast<int>(error->code);
        error_json["message"] = QString::fromStdString(error->message);
        json["error"] = error_json;
    }
    
    return json;
}

PluginLifecycleEventData PluginLifecycleEventData::from_json(const QJsonObject& json) {
    PluginLifecycleEventData data;
    data.plugin_id = json["plugin_id"].toString();
    data.event_type = static_cast<PluginLifecycleEvent>(json["event_type"].toInt());
    data.old_state = static_cast<PluginState>(json["old_state"].toInt());
    data.new_state = static_cast<PluginState>(json["new_state"].toInt());
    
    auto timestamp_str = json["timestamp"].toString();
    auto datetime = QDateTime::fromString(timestamp_str, Qt::ISODate);
    data.timestamp = std::chrono::system_clock::from_time_t(datetime.toSecsSinceEpoch());
    
    data.message = json["message"].toString();
    data.metadata = json["metadata"].toObject();
    
    if (json.contains("error")) {
        auto error_json = json["error"].toObject();
        PluginError plugin_error;
        plugin_error.code = static_cast<PluginErrorCode>(error_json["code"].toInt());
        plugin_error.message = error_json["message"].toString().toStdString();
        data.error = plugin_error;
    }
    
    return data;
}

// === PluginHealthStatus Implementation ===

QJsonObject PluginHealthStatus::to_json() const {
    QJsonObject json;
    json["plugin_id"] = plugin_id;
    json["is_healthy"] = is_healthy;
    json["last_check"] = QDateTime::fromSecsSinceEpoch(
        std::chrono::duration_cast<std::chrono::seconds>(last_check.time_since_epoch()).count())
        .toString(Qt::ISODate);
    json["response_time"] = static_cast<qint64>(response_time.count());
    json["metrics"] = metrics;
    json["warnings"] = QJsonArray::fromStringList(warnings);
    json["errors"] = QJsonArray::fromStringList(errors);
    return json;
}

// === PluginLifecycleManager Private Implementation ===

struct PluginLifecycleInfo {
    std::shared_ptr<IPlugin> plugin;
    PluginLifecycleConfig config;
    std::unique_ptr<QStateMachine> state_machine;
    std::vector<PluginLifecycleEventData> event_history;
    PluginHealthStatus health_status;
    PluginHealthCheckCallback health_check_callback;
    std::unique_ptr<QTimer> health_check_timer;
    std::unique_ptr<QTimer> operation_timeout_timer;
    int restart_attempts = 0;
    std::chrono::system_clock::time_point last_restart_time;
    bool health_monitoring_enabled = false;
};

struct LifecycleEventCallback {
    QString id;
    QString plugin_id_filter;
    PluginLifecycleEvent event_type;
    PluginLifecycleEventCallback callback;
};

class PluginLifecycleManager::Private {
public:
    mutable QMutex mutex;
    PluginLifecycleConfig default_config;
    std::unordered_map<QString, std::unique_ptr<PluginLifecycleInfo>> plugins;
    std::unordered_map<QString, LifecycleEventCallback> event_callbacks;
    
    void create_state_machine(PluginLifecycleInfo* info);
    void emit_lifecycle_event(const PluginLifecycleEventData& event_data);
    void perform_health_check(const QString& plugin_id);
    void handle_plugin_error(const QString& plugin_id, const PluginError& error);
    bool should_auto_restart(const QString& plugin_id);
    void schedule_restart(const QString& plugin_id);
};

void PluginLifecycleManager::Private::create_state_machine(PluginLifecycleInfo* info) {
    if (!info || !info->plugin) return;
    
    info->state_machine = std::make_unique<QStateMachine>();
    
    // Create states
    auto* unloaded_state = new QState(info->state_machine.get());
    auto* loading_state = new QState(info->state_machine.get());
    auto* loaded_state = new QState(info->state_machine.get());
    auto* initializing_state = new QState(info->state_machine.get());
    auto* running_state = new QState(info->state_machine.get());
    auto* paused_state = new QState(info->state_machine.get());
    auto* stopping_state = new QState(info->state_machine.get());
    auto* stopped_state = new QState(info->state_machine.get());
    auto* error_state = new QState(info->state_machine.get());
    auto* reloading_state = new QState(info->state_machine.get());
    
    // Set initial state
    info->state_machine->setInitialState(unloaded_state);
    
    // Define transitions
    unloaded_state->addTransition(loading_state);
    loading_state->addTransition(loaded_state);
    loading_state->addTransition(error_state);
    loaded_state->addTransition(initializing_state);
    loaded_state->addTransition(error_state);
    initializing_state->addTransition(running_state);
    initializing_state->addTransition(error_state);
    running_state->addTransition(paused_state);
    running_state->addTransition(stopping_state);
    running_state->addTransition(error_state);
    paused_state->addTransition(running_state);
    paused_state->addTransition(stopping_state);
    paused_state->addTransition(error_state);
    stopping_state->addTransition(stopped_state);
    stopped_state->addTransition(unloaded_state);
    error_state->addTransition(reloading_state);
    reloading_state->addTransition(loaded_state);
    reloading_state->addTransition(error_state);
    
    // Start the state machine
    info->state_machine->start();
}

void PluginLifecycleManager::Private::emit_lifecycle_event(const PluginLifecycleEventData& event_data) {
    // Store event in history
    auto it = plugins.find(event_data.plugin_id);
    if (it != plugins.end()) {
        auto& history = it->second->event_history;
        history.push_back(event_data);
        
        // Limit history size
        const size_t max_history_size = 1000;
        if (history.size() > max_history_size) {
            history.erase(history.begin(), history.begin() + (history.size() - max_history_size));
        }
    }
    
    // Notify callbacks
    for (const auto& [callback_id, callback_info] : event_callbacks) {
        bool should_notify = false;
        
        // Check plugin filter
        if (callback_info.plugin_id_filter.isEmpty() || 
            callback_info.plugin_id_filter == event_data.plugin_id) {
            // Check event type filter
            if (callback_info.event_type == event_data.event_type) {
                should_notify = true;
            }
        }
        
        if (should_notify && callback_info.callback) {
            try {
                callback_info.callback(event_data);
            } catch (const std::exception& e) {
                qCWarning(lifecycleLog) << "Exception in lifecycle event callback:" << e.what();
            } catch (...) {
                qCWarning(lifecycleLog) << "Unknown exception in lifecycle event callback";
            }
        }
    }
}

void PluginLifecycleManager::Private::perform_health_check(const QString& plugin_id) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end() || !it->second->health_monitoring_enabled) {
        return;
    }
    
    auto& info = it->second;
    auto start_time = std::chrono::steady_clock::now();
    
    PluginHealthStatus health_status;
    health_status.plugin_id = plugin_id;
    health_status.last_check = std::chrono::system_clock::now();
    
    try {
        if (info->health_check_callback) {
            // Use custom health check
            health_status = info->health_check_callback(plugin_id);
        } else {
            // Default health check - just check if plugin is responsive
            health_status.is_healthy = (info->plugin->state() == PluginState::Running);
        }
        
        auto end_time = std::chrono::steady_clock::now();
        health_status.response_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
        
    } catch (const std::exception& e) {
        health_status.is_healthy = false;
        health_status.errors.push_back(QString("Health check exception: %1").arg(e.what()));
    } catch (...) {
        health_status.is_healthy = false;
        health_status.errors.push_back("Unknown health check exception");
    }
    
    // Update stored health status
    info->health_status = health_status;
    
    // Emit health change event if status changed
    static std::unordered_map<QString, bool> previous_health_status;
    bool previous_healthy = previous_health_status[plugin_id];
    
    if (previous_healthy != health_status.is_healthy) {
        previous_health_status[plugin_id] = health_status.is_healthy;
        
        PluginLifecycleEventData event_data;
        event_data.plugin_id = plugin_id;
        event_data.event_type = PluginLifecycleEvent::HealthCheck;
        event_data.old_state = info->plugin->state();
        event_data.new_state = info->plugin->state();
        event_data.timestamp = std::chrono::system_clock::now();
        event_data.message = health_status.is_healthy ? "Plugin is healthy" : "Plugin health check failed";
        event_data.metadata["health_status"] = health_status.to_json();
        
        emit_lifecycle_event(event_data);
    }
}

void PluginLifecycleManager::Private::handle_plugin_error(const QString& plugin_id, const PluginError& error) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end()) return;
    
    auto& info = it->second;
    
    // Create error event
    PluginLifecycleEventData event_data;
    event_data.plugin_id = plugin_id;
    event_data.event_type = PluginLifecycleEvent::Error;
    event_data.old_state = info->plugin->state();
    event_data.new_state = PluginState::Error;
    event_data.timestamp = std::chrono::system_clock::now();
    event_data.message = QString::fromStdString(error.message);
    event_data.error = error;
    
    emit_lifecycle_event(event_data);
    
    // Check if auto-restart is enabled and should be attempted
    if (should_auto_restart(plugin_id)) {
        schedule_restart(plugin_id);
    }
}

bool PluginLifecycleManager::Private::should_auto_restart(const QString& plugin_id) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end()) return false;
    
    auto& info = it->second;
    
    if (!info->config.auto_restart_on_failure) {
        return false;
    }
    
    if (info->restart_attempts >= info->config.max_restart_attempts) {
        return false;
    }
    
    // Check if enough time has passed since last restart
    auto now = std::chrono::system_clock::now();
    auto time_since_restart = now - info->last_restart_time;
    
    if (time_since_restart < info->config.restart_delay) {
        return false;
    }
    
    return true;
}

void PluginLifecycleManager::Private::schedule_restart(const QString& plugin_id) {
    auto it = plugins.find(plugin_id);
    if (it == plugins.end()) return;
    
    auto& info = it->second;
    
    // Create restart timer
    auto* restart_timer = new QTimer();
    restart_timer->setSingleShot(true);
    restart_timer->setInterval(static_cast<int>(info->config.restart_delay.count()));
    
    QObject::connect(restart_timer, &QTimer::timeout, [this, plugin_id, restart_timer]() {
        // Attempt restart
        auto it = plugins.find(plugin_id);
        if (it != plugins.end()) {
            auto& info = it->second;
            info->restart_attempts++;
            info->last_restart_time = std::chrono::system_clock::now();
            
            qCInfo(lifecycleLog) << "Attempting auto-restart for plugin:" << plugin_id
                                << "attempt:" << info->restart_attempts;
            
            // Try to restart the plugin
            try {
                info->plugin->shutdown();
                auto init_result = info->plugin->initialize();
                
                if (init_result) {
                    qCInfo(lifecycleLog) << "Auto-restart successful for plugin:" << plugin_id;
                    info->restart_attempts = 0; // Reset on success
                } else {
                    qCWarning(lifecycleLog) << "Auto-restart failed for plugin:" << plugin_id
                                           << "error:" << init_result.error().message.c_str();
                }
            } catch (const std::exception& e) {
                qCWarning(lifecycleLog) << "Exception during auto-restart for plugin:" << plugin_id
                                       << "error:" << e.what();
            }
        }
        
        restart_timer->deleteLater();
    });
    
    restart_timer->start();
}

// === PluginLifecycleManager Implementation ===

PluginLifecycleManager::PluginLifecycleManager(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>()) {
    
    // Set up default configuration
    d->default_config = PluginLifecycleConfig{};
    
    qCDebug(lifecycleLog) << "Plugin lifecycle manager initialized";
}

PluginLifecycleManager::~PluginLifecycleManager() = default;

qtplugin::expected<void, PluginError>
PluginLifecycleManager::register_plugin(std::shared_ptr<IPlugin> plugin,
                                       const PluginLifecycleConfig& config) {
    if (!plugin) {
        return make_error<void>(PluginErrorCode::InvalidArgument, "Plugin is null");
    }

    QString plugin_id = QString::fromStdString(plugin->id());

    QMutexLocker locker(&d->mutex);

    // Check if already registered
    if (d->plugins.find(plugin_id) != d->plugins.end()) {
        return make_error<void>(PluginErrorCode::AlreadyExists,
                               "Plugin already registered: " + plugin_id.toStdString());
    }

    // Create plugin lifecycle info
    auto info = std::make_unique<PluginLifecycleInfo>();
    info->plugin = plugin;
    info->config = config;
    info->health_status.plugin_id = plugin_id;
    info->health_status.last_check = std::chrono::system_clock::now();

    // Create state machine
    d->create_state_machine(info.get());

    // Set up health monitoring if enabled
    if (config.enable_health_monitoring) {
        info->health_check_timer = std::make_unique<QTimer>();
        info->health_check_timer->setInterval(static_cast<int>(config.health_check_interval.count()));

        connect(info->health_check_timer.get(), &QTimer::timeout, [this, plugin_id]() {
            d->perform_health_check(plugin_id);
        });

        info->health_monitoring_enabled = true;
        info->health_check_timer->start();
    }

    // Store plugin info
    d->plugins[plugin_id] = std::move(info);

    qCDebug(lifecycleLog) << "Registered plugin for lifecycle management:" << plugin_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginLifecycleManager::unregister_plugin(const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                               "Plugin not registered: " + plugin_id.toStdString());
    }

    // Stop health monitoring
    if (it->second->health_check_timer) {
        it->second->health_check_timer->stop();
    }

    // Stop state machine
    if (it->second->state_machine) {
        it->second->state_machine->stop();
    }

    // Remove plugin
    d->plugins.erase(it);

    qCDebug(lifecycleLog) << "Unregistered plugin from lifecycle management:" << plugin_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginLifecycleManager::initialize_plugin(const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                               "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    // Check current state
    PluginState current_state = plugin->state();
    if (current_state == PluginState::Running) {
        return make_success(); // Already initialized
    }

    // Emit before initialize event
    PluginLifecycleEventData before_event;
    before_event.plugin_id = plugin_id;
    before_event.event_type = PluginLifecycleEvent::BeforeInitialize;
    before_event.old_state = current_state;
    before_event.new_state = PluginState::Initializing;
    before_event.timestamp = std::chrono::system_clock::now();
    before_event.message = "Starting plugin initialization";

    d->emit_lifecycle_event(before_event);
    emit plugin_state_changed(plugin_id, current_state, PluginState::Initializing);

    // Set up timeout timer
    info->operation_timeout_timer = std::make_unique<QTimer>();
    info->operation_timeout_timer->setSingleShot(true);
    info->operation_timeout_timer->setInterval(static_cast<int>(info->config.initialization_timeout.count()));

    bool timeout_occurred = false;
    connect(info->operation_timeout_timer.get(), &QTimer::timeout, [&timeout_occurred, plugin_id, this]() {
        timeout_occurred = true;

        PluginLifecycleEventData timeout_event;
        timeout_event.plugin_id = plugin_id;
        timeout_event.event_type = PluginLifecycleEvent::Timeout;
        timeout_event.old_state = PluginState::Initializing;
        timeout_event.new_state = PluginState::Error;
        timeout_event.timestamp = std::chrono::system_clock::now();
        timeout_event.message = "Plugin initialization timeout";

        d->emit_lifecycle_event(timeout_event);
    });

    info->operation_timeout_timer->start();

    // Attempt initialization
    auto init_result = plugin->initialize();

    // Stop timeout timer
    info->operation_timeout_timer->stop();
    info->operation_timeout_timer.reset();

    // Handle result
    PluginLifecycleEventData after_event;
    after_event.plugin_id = plugin_id;
    after_event.event_type = PluginLifecycleEvent::AfterInitialize;
    after_event.old_state = PluginState::Initializing;
    after_event.timestamp = std::chrono::system_clock::now();

    if (timeout_occurred) {
        after_event.new_state = PluginState::Error;
        after_event.message = "Plugin initialization timed out";

        PluginError timeout_error;
        timeout_error.code = PluginErrorCode::Timeout;
        timeout_error.message = "Initialization timeout";
        after_event.error = timeout_error;

        d->emit_lifecycle_event(after_event);
        emit plugin_state_changed(plugin_id, PluginState::Initializing, PluginState::Error);

        return make_error<void>(PluginErrorCode::Timeout, "Plugin initialization timed out");
    }

    if (init_result) {
        after_event.new_state = PluginState::Running;
        after_event.message = "Plugin initialization successful";

        d->emit_lifecycle_event(after_event);
        emit plugin_state_changed(plugin_id, PluginState::Initializing, PluginState::Running);

        qCDebug(lifecycleLog) << "Successfully initialized plugin:" << plugin_id;

        return make_success();
    } else {
        after_event.new_state = PluginState::Error;
        after_event.message = QString::fromStdString(init_result.error().message);
        after_event.error = init_result.error();

        d->emit_lifecycle_event(after_event);
        emit plugin_state_changed(plugin_id, PluginState::Initializing, PluginState::Error);

        d->handle_plugin_error(plugin_id, init_result.error());

        return init_result;
    }
}

qtplugin::expected<void, PluginError>
PluginLifecycleManager::shutdown_plugin(const QString& plugin_id, bool force) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                               "Plugin not registered: " + plugin_id.toStdString());
    }

    auto& info = it->second;
    auto plugin = info->plugin;

    PluginState current_state = plugin->state();

    // Emit before shutdown event
    PluginLifecycleEventData before_event;
    before_event.plugin_id = plugin_id;
    before_event.event_type = PluginLifecycleEvent::BeforeShutdown;
    before_event.old_state = current_state;
    before_event.new_state = PluginState::Stopping;
    before_event.timestamp = std::chrono::system_clock::now();
    before_event.message = force ? "Starting forced plugin shutdown" : "Starting graceful plugin shutdown";

    d->emit_lifecycle_event(before_event);
    emit plugin_state_changed(plugin_id, current_state, PluginState::Stopping);

    // Perform shutdown
    try {
        if (!force && info->config.enable_graceful_shutdown) {
            // Set up timeout for graceful shutdown
            info->operation_timeout_timer = std::make_unique<QTimer>();
            info->operation_timeout_timer->setSingleShot(true);
            info->operation_timeout_timer->setInterval(static_cast<int>(info->config.shutdown_timeout.count()));

            bool timeout_occurred = false;
            connect(info->operation_timeout_timer.get(), &QTimer::timeout, [&timeout_occurred]() {
                timeout_occurred = true;
            });

            info->operation_timeout_timer->start();

            // Attempt graceful shutdown
            plugin->shutdown();

            info->operation_timeout_timer->stop();
            info->operation_timeout_timer.reset();

            if (timeout_occurred) {
                qCWarning(lifecycleLog) << "Graceful shutdown timed out for plugin:" << plugin_id
                                       << "forcing shutdown";
                // Force shutdown after timeout
                plugin->shutdown();
            }
        } else {
            // Force shutdown
            plugin->shutdown();
        }

        // Emit after shutdown event
        PluginLifecycleEventData after_event;
        after_event.plugin_id = plugin_id;
        after_event.event_type = PluginLifecycleEvent::AfterShutdown;
        after_event.old_state = PluginState::Stopping;
        after_event.new_state = PluginState::Stopped;
        after_event.timestamp = std::chrono::system_clock::now();
        after_event.message = "Plugin shutdown completed";

        d->emit_lifecycle_event(after_event);
        emit plugin_state_changed(plugin_id, PluginState::Stopping, PluginState::Stopped);

        qCDebug(lifecycleLog) << "Successfully shutdown plugin:" << plugin_id;

        return make_success();

    } catch (const std::exception& e) {
        PluginError error;
        error.code = PluginErrorCode::ExecutionFailed;
        error.message = std::string("Shutdown exception: ") + e.what();

        d->handle_plugin_error(plugin_id, error);

        return make_error<void>(error.code, error.message);
    } catch (...) {
        PluginError error;
        error.code = PluginErrorCode::ExecutionFailed;
        error.message = "Unknown shutdown exception";

        d->handle_plugin_error(plugin_id, error);

        return make_error<void>(error.code, error.message);
    }
}

} // namespace qtplugin
