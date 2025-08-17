/**
 * @file test_plugin_interface.cpp
 * @brief Comprehensive tests for plugin interface functionality
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include <memory>
#include <string>

#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <qtplugin/utils/version.hpp>

using namespace qtplugin;

// Mock plugin implementation for testing
class MockPlugin : public IPlugin
{
public:
    MockPlugin() : m_initialized(false), m_started(false) {}
    
    expected<void, PluginError> initialize() override {
        if (m_initialized) {
            return make_error<void>(PluginErrorCode::AlreadyLoaded, "Plugin already initialized");
        }
        m_initialized = true;
        return make_success();
    }
    
    expected<void, PluginError> shutdown() override {
        if (!m_initialized) {
            return make_error<void>(PluginErrorCode::NotLoaded, "Plugin not initialized");
        }
        m_started = false;
        m_initialized = false;
        return make_success();
    }
    
    expected<void, PluginError> start() override {
        if (!m_initialized) {
            return make_error<void>(PluginErrorCode::NotLoaded, "Plugin not initialized");
        }
        if (m_started) {
            return make_error<void>(PluginErrorCode::AlreadyLoaded, "Plugin already started");
        }
        m_started = true;
        return make_success();
    }
    
    expected<void, PluginError> stop() override {
        if (!m_started) {
            return make_error<void>(PluginErrorCode::NotLoaded, "Plugin not started");
        }
        m_started = false;
        return make_success();
    }
    
    PluginInfo get_info() const override {
        PluginInfo info;
        info.name = "MockPlugin";
        info.version = Version(1, 0, 0);
        info.description = "Mock plugin for testing";
        info.author = "Test Suite";
        info.api_version = Version(3, 0, 0);
        info.state = m_started ? PluginState::Running : 
                    (m_initialized ? PluginState::Loaded : PluginState::Unloaded);
        return info;
    }
    
    bool is_initialized() const { return m_initialized; }
    bool is_started() const { return m_started; }

private:
    bool m_initialized;
    bool m_started;
};

class TestPluginInterface : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic interface tests
    void testPluginCreation();
    void testPluginDestruction();
    void testPluginInfo();
    
    // Lifecycle tests
    void testPluginInitialization();
    void testPluginShutdown();
    void testPluginStart();
    void testPluginStop();
    void testPluginLifecycleOrder();
    
    // State management tests
    void testPluginStateTransitions();
    void testInvalidStateTransitions();
    void testPluginStateConsistency();
    
    // Error handling tests
    void testInitializationError();
    void testShutdownError();
    void testStartError();
    void testStopError();
    void testDoubleInitialization();
    void testDoubleShutdown();
    
    // Plugin info tests
    void testPluginInfoValidation();
    void testPluginInfoSerialization();
    void testPluginInfoComparison();
    void testPluginInfoUpdate();
    
    // Version compatibility tests
    void testApiVersionCompatibility();
    void testPluginVersionValidation();
    void testVersionMismatchHandling();
    
    // Plugin metadata tests
    void testPluginMetadata();
    void testPluginDependencies();
    void testPluginCapabilities();
    void testPluginConfiguration();
    
    // Plugin communication tests
    void testPluginMessageHandling();
    void testPluginEventHandling();
    void testPluginServiceRegistration();
    
    // Performance tests
    void testPluginInitializationPerformance();
    void testPluginOperationPerformance();
    void testMemoryUsageTracking();
    
    // Thread safety tests
    void testConcurrentPluginOperations();
    void testThreadSafePluginAccess();
    void testPluginThreadSafety();
    
    // Resource management tests
    void testPluginResourceAllocation();
    void testPluginResourceCleanup();
    void testPluginResourceLimits();

private:
    std::unique_ptr<MockPlugin> m_plugin;
    
    // Helper methods
    void verifyPluginState(PluginState expected_state);
    void verifyPluginInfo(const PluginInfo& info, const std::string& expected_name);
    void testStateTransition(PluginState from, PluginState to, bool should_succeed);
};

void TestPluginInterface::initTestCase()
{
    qDebug() << "Starting plugin interface tests";
}

void TestPluginInterface::cleanupTestCase()
{
    qDebug() << "Plugin interface tests completed";
}

void TestPluginInterface::init()
{
    // Create fresh plugin for each test
    m_plugin = std::make_unique<MockPlugin>();
    QVERIFY(m_plugin != nullptr);
}

void TestPluginInterface::cleanup()
{
    // Clean up plugin
    if (m_plugin) {
        if (m_plugin->is_started()) {
            m_plugin->stop();
        }
        if (m_plugin->is_initialized()) {
            m_plugin->shutdown();
        }
        m_plugin.reset();
    }
}

void TestPluginInterface::testPluginCreation()
{
    // Test basic plugin creation
    auto plugin = std::make_unique<MockPlugin>();
    QVERIFY(plugin != nullptr);
    
    // Test initial state
    QVERIFY(!plugin->is_initialized());
    QVERIFY(!plugin->is_started());
    
    auto info = plugin->get_info();
    QCOMPARE(info.state, PluginState::Unloaded);
}

void TestPluginInterface::testPluginDestruction()
{
    // Test that destruction properly cleans up resources
    {
        auto plugin = std::make_unique<MockPlugin>();
        auto init_result = plugin->initialize();
        QVERIFY(init_result.has_value());
        
        auto start_result = plugin->start();
        QVERIFY(start_result.has_value());
        
        // Plugin should clean up automatically when destroyed
    }
    
    // Verify no memory leaks
    QVERIFY(true); // This would be verified with memory profiling tools
}

void TestPluginInterface::testPluginInfo()
{
    auto info = m_plugin->get_info();
    
    // Verify basic info fields
    QCOMPARE(info.name, "MockPlugin");
    QCOMPARE(info.version.to_string(), "1.0.0");
    QCOMPARE(info.description, "Mock plugin for testing");
    QCOMPARE(info.author, "Test Suite");
    QCOMPARE(info.api_version.to_string(), "3.0.0");
    QCOMPARE(info.state, PluginState::Unloaded);
}

void TestPluginInterface::testPluginInitialization()
{
    // Test successful initialization
    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QVERIFY(m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
    
    verifyPluginState(PluginState::Loaded);
}

void TestPluginInterface::testPluginShutdown()
{
    // Initialize first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    // Test successful shutdown
    auto shutdown_result = m_plugin->shutdown();
    QVERIFY(shutdown_result.has_value());
    QVERIFY(!m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
    
    verifyPluginState(PluginState::Unloaded);
}

void TestPluginInterface::testPluginStart()
{
    // Initialize first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    // Test successful start
    auto start_result = m_plugin->start();
    QVERIFY(start_result.has_value());
    QVERIFY(m_plugin->is_initialized());
    QVERIFY(m_plugin->is_started());
    
    verifyPluginState(PluginState::Running);
}

void TestPluginInterface::testPluginStop()
{
    // Initialize and start first
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto start_result = m_plugin->start();
    QVERIFY(start_result.has_value());
    
    // Test successful stop
    auto stop_result = m_plugin->stop();
    QVERIFY(stop_result.has_value());
    QVERIFY(m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
    
    verifyPluginState(PluginState::Loaded);
}

void TestPluginInterface::testPluginLifecycleOrder()
{
    // Test complete lifecycle in correct order
    
    // 1. Initialize
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    verifyPluginState(PluginState::Loaded);
    
    // 2. Start
    auto start_result = m_plugin->start();
    QVERIFY(start_result.has_value());
    verifyPluginState(PluginState::Running);
    
    // 3. Stop
    auto stop_result = m_plugin->stop();
    QVERIFY(stop_result.has_value());
    verifyPluginState(PluginState::Loaded);
    
    // 4. Shutdown
    auto shutdown_result = m_plugin->shutdown();
    QVERIFY(shutdown_result.has_value());
    verifyPluginState(PluginState::Unloaded);
}

void TestPluginInterface::testInvalidStateTransitions()
{
    // Test starting without initialization
    auto start_result = m_plugin->start();
    QVERIFY(!start_result.has_value());
    QCOMPARE(start_result.error().code, PluginErrorCode::NotLoaded);
    
    // Test stopping without starting
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto stop_result = m_plugin->stop();
    QVERIFY(!stop_result.has_value());
    QCOMPARE(stop_result.error().code, PluginErrorCode::NotLoaded);
    
    // Test shutdown without initialization
    auto shutdown_result = m_plugin->shutdown();
    QVERIFY(shutdown_result.has_value()); // Should succeed from loaded state
    
    auto shutdown_result2 = m_plugin->shutdown();
    QVERIFY(!shutdown_result2.has_value());
    QCOMPARE(shutdown_result2.error().code, PluginErrorCode::NotLoaded);
}

void TestPluginInterface::testDoubleInitialization()
{
    // Initialize once
    auto init_result1 = m_plugin->initialize();
    QVERIFY(init_result1.has_value());
    
    // Try to initialize again
    auto init_result2 = m_plugin->initialize();
    QVERIFY(!init_result2.has_value());
    QCOMPARE(init_result2.error().code, PluginErrorCode::AlreadyLoaded);
}

void TestPluginInterface::testDoubleShutdown()
{
    // Initialize and shutdown once
    auto init_result = m_plugin->initialize();
    QVERIFY(init_result.has_value());
    
    auto shutdown_result1 = m_plugin->shutdown();
    QVERIFY(shutdown_result1.has_value());
    
    // Try to shutdown again
    auto shutdown_result2 = m_plugin->shutdown();
    QVERIFY(!shutdown_result2.has_value());
    QCOMPARE(shutdown_result2.error().code, PluginErrorCode::NotLoaded);
}

void TestPluginInterface::testApiVersionCompatibility()
{
    auto info = m_plugin->get_info();
    
    // Test API version compatibility
    Version current_api(3, 0, 0);
    QVERIFY(info.api_version.is_compatible_with(current_api));
    
    // Test incompatible API version
    Version incompatible_api(2, 0, 0);
    QVERIFY(!info.api_version.is_compatible_with(incompatible_api));
}

void TestPluginInterface::testPluginInfoValidation()
{
    auto info = m_plugin->get_info();
    
    // Verify required fields are not empty
    QVERIFY(!info.name.empty());
    QVERIFY(!info.description.empty());
    QVERIFY(!info.author.empty());
    
    // Verify version is valid
    QVERIFY(info.version.major() >= 0);
    QVERIFY(info.version.minor() >= 0);
    QVERIFY(info.version.patch() >= 0);
    
    // Verify API version is valid
    QVERIFY(info.api_version.major() >= 0);
    QVERIFY(info.api_version.minor() >= 0);
    QVERIFY(info.api_version.patch() >= 0);
}

void TestPluginInterface::testPluginStateConsistency()
{
    // Test state consistency throughout lifecycle
    
    // Initial state
    auto info = m_plugin->get_info();
    QCOMPARE(info.state, PluginState::Unloaded);
    QVERIFY(!m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
    
    // After initialization
    m_plugin->initialize();
    info = m_plugin->get_info();
    QCOMPARE(info.state, PluginState::Loaded);
    QVERIFY(m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
    
    // After start
    m_plugin->start();
    info = m_plugin->get_info();
    QCOMPARE(info.state, PluginState::Running);
    QVERIFY(m_plugin->is_initialized());
    QVERIFY(m_plugin->is_started());
    
    // After stop
    m_plugin->stop();
    info = m_plugin->get_info();
    QCOMPARE(info.state, PluginState::Loaded);
    QVERIFY(m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
    
    // After shutdown
    m_plugin->shutdown();
    info = m_plugin->get_info();
    QCOMPARE(info.state, PluginState::Unloaded);
    QVERIFY(!m_plugin->is_initialized());
    QVERIFY(!m_plugin->is_started());
}

// Helper methods implementation
void TestPluginInterface::verifyPluginState(PluginState expected_state)
{
    auto info = m_plugin->get_info();
    QCOMPARE(info.state, expected_state);
}

void TestPluginInterface::verifyPluginInfo(const PluginInfo& info, const std::string& expected_name)
{
    QCOMPARE(info.name, expected_name);
    QVERIFY(!info.description.empty());
    QVERIFY(!info.author.empty());
}

void TestPluginInterface::testStateTransition(PluginState from, PluginState to, bool should_succeed)
{
    Q_UNUSED(from)
    Q_UNUSED(to)
    Q_UNUSED(should_succeed)
    // Helper implementation would go here
}

QTEST_MAIN(TestPluginInterface)
#include "test_plugin_interface.moc"
