/**
 * @file test_service_plugin_integration.cpp
 * @brief Integration tests for the Advanced Service Plugin
 */

#include <QtTest/QtTest>
#include <QObject>
#include <QSignalSpy>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <memory>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>

#include <qtplugin/qtplugin.hpp>
#include "../src/service_plugin.hpp"

/**
 * @brief Integration test suite for AdvancedServicePlugin
 */
class TestServicePluginIntegration : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Service lifecycle tests
    void testServiceLifecycle();
    void testServiceStateTransitions();
    void testServiceStartStop();
    void testServicePauseResume();
    void testServiceErrorHandling();

    // Configuration tests
    void testDefaultConfiguration();
    void testConfigurationValidation();
    void testRuntimeConfigurationUpdate();
    void testInvalidConfigurationHandling();
    void testConfigurationPersistence();

    // Performance monitoring tests
    void testPerformanceMetrics();
    void testMetricsCollection();
    void testPerformanceMonitoring();
    void testResourceTracking();

    // Work queue tests
    void testWorkQueueProcessing();
    void testTaskExecution();
    void testQueueManagement();
    void testTaskFailureHandling();
    void testQueueOverflow();

    // Command execution tests
    void testCommandExecution();
    void testCommandParameters();
    void testCommandValidation();
    void testCommandPerformance();

    // Signal/slot communication tests
    void testServiceSignals();
    void testTaskSignals();
    void testPerformanceSignals();
    void testErrorSignals();

    // Thread safety tests
    void testConcurrentOperations();
    void testConcurrentCommands();
    void testThreadSafeConfiguration();

    // Integration with PluginManager tests
    void testPluginManagerIntegration();
    void testPluginLoading();
    void testPluginUnloading();
    void testHotReloading();

    // Real-world scenario tests
    void testLongRunningService();
    void testHighLoadScenario();
    void testErrorRecovery();
    void testGracefulShutdown();

private:
    std::unique_ptr<AdvancedServicePlugin> m_plugin;
    std::unique_ptr<qtplugin::PluginManager> m_manager;
    std::unique_ptr<QTemporaryDir> m_temp_dir;
};

void TestServicePluginIntegration::initTestCase() {
    // Initialize QtPlugin library
    qtplugin::LibraryInitializer init;
    QVERIFY(init.is_initialized());
    
    qDebug() << "Starting AdvancedServicePlugin integration tests";
}

void TestServicePluginIntegration::cleanupTestCase() {
    qDebug() << "AdvancedServicePlugin integration tests completed";
}

void TestServicePluginIntegration::init() {
    m_plugin = std::make_unique<AdvancedServicePlugin>();
    m_manager = std::make_unique<qtplugin::PluginManager>();
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
}

void TestServicePluginIntegration::cleanup() {
    if (m_plugin) {
        if (m_plugin->state() != qtplugin::PluginState::Unloaded) {
            m_plugin->shutdown();
        }
        m_plugin.reset();
    }
    m_manager.reset();
    m_temp_dir.reset();
}

void TestServicePluginIntegration::testServiceLifecycle() {
    // Test complete service lifecycle
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Stopped);
    
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);
    
    // Start service
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Running);
    QVERIFY(m_plugin->is_service_running());
    
    // Stop service
    auto stop_result = m_plugin->stop_service();
    QVERIFY(stop_result.has_value());
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Stopped);
    QVERIFY(!m_plugin->is_service_running());
    
    // Shutdown plugin
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
}

void TestServicePluginIntegration::testServiceStateTransitions() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Running);
    
    // Test pause
    auto pause_result = m_plugin->pause_service();
    QVERIFY(pause_result.has_value());
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Paused);
    
    // Test resume
    auto resume_result = m_plugin->resume_service();
    QVERIFY(resume_result.has_value());
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Running);
    
    // Test stop
    auto stop_result = m_plugin->stop_service();
    QVERIFY(stop_result.has_value());
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Stopped);
}

