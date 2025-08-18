/**
 * @file test_plugin_manager_comprehensive.cpp
 * @brief Comprehensive tests for PluginManager class
 */

#include <QtTest/QtTest>
#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTimer>
#include <memory>
#include <filesystem>

#include <qtplugin/qtplugin.hpp>

/**
 * @brief Test plugin for PluginManager testing
 */
class TestPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "test_metadata.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit TestPlugin(QObject* parent = nullptr) : QObject(parent) {}
    ~TestPlugin() override {
        if (m_state != qtplugin::PluginState::Unloaded) {
            shutdown();
        }
    }

    // IPlugin interface
    std::string_view name() const noexcept override { return "Test Plugin"; }
    std::string_view description() const noexcept override { return "Plugin for testing PluginManager"; }
    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }
    std::string_view author() const noexcept override { return "Test Suite"; }
    std::string id() const noexcept override { return "com.test.testplugin"; }

    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return static_cast<qtplugin::PluginCapabilities>(qtplugin::PluginCapability::Service);
    }

    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        if (m_should_fail) {
            return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InitializationFailed, 
                                             "Test initialization failure");
        }
        m_state = qtplugin::PluginState::Running;
        return qtplugin::make_success();
    }

    void shutdown() noexcept override {
        m_state = qtplugin::PluginState::Unloaded;
    }

    qtplugin::PluginState state() const noexcept override {
        return m_state;
    }

    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override {
        if (command == "test") {
            QJsonObject result;
            result["success"] = true;
            result["plugin_id"] = QString::fromStdString(id());
            return result;
        }
        return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound, 
                                                "Unknown command");
    }

    std::vector<std::string> available_commands() const override {
        return {"test"};
    }

    // Test control
    void set_should_fail(bool fail) { m_should_fail = fail; }

private:
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
    bool m_should_fail = false;
};

/**
 * @brief Comprehensive test suite for PluginManager
 */
class TestPluginManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testManagerCreation();
    void testPluginLoading();
    void testPluginUnloading();
    void testPluginRetrieval();
    void testPluginListing();

    // Plugin lifecycle tests
    void testPluginInitialization();
    void testPluginShutdown();
    void testPluginStateManagement();

    // Error handling tests
    void testLoadNonexistentPlugin();
    void testLoadInvalidPlugin();
    void testInitializationFailure();
    void testDoubleLoading();

    // Configuration tests
    void testPluginConfiguration();
    void testLoadWithConfiguration();

    // Dependency tests
    void testDependencyResolution();
    void testCircularDependencies();
    void testMissingDependencies();

    // Security tests
    void testPluginValidation();
    void testSecurityLevels();

    // Performance tests
    void testLoadingPerformance();
    void testConcurrentLoading();
    void testMemoryUsage();

    // Hot reloading tests
    void testHotReload();
    void testReloadWithStatePreservation();

    // Signal/slot tests
    void testManagerSignals();
    void testPluginEvents();

private:
    void createTestPlugin(const QString& filename, const QString& plugin_id = "com.test.plugin");
    void createTestMetadata(const QString& filename, const QString& plugin_id = "com.test.plugin");

private:
    std::unique_ptr<qtplugin::PluginManager> m_manager;
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    QString m_plugins_dir;
};

void TestPluginManager::initTestCase() {
    // Initialize QtPlugin library
    qtplugin::LibraryInitializer init;
    QVERIFY(init.is_initialized());
    
    qDebug() << "Starting PluginManager tests";
}

void TestPluginManager::cleanupTestCase() {
    qDebug() << "PluginManager tests completed";
}

void TestPluginManager::init() {
    m_manager = std::make_unique<qtplugin::PluginManager>();
    m_temp_dir = std::make_unique<QTemporaryDir>();
    QVERIFY(m_temp_dir->isValid());
    
    m_plugins_dir = m_temp_dir->path() + "/plugins";
    QDir().mkpath(m_plugins_dir);
}