void TestServicePluginIntegration::testServiceStartStop() {
    QSignalSpy started_spy(m_plugin.get(), &AdvancedServicePlugin::service_started);
    QSignalSpy stopped_spy(m_plugin.get(), &AdvancedServicePlugin::service_stopped);
    
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    // Start service
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    QCOMPARE(started_spy.count(), 1);
    
    // Stop service
    auto stop_result = m_plugin->stop_service();
    QVERIFY(stop_result.has_value());
    QCOMPARE(stopped_spy.count(), 1);
}

void TestServicePluginIntegration::testServicePauseResume() {
    QSignalSpy paused_spy(m_plugin.get(), &AdvancedServicePlugin::service_paused);
    QSignalSpy resumed_spy(m_plugin.get(), &AdvancedServicePlugin::service_resumed);
    
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Pause service
    auto pause_result = m_plugin->pause_service();
    QVERIFY(pause_result.has_value());
    QCOMPARE(paused_spy.count(), 1);
    
    // Resume service
    auto resume_result = m_plugin->resume_service();
    QVERIFY(resume_result.has_value());
    QCOMPARE(resumed_spy.count(), 1);
}

void TestServicePluginIntegration::testServiceErrorHandling() {
    QSignalSpy error_spy(m_plugin.get(), &AdvancedServicePlugin::service_error);
    
    // Test starting service without initialization
    auto start_result = m_plugin->start_service();
    QVERIFY(!start_result.has_value());
    QCOMPARE(start_result.error().code, qtplugin::PluginErrorCode::StateError);
    
    // Test pausing service that's not running
    auto pause_result = m_plugin->pause_service();
    QVERIFY(!pause_result.has_value());
    QCOMPARE(pause_result.error().code, qtplugin::PluginErrorCode::StateError);
    
    // Test resuming service that's not paused
    auto resume_result = m_plugin->resume_service();
    QVERIFY(!resume_result.has_value());
    QCOMPARE(resume_result.error().code, qtplugin::PluginErrorCode::StateError);
}

void TestServicePluginIntegration::testDefaultConfiguration() {
    auto default_config = m_plugin->default_configuration();
    QVERIFY(default_config.has_value());
    
    QJsonObject config = default_config.value();
    QVERIFY(config.contains("timer_interval"));
    QVERIFY(config.contains("max_queue_size"));
    QVERIFY(config.contains("enable_monitoring"));
    QVERIFY(config.contains("performance_tracking"));
    QVERIFY(config.contains("retry_policy"));
    
    // Verify default values
    QCOMPARE(config["timer_interval"].toInt(), 1000);
    QCOMPARE(config["max_queue_size"].toInt(), 100);
    QCOMPARE(config["enable_monitoring"].toBool(), true);
}

void TestServicePluginIntegration::testConfigurationValidation() {
    // Test valid configuration
    QJsonObject valid_config;
    valid_config["timer_interval"] = 2000;
    valid_config["max_queue_size"] = 200;
    valid_config["enable_monitoring"] = false;
    
    QVERIFY(m_plugin->validate_configuration(valid_config));
    
    auto config_result = m_plugin->configure(valid_config);
    QVERIFY(config_result.has_value());
    
    // Test invalid configuration
    QJsonObject invalid_config;
    invalid_config["timer_interval"] = 50; // Too low
    invalid_config["max_queue_size"] = 20000; // Too high
    
    QVERIFY(!m_plugin->validate_configuration(invalid_config));
    
    auto invalid_result = m_plugin->configure(invalid_config);
    QVERIFY(!invalid_result.has_value());
    QCOMPARE(invalid_result.error().code, qtplugin::PluginErrorCode::InvalidConfiguration);
}

void TestServicePluginIntegration::testRuntimeConfigurationUpdate() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Update configuration during runtime
    QJsonObject new_config;
    new_config["timer_interval"] = 500;
    new_config["max_queue_size"] = 50;
    
    auto config_result = m_plugin->configure(new_config);
    QVERIFY(config_result.has_value());
    
    // Verify configuration was applied
    auto current_config = m_plugin->current_configuration();
    QCOMPARE(current_config["timer_interval"].toInt(), 500);
    QCOMPARE(current_config["max_queue_size"].toInt(), 50);
}