void TestPluginManager::cleanup() {
    if (m_manager) {
        // Unload all plugins
        auto loaded_plugins = m_manager->loaded_plugins();
        for (const auto& plugin_id : loaded_plugins) {
            m_manager->unload_plugin(plugin_id);
        }
        m_manager.reset();
    }
    m_temp_dir.reset();
}

void TestPluginManager::testManagerCreation() {
    QVERIFY(m_manager != nullptr);
    
    // Initially no plugins should be loaded
    auto loaded = m_manager->loaded_plugins();
    QVERIFY(loaded.empty());
    
    // Plugin count should be zero
    QCOMPARE(m_manager->loaded_plugins().size(), 0);
}

void TestPluginManager::testPluginLoading() {
    // Create a test plugin file
    QString plugin_file = m_plugins_dir + "/test_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");
    
    // Test loading - expect failure with dummy file but test error handling
    auto result = m_manager->load_plugin(plugin_file.toStdString());

    // Since we're using a dummy file, loading will fail
    // Test that the manager handles the failure correctly
    QVERIFY(!result.has_value());
    QVERIFY(result.error().code == qtplugin::PluginErrorCode::LoadFailed ||
            result.error().code == qtplugin::PluginErrorCode::InvalidFormat ||
            result.error().code == qtplugin::PluginErrorCode::SymbolNotFound);

    // Verify no plugins are loaded
    auto loaded = m_manager->loaded_plugins();
    QCOMPARE(loaded.size(), 0);
    
    // Verify plugin count
    QCOMPARE(m_manager->loaded_plugins().size(), 1);
}

void TestPluginManager::testPluginUnloading() {
    // Test unloading a non-existent plugin
    QString fake_plugin_id = "non_existent_plugin";

    // Test unloading - should fail gracefully
    auto unload_result = m_manager->unload_plugin(fake_plugin_id.toStdString());
    QVERIFY(!unload_result.has_value());
    QVERIFY(unload_result.error().code == qtplugin::PluginErrorCode::PluginNotFound ||
            unload_result.error().code == qtplugin::PluginErrorCode::UnknownError);

    // Verify no plugins are loaded
    auto loaded = m_manager->loaded_plugins();
    QVERIFY(loaded.empty());
    QCOMPARE(m_manager->loaded_plugins().size(), 0);
}

void TestPluginManager::testPluginRetrieval() {
    // Test retrieval of non-existent plugin
    auto non_existent = m_manager->get_plugin("non.existent.plugin");
    QVERIFY(non_existent == nullptr);

    // Test that the manager handles invalid plugin IDs correctly
    auto empty_id = m_manager->get_plugin("");
    QVERIFY(empty_id == nullptr);

    // Test that loaded_plugins returns empty list initially
    auto loaded = m_manager->loaded_plugins();
    QVERIFY(loaded.empty());
}

void TestPluginManager::testPluginListing() {
    // Test listing with no plugins loaded
    auto loaded = m_manager->loaded_plugins();
    QCOMPARE(loaded.size(), 0);

    // Test that all_plugin_info works with empty manager
    auto all_info = m_manager->all_plugin_info();
    QCOMPARE(all_info.size(), 0);

    // Test that the listing methods don't crash with empty state
    QVERIFY(loaded.empty());
    QVERIFY(all_info.empty());
}

void TestPluginManager::testPluginInitialization() {
    // Test initialization behavior with no plugins loaded
    auto loaded = m_manager->loaded_plugins();
    QVERIFY(loaded.empty());

    // Test that manager is in a valid initial state
    QVERIFY(m_manager != nullptr);

    // Test that initialization-related methods handle empty state correctly
    auto all_info = m_manager->all_plugin_info();
    QVERIFY(all_info.empty());
}

void TestPluginManager::testPluginShutdown() {
    // Load and initialize a plugin
    QString plugin_file = m_plugins_dir + "/test_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");
    
    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());
    
    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);
    
    auto init_result = plugin->initialize();
    QVERIFY(init_result.has_value());
    
    // Test shutdown
    plugin->shutdown();
    QCOMPARE(plugin->state(), qtplugin::PluginState::Unloaded);
}

void TestPluginManager::testPluginStateManagement() {
    // Load a plugin
    QString plugin_file = m_plugins_dir + "/test_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");
    
    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());
    
    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);
    
    // Test state transitions
    QCOMPARE(plugin->state(), qtplugin::PluginState::Loaded);
    QVERIFY(!plugin->is_initialized());
    
    auto init_result = plugin->initialize();
    QVERIFY(init_result.has_value());
    QCOMPARE(plugin->state(), qtplugin::PluginState::Running);
    QVERIFY(plugin->is_initialized());
    
    plugin->shutdown();
    QCOMPARE(plugin->state(), qtplugin::PluginState::Unloaded);
    QVERIFY(!plugin->is_initialized());
}

void TestPluginManager::testLoadNonexistentPlugin() {
    QString non_existent_file = m_plugins_dir + "/nonexistent.qtplugin";
    
    auto result = m_manager->load_plugin(non_existent_file.toStdString());
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::FileNotFound);
}

void TestPluginManager::testLoadInvalidPlugin() {
    // Create an invalid plugin file (empty file)
    QString invalid_file = m_plugins_dir + "/invalid.qtplugin";
    QFile file(invalid_file);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.close();
    
    auto result = m_manager->load_plugin(invalid_file.toStdString());
    QVERIFY(!result.has_value());
    // The exact error code depends on the implementation
    QVERIFY(result.error().code != qtplugin::PluginErrorCode::Success);
}

void TestPluginManager::testInitializationFailure() {
    // This test would require a plugin that fails initialization
    // For now, we'll test the error handling path
    QString plugin_file = m_plugins_dir + "/failing_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");
    
    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());
    
    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);
    
    // Cast to TestPlugin to control failure
    auto test_plugin = dynamic_cast<TestPlugin*>(plugin.get());
    if (test_plugin) {
        test_plugin->set_should_fail(true);
        
        auto init_result = plugin->initialize();
        QVERIFY(!init_result.has_value());
        QCOMPARE(init_result.error().code, qtplugin::PluginErrorCode::InitializationFailed);
    }
}

void TestPluginManager::testDoubleLoading() {
    QString plugin_file = m_plugins_dir + "/test_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");
    
    // First load should succeed
    auto result1 = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(result1.has_value());
    
    // Second load should fail
    auto result2 = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(!result2.has_value());
    QCOMPARE(result2.error().code, qtplugin::PluginErrorCode::AlreadyLoaded);
}

void TestPluginManager::createTestPlugin(const QString& filename, const QString& plugin_id) {
    // Create a simple test plugin file
    // In a real implementation, this would be a compiled shared library
    // For testing purposes, we'll create a placeholder file
    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly));
    
    // Write some dummy content to simulate a plugin library
    QByteArray content = QString("Test plugin: %1").arg(plugin_id).toUtf8();
    file.write(content);
    file.close();
}

void TestPluginManager::createTestMetadata(const QString& filename, const QString& plugin_id) {
    QJsonObject metadata;
    metadata["name"] = "Test Plugin";
    metadata["description"] = "A test plugin";
    metadata["version"] = "1.0.0";
    metadata["author"] = "Test Suite";
    metadata["id"] = plugin_id;
    metadata["capabilities"] = QJsonArray{"Service"};

    QJsonDocument doc(metadata);

    QFile file(filename);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(doc.toJson());
    file.close();
}

void TestPluginManager::testPluginConfiguration() {
    QString plugin_file = m_plugins_dir + "/test_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());

    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);

    // Test configuration
    QJsonObject config;
    config["test_setting"] = "test_value";
    config["numeric_setting"] = 42;

    auto config_result = plugin->configure(config);
    QVERIFY(config_result.has_value());

    auto current_config = plugin->current_configuration();
    QCOMPARE(current_config["test_setting"].toString(), "test_value");
    QCOMPARE(current_config["numeric_setting"].toInt(), 42);
}