void TestServicePluginIntegration::testInvalidConfigurationHandling() {
    QJsonObject invalid_configs[] = {
        {{"timer_interval", -1}},
        {{"max_queue_size", 0}},
        {{"performance_interval", 100}}, // Too low
        {{"log_level", "invalid_level"}}
    };
    
    for (const auto& config : invalid_configs) {
        auto result = m_plugin->configure(config);
        QVERIFY(!result.has_value());
        QCOMPARE(result.error().code, qtplugin::PluginErrorCode::InvalidConfiguration);
    }
}

void TestServicePluginIntegration::testConfigurationPersistence() {
    QJsonObject config1;
    config1["timer_interval"] = 1500;
    config1["custom_setting"] = "test_value";
    
    auto result1 = m_plugin->configure(config1);
    QVERIFY(result1.has_value());
    
    auto current1 = m_plugin->current_configuration();
    QCOMPARE(current1["timer_interval"].toInt(), 1500);
    QCOMPARE(current1["custom_setting"].toString(), "test_value");
    
    // Update with partial configuration
    QJsonObject config2;
    config2["timer_interval"] = 2000;
    
    auto result2 = m_plugin->configure(config2);
    QVERIFY(result2.has_value());
    
    auto current2 = m_plugin->current_configuration();
    QCOMPARE(current2["timer_interval"].toInt(), 2000);
    QCOMPARE(current2["custom_setting"].toString(), "test_value"); // Should persist
}

void TestServicePluginIntegration::testPerformanceMetrics() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Get initial metrics
    auto metrics_result = m_plugin->execute_command("metrics");
    QVERIFY(metrics_result.has_value());
    
    QJsonObject metrics = metrics_result.value();
    QVERIFY(metrics.contains("tasks_processed"));
    QVERIFY(metrics.contains("tasks_failed"));
    QVERIFY(metrics.contains("uptime_ms"));
    QVERIFY(metrics.contains("current_memory_usage"));
    QVERIFY(metrics.contains("error_rate"));
    
    // Initial values should be zero or reasonable defaults
    QCOMPARE(metrics["tasks_processed"].toInt(), 0);
    QCOMPARE(metrics["tasks_failed"].toInt(), 0);
    QVERIFY(metrics["uptime_ms"].toInt() >= 0);
}

void TestServicePluginIntegration::testMetricsCollection() {
    QSignalSpy metrics_spy(m_plugin.get(), &AdvancedServicePlugin::performance_metrics_updated);
    
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Wait for metrics update (performance timer should trigger)
    QVERIFY(metrics_spy.wait(6000)); // Wait up to 6 seconds for performance update
    
    QVERIFY(metrics_spy.count() >= 1);
    
    // Verify metrics signal contains valid data
    QList<QVariant> signal_args = metrics_spy.first();
    QJsonObject metrics = signal_args.at(0).toJsonObject();
    QVERIFY(metrics.contains("tasks_processed"));
    QVERIFY(metrics.contains("uptime_ms"));
}

void TestServicePluginIntegration::testPerformanceMonitoring() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Add some tasks to generate metrics
    for (int i = 0; i < 5; ++i) {
        QJsonObject params;
        params["type"] = "test_task";
        params["data"] = QJsonObject{{"id", i}};
        
        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }
    
    // Wait for tasks to be processed
    QTest::qWait(2000);
    
    // Check metrics
    auto metrics_result = m_plugin->execute_command("metrics");
    QVERIFY(metrics_result.has_value());
    
    QJsonObject metrics = metrics_result.value();
    QVERIFY(metrics["tasks_processed"].toInt() > 0);
    QVERIFY(metrics["total_processing_time_ms"].toInt() >= 0);
    QVERIFY(metrics["average_processing_time_ms"].toDouble() >= 0);
}