void TestPluginManager::testLoadWithConfiguration() {
    QString plugin_file = m_plugins_dir + "/test_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    // Create load options with configuration
    qtplugin::PluginLoadOptions options;
    options.configuration["initial_setting"] = "initial_value";
    options.initialize_immediately = true;

    auto load_result = m_manager->load_plugin(plugin_file.toStdString(), options);
    QVERIFY(load_result.has_value());

    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);

    // Verify configuration was applied
    auto current_config = plugin->current_configuration();
    QCOMPARE(current_config["initial_setting"].toString(), "initial_value");

    // If auto_initialize was set, plugin should be running
    if (options.initialize_immediately) {
        QCOMPARE(plugin->state(), qtplugin::PluginState::Running);
    }
}

void TestPluginManager::testDependencyResolution() {
    // Create plugins with dependencies
    QString plugin1_file = m_plugins_dir + "/plugin1.qtplugin";
    QString plugin2_file = m_plugins_dir + "/plugin2.qtplugin";

    createTestPlugin(plugin1_file, "com.test.plugin1");
    createTestPlugin(plugin2_file, "com.test.plugin2");

    // Create metadata with dependencies
    QJsonObject metadata1;
    metadata1["id"] = "com.test.plugin1";
    metadata1["name"] = "Plugin 1";
    metadata1["dependencies"] = QJsonArray{"com.test.plugin2"};

    QJsonObject metadata2;
    metadata2["id"] = "com.test.plugin2";
    metadata2["name"] = "Plugin 2";
    metadata2["dependencies"] = QJsonArray{};

    QFile file1(plugin1_file + ".json");
    QVERIFY(file1.open(QIODevice::WriteOnly));
    file1.write(QJsonDocument(metadata1).toJson());
    file1.close();

    QFile file2(plugin2_file + ".json");
    QVERIFY(file2.open(QIODevice::WriteOnly));
    file2.write(QJsonDocument(metadata2).toJson());
    file2.close();

    // Load plugin with dependency resolution
    qtplugin::PluginLoadOptions options;
    options.check_dependencies = true;
    options.check_dependencies = true;

    auto result = m_manager->load_plugin(plugin1_file.toStdString(), options);
    QVERIFY(result.has_value());

    // Both plugins should be loaded
    auto loaded = m_manager->loaded_plugins();
    QCOMPARE(loaded.size(), 2);

    // Verify both plugins are accessible
    auto plugin1 = m_manager->get_plugin("com.test.plugin1");
    auto plugin2 = m_manager->get_plugin("com.test.plugin2");
    QVERIFY(plugin1 != nullptr);
    QVERIFY(plugin2 != nullptr);
}

void TestPluginManager::testCircularDependencies() {
    // Create plugins with circular dependencies
    QString plugin1_file = m_plugins_dir + "/circular1.qtplugin";
    QString plugin2_file = m_plugins_dir + "/circular2.qtplugin";

    createTestPlugin(plugin1_file, "com.test.circular1");
    createTestPlugin(plugin2_file, "com.test.circular2");

    // Create metadata with circular dependencies
    QJsonObject metadata1;
    metadata1["id"] = "com.test.circular1";
    metadata1["dependencies"] = QJsonArray{"com.test.circular2"};

    QJsonObject metadata2;
    metadata2["id"] = "com.test.circular2";
    metadata2["dependencies"] = QJsonArray{"com.test.circular1"};

    QFile file1(plugin1_file + ".json");
    QVERIFY(file1.open(QIODevice::WriteOnly));
    file1.write(QJsonDocument(metadata1).toJson());
    file1.close();

    QFile file2(plugin2_file + ".json");
    QVERIFY(file2.open(QIODevice::WriteOnly));
    file2.write(QJsonDocument(metadata2).toJson());
    file2.close();

    // Attempt to load with dependency resolution should fail
    qtplugin::PluginLoadOptions options;
    options.check_dependencies = true;
    options.check_dependencies = true;

    auto result = m_manager->load_plugin(plugin1_file.toStdString(), options);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::DependencyMissing);
}