void TestServicePluginIntegration::testResourceTracking() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Get resource metrics
    auto metrics_result = m_plugin->execute_command("metrics");
    QVERIFY(metrics_result.has_value());
    
    QJsonObject metrics = metrics_result.value();
    QVERIFY(metrics.contains("current_memory_usage"));
    QVERIFY(metrics.contains("peak_memory_usage"));
    QVERIFY(metrics.contains("cpu_usage_percent"));
    
    // Values should be reasonable
    QVERIFY(metrics["current_memory_usage"].toInt() >= 0);
    QVERIFY(metrics["peak_memory_usage"].toInt() >= 0);
    QVERIFY(metrics["cpu_usage_percent"].toDouble() >= 0.0);
    QVERIFY(metrics["cpu_usage_percent"].toDouble() <= 100.0);
}

void TestServicePluginIntegration::testWorkQueueProcessing() {
    QSignalSpy task_completed_spy(m_plugin.get(), &AdvancedServicePlugin::task_completed);
    QSignalSpy queue_changed_spy(m_plugin.get(), &AdvancedServicePlugin::queue_size_changed);
    
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Add tasks to queue
    const int num_tasks = 3;
    for (int i = 0; i < num_tasks; ++i) {
        QJsonObject params;
        params["type"] = "test_task";
        params["data"] = QJsonObject{{"task_id", i}};
        
        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }
    
    // Wait for tasks to be processed
    QVERIFY(task_completed_spy.wait(5000));
    
    // Verify tasks were processed
    QVERIFY(task_completed_spy.count() >= 1);
    QVERIFY(queue_changed_spy.count() >= 1);
}

void TestServicePluginIntegration::testTaskExecution() {
    QSignalSpy task_completed_spy(m_plugin.get(), &AdvancedServicePlugin::task_completed);
    
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Add a task
    QJsonObject params;
    params["type"] = "test_task";
    params["data"] = QJsonObject{{"message", "Hello, World!"}};
    
    auto add_result = m_plugin->execute_command("add_task", params);
    QVERIFY(add_result.has_value());
    
    QJsonObject response = add_result.value();
    QVERIFY(response["success"].toBool());
    QVERIFY(response.contains("task_id"));
    
    // Wait for task completion
    QVERIFY(task_completed_spy.wait(3000));
    
    // Verify task completion signal
    QCOMPARE(task_completed_spy.count(), 1);
    QList<QVariant> signal_args = task_completed_spy.first();
    int task_id = signal_args.at(0).toInt();
    qint64 processing_time = signal_args.at(1).toLongLong();
    
    QVERIFY(task_id > 0);
    QVERIFY(processing_time >= 0);
}

void TestServicePluginIntegration::testQueueManagement() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Add multiple tasks
    const int num_tasks = 5;
    for (int i = 0; i < num_tasks; ++i) {
        QJsonObject params;
        params["type"] = "test_task";
        params["data"] = QJsonObject{{"id", i}};
        
        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }
    
    // Check queue status
    auto status_result = m_plugin->execute_command("status");
    QVERIFY(status_result.has_value());
    
    QJsonObject status = status_result.value();
    QVERIFY(status.contains("queue_size"));
    
    // Clear queue
    auto clear_result = m_plugin->execute_command("clear_queue");
    QVERIFY(clear_result.has_value());
    
    QJsonObject clear_response = clear_result.value();
    QVERIFY(clear_response["success"].toBool());
    
    // Verify queue is cleared
    auto status_after_clear = m_plugin->execute_command("status");
    QVERIFY(status_after_clear.has_value());
    
    QJsonObject status_cleared = status_after_clear.value();
    QCOMPARE(status_cleared["queue_size"].toInt(), 0);
}

void TestServicePluginIntegration::testTaskFailureHandling() {
    QSignalSpy task_failed_spy(m_plugin.get(), &AdvancedServicePlugin::task_failed);
    
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    
    // Add tasks that might fail (depending on implementation)
    for (int i = 0; i < 10; ++i) {
        QJsonObject params;
        params["type"] = "test_task";
        params["data"] = QJsonObject{{"id", i}};
        
        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }
    
    // Wait for processing
    QTest::qWait(3000);
    
    // Check if any tasks failed (this depends on the implementation)
    auto metrics_result = m_plugin->execute_command("metrics");
    QVERIFY(metrics_result.has_value());
    
    QJsonObject metrics = metrics_result.value();
    int tasks_processed = metrics["tasks_processed"].toInt();
    int tasks_failed = metrics["tasks_failed"].toInt();
    
    QVERIFY(tasks_processed >= 0);
    QVERIFY(tasks_failed >= 0);
    QVERIFY(tasks_processed + tasks_failed > 0); // Some tasks should have been processed
}