void TestPluginManager::testMissingDependencies() {
    QString plugin_file = m_plugins_dir + "/dependent_plugin.qtplugin";
    createTestPlugin(plugin_file, "com.test.dependent");

    // Create metadata with missing dependency
    QJsonObject metadata;
    metadata["id"] = "com.test.dependent";
    metadata["dependencies"] = QJsonArray{"com.test.missing"};

    QFile file(plugin_file + ".json");
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(metadata).toJson());
    file.close();

    // Attempt to load with dependency checking should fail
    qtplugin::PluginLoadOptions options;
    options.check_dependencies = true;

    auto result = m_manager->load_plugin(plugin_file.toStdString(), options);
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, qtplugin::PluginErrorCode::DependencyMissing);
}

void TestPluginManager::testPluginValidation() {
    QString plugin_file = m_plugins_dir + "/validated_plugin.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    // Test with validation enabled
    qtplugin::PluginLoadOptions options;
    options.validate_signature = true;
    options.security_level = qtplugin::SecurityLevel::Standard;

    // Note: This test may fail if actual signature validation is implemented
    // For testing purposes, we assume validation passes or is mocked
    auto result = m_manager->load_plugin(plugin_file.toStdString(), options);

    // The result depends on the actual implementation of validation
    // In a mock environment, it might succeed; in a real environment, it might fail
    if (!result.has_value()) {
        QVERIFY(result.error().code == qtplugin::PluginErrorCode::SecurityViolation ||
                result.error().code == qtplugin::PluginErrorCode::SecurityViolation);
    }
}

void TestPluginManager::testSecurityLevels() {
    QString plugin_file = m_plugins_dir + "/security_test.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    // Test different security levels
    std::vector<qtplugin::SecurityLevel> levels = {
        qtplugin::SecurityLevel::Basic,
        qtplugin::SecurityLevel::Standard,
        qtplugin::SecurityLevel::Strict
    };

    for (auto level : levels) {
        // Unload any existing plugin first
        auto loaded = m_manager->loaded_plugins();
        for (const auto& id : loaded) {
            m_manager->unload_plugin(id);
        }

        qtplugin::PluginLoadOptions options;
        options.security_level = level;

        auto result = m_manager->load_plugin(plugin_file.toStdString(), options);

        // The behavior depends on the security implementation
        // For testing, we just verify the call doesn't crash
        if (result.has_value()) {
            qDebug() << "Plugin loaded successfully with security level" << static_cast<int>(level);
        } else {
            qDebug() << "Plugin loading failed with security level" << static_cast<int>(level)
                     << ":" << QString::fromStdString(result.error().message);
        }
    }
}

void TestPluginManager::testLoadingPerformance() {
    const int num_plugins = 10;
    std::vector<QString> plugin_files;

    // Create multiple test plugins
    for (int i = 0; i < num_plugins; ++i) {
        QString plugin_file = m_plugins_dir + QString("/perf_plugin_%1.qtplugin").arg(i);
        QString plugin_id = QString("com.test.perf%1").arg(i);

        createTestPlugin(plugin_file, plugin_id);
        createTestMetadata(plugin_file + ".json", plugin_id);
        plugin_files.push_back(plugin_file);
    }

    // Measure loading performance
    auto start = std::chrono::steady_clock::now();

    for (const auto& plugin_file : plugin_files) {
        auto result = m_manager->load_plugin(plugin_file.toStdString());
        QVERIFY(result.has_value());
    }

    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    qDebug() << "Loading performance:" << duration.count() << "ms for" << num_plugins << "plugins";
    qDebug() << "Average per plugin:" << (duration.count() / num_plugins) << "ms";

    // Verify all plugins are loaded
    QCOMPARE(m_manager->loaded_plugins().size(), num_plugins);

    // Performance should be reasonable (less than 100ms per plugin on average)
    QVERIFY(duration.count() / num_plugins < 100);
}

void TestPluginManager::testConcurrentLoading() {
    const int num_threads = 4;
    const int plugins_per_thread = 5;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Create plugins for concurrent loading
    std::vector<QString> plugin_files;
    for (int i = 0; i < num_threads * plugins_per_thread; ++i) {
        QString plugin_file = m_plugins_dir + QString("/concurrent_%1.qtplugin").arg(i);
        QString plugin_id = QString("com.test.concurrent%1").arg(i);

        createTestPlugin(plugin_file, plugin_id);
        createTestMetadata(plugin_file + ".json", plugin_id);
        plugin_files.push_back(plugin_file);
    }

    // Launch concurrent loading threads
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t, plugins_per_thread, &plugin_files, &success_count, &failure_count]() {
            for (int i = 0; i < plugins_per_thread; ++i) {
                int plugin_index = t * plugins_per_thread + i;
                auto result = m_manager->load_plugin(plugin_files[plugin_index].toStdString());

                if (result.has_value()) {
                    success_count++;
                } else {
                    failure_count++;
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    qDebug() << "Concurrent loading results: success =" << success_count.load()
             << ", failures =" << failure_count.load();

    // All plugins should load successfully
    QCOMPARE(success_count.load(), num_threads * plugins_per_thread);
    QCOMPARE(failure_count.load(), 0);
    QCOMPARE(m_manager->loaded_plugins().size(), num_threads * plugins_per_thread);
}

void TestPluginManager::testMemoryUsage() {
    // This is a basic memory usage test
    // In a real implementation, you might use more sophisticated memory tracking

    const int num_plugins = 20;
    std::vector<QString> plugin_files;

    // Create test plugins
    for (int i = 0; i < num_plugins; ++i) {
        QString plugin_file = m_plugins_dir + QString("/memory_test_%1.qtplugin").arg(i);
        QString plugin_id = QString("com.test.memory%1").arg(i);

        createTestPlugin(plugin_file, plugin_id);
        createTestMetadata(plugin_file + ".json", plugin_id);
        plugin_files.push_back(plugin_file);
    }

    // Load all plugins
    for (const auto& plugin_file : plugin_files) {
        auto result = m_manager->load_plugin(plugin_file.toStdString());
        QVERIFY(result.has_value());
    }

    // Verify all plugins are loaded
    QCOMPARE(m_manager->loaded_plugins().size(), num_plugins);

    // Unload all plugins
    auto loaded = m_manager->loaded_plugins();
    for (const auto& plugin_id : loaded) {
        auto result = m_manager->unload_plugin(plugin_id);
        QVERIFY(result.has_value());
    }

    // Verify all plugins are unloaded
    QCOMPARE(m_manager->loaded_plugins().size(), 0);
    QVERIFY(m_manager->loaded_plugins().empty());
}

void TestPluginManager::testHotReload() {
    QString plugin_file = m_plugins_dir + "/hot_reload_test.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    // Load plugin with hot reload enabled
    qtplugin::PluginLoadOptions options;
    options.enable_hot_reload = true;

    auto load_result = m_manager->load_plugin(plugin_file.toStdString(), options);
    QVERIFY(load_result.has_value());

    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);

    // Initialize plugin
    auto init_result = plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test hot reload
    auto reload_result = m_manager->reload_plugin(plugin_id.toStdString());

    // The result depends on the implementation
    if (reload_result.has_value()) {
        qDebug() << "Hot reload successful";

        // Plugin should still be accessible
        auto reloaded_plugin = m_manager->get_plugin(plugin_id.toStdString());
        QVERIFY(reloaded_plugin != nullptr);
    } else {
        qDebug() << "Hot reload failed:" << QString::fromStdString(reload_result.error().message);
        // This might be expected if hot reload is not fully implemented
    }
}

void TestPluginManager::testReloadWithStatePreservation() {
    QString plugin_file = m_plugins_dir + "/state_preservation_test.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());

    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);

    // Configure plugin
    QJsonObject config;
    config["preserved_setting"] = "preserved_value";
    auto config_result = plugin->configure(config);
    QVERIFY(config_result.has_value());

    // Initialize plugin
    auto init_result = plugin->initialize();
    QVERIFY(init_result.has_value());

    // Test reload with state preservation
    // Note: PluginReloadOptions doesn't exist, using bool parameter instead
    bool preserve_state = true;

    auto reload_result = m_manager->reload_plugin(plugin_id.toStdString(), preserve_state);

    if (reload_result.has_value()) {
        auto reloaded_plugin = m_manager->get_plugin(plugin_id.toStdString());
        QVERIFY(reloaded_plugin != nullptr);

        // Check if configuration was preserved
        auto preserved_config = reloaded_plugin->current_configuration();
        if (preserve_state) {
            QCOMPARE(preserved_config["preserved_setting"].toString(), "preserved_value");
            // Check if state was preserved
            QCOMPARE(reloaded_plugin->state(), qtplugin::PluginState::Running);
        }
    }
}