void TestServicePluginIntegration::testQueueOverflow() {
    // Configure small queue size
    QJsonObject config;
    config["max_queue_size"] = 3;
    config["timer_interval"] = 2000; // Slow processing to cause overflow

    auto config_result = m_plugin->configure(config);
    QVERIFY(config_result.has_value());

    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Add more tasks than queue can hold
    for (int i = 0; i < 10; ++i) {
        QJsonObject params;
        params["type"] = "test_task";
        params["data"] = QJsonObject{{"id", i}};

        auto result = m_plugin->execute_command("add_task", params);
        // All should succeed, but queue should drop oldest items
        QVERIFY(result.has_value());
    }

    // Check queue size doesn't exceed maximum
    auto status_result = m_plugin->execute_command("status");
    QVERIFY(status_result.has_value());

    QJsonObject status = status_result.value();
    QVERIFY(status["queue_size"].toInt() <= 3);
}

void TestServicePluginIntegration::testCommandExecution() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test all available commands
    auto commands = m_plugin->available_commands();
    QVERIFY(!commands.empty());

    for (const auto& command : commands) {
        if (command == "start" || command == "stop" || command == "pause" || command == "resume") {
            // Skip lifecycle commands for this test
            continue;
        }

        auto result = m_plugin->execute_command(command);
        QVERIFY(result.has_value());

        QJsonObject response = result.value();
        // Each command should return some response
        QVERIFY(!response.isEmpty());
    }
}

void TestServicePluginIntegration::testCommandParameters() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Test configure command with parameters
    QJsonObject config_params;
    config_params["timer_interval"] = 1500;
    config_params["enable_monitoring"] = false;

    auto config_result = m_plugin->execute_command("configure", config_params);
    QVERIFY(config_result.has_value());

    QJsonObject config_response = config_result.value();
    QVERIFY(config_response["success"].toBool());

    // Test add_task command with parameters
    QJsonObject task_params;
    task_params["type"] = "parameterized_task";
    task_params["data"] = QJsonObject{{"param1", "value1"}, {"param2", 42}};

    auto task_result = m_plugin->execute_command("add_task", task_params);
    QVERIFY(task_result.has_value());

    QJsonObject task_response = task_result.value();
    QVERIFY(task_response["success"].toBool());
    QVERIFY(task_response.contains("task_id"));
}

void TestServicePluginIntegration::testCommandValidation() {
    // Test unknown command
    auto unknown_result = m_plugin->execute_command("unknown_command");
    QVERIFY(!unknown_result.has_value());
    QCOMPARE(unknown_result.error().code, qtplugin::PluginErrorCode::CommandNotFound);

    // Test command with invalid parameters
    QJsonObject invalid_params;
    invalid_params["invalid_param"] = "invalid_value";

    auto invalid_result = m_plugin->execute_command("configure", invalid_params);
    QVERIFY(!invalid_result.has_value());
    QCOMPARE(invalid_result.error().code, qtplugin::PluginErrorCode::InvalidConfiguration);
}

void TestServicePluginIntegration::testCommandPerformance() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    const int num_commands = 1000;
    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < num_commands; ++i) {
        auto result = m_plugin->execute_command("status");
        QVERIFY(result.has_value());
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    qDebug() << "Command performance:" << duration.count() << "ms for" << num_commands << "commands";
    qDebug() << "Average per command:" << (static_cast<double>(duration.count()) / num_commands) << "ms";

    // Commands should be fast (less than 1ms per command on average)
    QVERIFY(static_cast<double>(duration.count()) / num_commands < 1.0);
}

void TestServicePluginIntegration::testServiceSignals() {
    QSignalSpy started_spy(m_plugin.get(), &AdvancedServicePlugin::service_started);
    QSignalSpy stopped_spy(m_plugin.get(), &AdvancedServicePlugin::service_stopped);
    QSignalSpy paused_spy(m_plugin.get(), &AdvancedServicePlugin::service_paused);
    QSignalSpy resumed_spy(m_plugin.get(), &AdvancedServicePlugin::service_resumed);
    QSignalSpy error_spy(m_plugin.get(), &AdvancedServicePlugin::service_error);

    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test service lifecycle signals
    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());
    QCOMPARE(started_spy.count(), 1);

    auto pause_result = m_plugin->pause_service();
    QVERIFY(pause_result.has_value());
    QCOMPARE(paused_spy.count(), 1);

    auto resume_result = m_plugin->resume_service();
    QVERIFY(resume_result.has_value());
    QCOMPARE(resumed_spy.count(), 1);

    auto stop_result = m_plugin->stop_service();
    QVERIFY(stop_result.has_value());
    QCOMPARE(stopped_spy.count(), 1);

    // Error signals should not have been emitted
    QCOMPARE(error_spy.count(), 0);
}

void TestServicePluginIntegration::testTaskSignals() {
    QSignalSpy task_completed_spy(m_plugin.get(), &AdvancedServicePlugin::task_completed);
    QSignalSpy task_failed_spy(m_plugin.get(), &AdvancedServicePlugin::task_failed);
    QSignalSpy queue_changed_spy(m_plugin.get(), &AdvancedServicePlugin::queue_size_changed);

    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Add tasks
    for (int i = 0; i < 3; ++i) {
        QJsonObject params;
        params["type"] = "signal_test_task";
        params["data"] = QJsonObject{{"id", i}};

        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }

    // Wait for task processing
    QVERIFY(task_completed_spy.wait(5000));

    // Verify signals were emitted
    QVERIFY(task_completed_spy.count() >= 1);
    QVERIFY(queue_changed_spy.count() >= 1);

    // Verify signal parameters
    if (task_completed_spy.count() > 0) {
        QList<QVariant> args = task_completed_spy.first();
        QCOMPARE(args.size(), 2);
        QVERIFY(args.at(0).toInt() > 0); // task_id
        QVERIFY(args.at(1).toLongLong() >= 0); // processing_time
    }
}

void TestServicePluginIntegration::testPerformanceSignals() {
    QSignalSpy metrics_spy(m_plugin.get(), &AdvancedServicePlugin::performance_metrics_updated);

    // Configure short performance interval for testing
    QJsonObject config;
    config["performance_interval"] = 1000; // 1 second

    auto config_result = m_plugin->configure(config);
    QVERIFY(config_result.has_value());

    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Wait for performance metrics update
    QVERIFY(metrics_spy.wait(2000));

    QVERIFY(metrics_spy.count() >= 1);

    // Verify metrics signal contains valid data
    QList<QVariant> args = metrics_spy.first();
    QCOMPARE(args.size(), 1);

    QJsonObject metrics = args.at(0).toJsonObject();
    QVERIFY(metrics.contains("tasks_processed"));
    QVERIFY(metrics.contains("uptime_ms"));
    QVERIFY(metrics.contains("current_memory_usage"));
}

void TestServicePluginIntegration::testErrorSignals() {
    QSignalSpy error_spy(m_plugin.get(), &AdvancedServicePlugin::service_error);

    // Try to start service without initialization (should cause error)
    auto start_result = m_plugin->start_service();
    QVERIFY(!start_result.has_value());

    // Error signal might be emitted depending on implementation
    // This test verifies the signal mechanism works

    // Initialize and start service properly
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_success = m_plugin->start_service();
    QVERIFY(start_success.has_value());

    // No error signals should be emitted for successful operations
    QCOMPARE(error_spy.count(), 0);
}