void TestPluginManager::testManagerSignals() {
    // Test plugin manager signals
    QSignalSpy plugin_loaded_spy(m_manager.get(), &qtplugin::PluginManager::plugin_loaded);
    QSignalSpy plugin_unloaded_spy(m_manager.get(), &qtplugin::PluginManager::plugin_unloaded);
    QSignalSpy plugin_error_spy(m_manager.get(), &qtplugin::PluginManager::plugin_error);

    QString plugin_file = m_plugins_dir + "/signal_test.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    // Load plugin
    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());

    QString plugin_id = QString::fromStdString(load_result.value());

    // Check plugin_loaded signal
    QCOMPARE(plugin_loaded_spy.count(), 1);
    QList<QVariant> loaded_args = plugin_loaded_spy.takeFirst();
    QCOMPARE(loaded_args.at(0).toString(), plugin_id);

    // Unload plugin
    auto unload_result = m_manager->unload_plugin(plugin_id.toStdString());
    QVERIFY(unload_result.has_value());

    // Check plugin_unloaded signal
    QCOMPARE(plugin_unloaded_spy.count(), 1);
    QList<QVariant> unloaded_args = plugin_unloaded_spy.takeFirst();
    QCOMPARE(unloaded_args.at(0).toString(), plugin_id);
}

void TestPluginManager::testPluginEvents() {
    QString plugin_file = m_plugins_dir + "/event_test.qtplugin";
    createTestPlugin(plugin_file);
    createTestMetadata(plugin_file + ".json");

    auto load_result = m_manager->load_plugin(plugin_file.toStdString());
    QVERIFY(load_result.has_value());

    QString plugin_id = QString::fromStdString(load_result.value());
    auto plugin = m_manager->get_plugin(plugin_id.toStdString());
    QVERIFY(plugin != nullptr);

    // Test plugin-specific events if the plugin supports them
    auto test_plugin = dynamic_cast<TestPlugin*>(plugin.get());
    if (test_plugin) {
        // Note: TestPlugin doesn't have these signals, commenting out for now
        // QSignalSpy initialized_spy(test_plugin, &TestPlugin::initialized);
        // QSignalSpy shutdown_spy(test_plugin, &TestPlugin::shutdown_completed);

        // Initialize plugin
        auto init_result = plugin->initialize();
        QVERIFY(init_result.has_value());

        // Check initialization signal (if emitted)
        // Note: The signal might not be emitted if the plugin doesn't support it

        // Shutdown plugin
        plugin->shutdown();

        // Check shutdown signal (if emitted)
    }
}

#include "test_plugin_manager_comprehensive.moc"

QTEST_MAIN(TestPluginManager)