void TestServicePluginIntegration::testConcurrentOperations() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    const int num_threads = 4;
    const int operations_per_thread = 10;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Launch concurrent operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, operations_per_thread, &success_count, &failure_count]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                // Mix different operations
                if (i % 3 == 0) {
                    auto result = m_plugin->execute_command("status");
                    if (result.has_value()) success_count++; else failure_count++;
                } else if (i % 3 == 1) {
                    auto result = m_plugin->execute_command("metrics");
                    if (result.has_value()) success_count++; else failure_count++;
                } else {
                    QJsonObject params;
                    params["type"] = "concurrent_task";
                    params["data"] = QJsonObject{{"thread_op", i}};

                    auto result = m_plugin->execute_command("add_task", params);
                    if (result.has_value()) success_count++; else failure_count++;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    qDebug() << "Concurrent operations: success =" << success_count.load()
             << ", failures =" << failure_count.load();

    // Most operations should succeed
    QVERIFY(success_count.load() > failure_count.load());
    QCOMPARE(success_count.load() + failure_count.load(), num_threads * operations_per_thread);
}

void TestServicePluginIntegration::testConcurrentCommands() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    const int num_concurrent_commands = 50;
    std::vector<std::thread> threads;
    std::atomic<int> completed{0};

    for (int i = 0; i < num_concurrent_commands; ++i) {
        threads.emplace_back([this, i, &completed]() {
            auto result = m_plugin->execute_command("status");
            if (result.has_value()) {
                completed++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    QCOMPARE(completed.load(), num_concurrent_commands);
}

void TestServicePluginIntegration::testThreadSafeConfiguration() {
    // Initialize plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    const int num_config_threads = 3;
    std::vector<std::thread> threads;
    std::atomic<int> config_success{0};

    for (int t = 0; t < num_config_threads; ++t) {
        threads.emplace_back([this, t, &config_success]() {
            QJsonObject config;
            config["timer_interval"] = 1000 + (t * 100);
            config["test_setting"] = QString("thread_%1").arg(t);

            auto result = m_plugin->configure(config);
            if (result.has_value()) {
                config_success++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    QCOMPARE(config_success.load(), num_config_threads);

    // Verify final configuration is valid
    auto final_config = m_plugin->current_configuration();
    QVERIFY(final_config.contains("timer_interval"));
    QVERIFY(final_config.contains("test_setting"));
}

void TestServicePluginIntegration::testPluginManagerIntegration() {
    // This test would require actual plugin loading through PluginManager
    // For now, we test the plugin as a standalone object

    QVERIFY(m_manager != nullptr);
    QCOMPARE(m_manager->plugin_count(), 0);

    // Test plugin metadata
    QCOMPARE(m_plugin->name(), "Advanced Service Plugin");
    QCOMPARE(m_plugin->id(), "com.example.advanced_service");
    QVERIFY(m_plugin->capabilities() & qtplugin::PluginCapability::Service);
    QVERIFY(m_plugin->capabilities() & qtplugin::PluginCapability::Configuration);
    QVERIFY(m_plugin->capabilities() & qtplugin::PluginCapability::Monitoring);
}

void TestServicePluginIntegration::testPluginLoading() {
    // Test plugin loading simulation
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);

    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);

    // Plugin should be functional after loading
    auto commands = m_plugin->available_commands();
    QVERIFY(!commands.empty());

    auto status_result = m_plugin->execute_command("status");
    QVERIFY(status_result.has_value());
}

void TestServicePluginIntegration::testPluginUnloading() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Shutdown should clean up everything
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Stopped);
}

void TestServicePluginIntegration::testHotReloading() {
    // Initialize and configure plugin
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    QJsonObject config;
    config["timer_interval"] = 1500;
    config["custom_setting"] = "before_reload";

    auto config_result = m_plugin->configure(config);
    QVERIFY(config_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Simulate hot reload by shutdown and reinitialize
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);

    // Reinitialize (simulating reload)
    auto reinit_result = m_plugin->initialize();
    QVERIFY(reinit_result.has_value());

    // Plugin should be functional again
    auto status_result = m_plugin->execute_command("status");
    QVERIFY(status_result.has_value());
}

void TestServicePluginIntegration::testLongRunningService() {
    QSignalSpy task_completed_spy(m_plugin.get(), &AdvancedServicePlugin::task_completed);
    QSignalSpy metrics_spy(m_plugin.get(), &AdvancedServicePlugin::performance_metrics_updated);

    // Configure for long running test
    QJsonObject config;
    config["timer_interval"] = 500;
    config["performance_interval"] = 1000;

    auto config_result = m_plugin->configure(config);
    QVERIFY(config_result.has_value());

    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Add tasks periodically
    QTimer task_timer;
    int tasks_added = 0;
    connect(&task_timer, &QTimer::timeout, [this, &tasks_added]() {
        QJsonObject params;
        params["type"] = "long_running_task";
        params["data"] = QJsonObject{{"id", tasks_added++}};

        m_plugin->execute_command("add_task", params);
    });

    task_timer.start(200); // Add task every 200ms

    // Run for 5 seconds
    QTest::qWait(5000);
    task_timer.stop();

    // Verify service remained stable
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Running);
    QVERIFY(task_completed_spy.count() > 0);
    QVERIFY(metrics_spy.count() > 0);

    // Check final metrics
    auto metrics_result = m_plugin->execute_command("metrics");
    QVERIFY(metrics_result.has_value());

    QJsonObject metrics = metrics_result.value();
    QVERIFY(metrics["tasks_processed"].toInt() > 0);
    QVERIFY(metrics["uptime_ms"].toInt() >= 4500); // At least 4.5 seconds
}

void TestServicePluginIntegration::testHighLoadScenario() {
    // Configure for high load
    QJsonObject config;
    config["timer_interval"] = 100; // Fast processing
    config["max_queue_size"] = 1000; // Large queue

    auto config_result = m_plugin->configure(config);
    QVERIFY(config_result.has_value());

    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Add many tasks quickly
    const int num_tasks = 100;
    for (int i = 0; i < num_tasks; ++i) {
        QJsonObject params;
        params["type"] = "high_load_task";
        params["data"] = QJsonObject{{"id", i}};

        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }

    // Wait for processing
    QTest::qWait(3000);

    // Verify service handled the load
    auto metrics_result = m_plugin->execute_command("metrics");
    QVERIFY(metrics_result.has_value());

    QJsonObject metrics = metrics_result.value();
    int processed = metrics["tasks_processed"].toInt();
    int failed = metrics["tasks_failed"].toInt();

    qDebug() << "High load results: processed =" << processed << ", failed =" << failed;

    // Most tasks should be processed successfully
    QVERIFY(processed > 0);
    QVERIFY(processed >= failed); // More success than failures
}

void TestServicePluginIntegration::testErrorRecovery() {
    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Simulate error condition by invalid configuration
    QJsonObject invalid_config;
    invalid_config["timer_interval"] = -1; // Invalid

    auto invalid_result = m_plugin->configure(invalid_config);
    QVERIFY(!invalid_result.has_value());

    // Service should still be running
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Running);

    // Should still be able to execute commands
    auto status_result = m_plugin->execute_command("status");
    QVERIFY(status_result.has_value());

    // Apply valid configuration to recover
    QJsonObject valid_config;
    valid_config["timer_interval"] = 1000;

    auto valid_result = m_plugin->configure(valid_config);
    QVERIFY(valid_result.has_value());

    // Service should continue working normally
    auto final_status = m_plugin->execute_command("status");
    QVERIFY(final_status.has_value());
}

void TestServicePluginIntegration::testGracefulShutdown() {
    QSignalSpy stopped_spy(m_plugin.get(), &AdvancedServicePlugin::service_stopped);

    // Initialize and start service
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());

    auto start_result = m_plugin->start_service();
    QVERIFY(start_result.has_value());

    // Add some tasks
    for (int i = 0; i < 5; ++i) {
        QJsonObject params;
        params["type"] = "shutdown_test_task";
        params["data"] = QJsonObject{{"id", i}};

        auto result = m_plugin->execute_command("add_task", params);
        QVERIFY(result.has_value());
    }

    // Stop service gracefully
    auto stop_result = m_plugin->stop_service();
    QVERIFY(stop_result.has_value());
    QCOMPARE(stopped_spy.count(), 1);

    // Verify service is stopped
    QCOMPARE(m_plugin->service_state(), qtplugin::ServiceState::Stopped);

    // Shutdown plugin
    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);

    // No exceptions or crashes should occur
}

#include "test_service_plugin_integration.moc"

QTEST_MAIN(TestServicePluginIntegration)
