# Troubleshooting Guide

Common issues, solutions, and debugging techniques for QtPlugin applications and plugins.

## Quick Diagnosis

### Plugin Loading Issues

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| "Plugin file not found" | Incorrect path or missing file | Verify file exists and path is correct |
| "Invalid plugin format" | Wrong file type or corrupted | Check file extension and integrity |
| "Symbol not found" | Missing dependencies | Install required libraries |
| "Initialization failed" | Plugin-specific error | Check plugin logs and configuration |
| "Security violation" | Unsigned or untrusted plugin | Sign plugin or adjust security settings |

### Runtime Issues

| Symptom | Likely Cause | Solution |
|---------|--------------|----------|
| Application crashes on plugin load | ABI mismatch or memory corruption | Rebuild with same compiler/Qt version |
| Plugin commands fail | Invalid parameters or state | Validate input and check plugin state |
| Memory leaks | Improper resource cleanup | Review plugin shutdown procedures |
| Performance degradation | Inefficient plugin operations | Profile and optimize plugin code |

## Common Issues and Solutions

### 1. Plugin Loading Failures

#### Issue: "Failed to load plugin: file not found"

**Symptoms:**

```
Error: Failed to load plugin: file not found
Plugin path: ./plugins/myplugin.qtplugin
```

**Causes:**

- Plugin file doesn't exist at specified path
- Incorrect file permissions
- Path separator issues on different platforms

**Solutions:**

```cpp
// Verify plugin file exists before loading
std::filesystem::path plugin_path = "./plugins/myplugin.qtplugin";

if (!std::filesystem::exists(plugin_path)) {
    qWarning() << "Plugin file does not exist:" << plugin_path.c_str();
    
    // List available plugins
    for (const auto& entry : std::filesystem::directory_iterator("./plugins")) {
        if (entry.path().extension() == ".qtplugin") {
            qDebug() << "Available plugin:" << entry.path().filename().c_str();
        }
    }
    return;
}

// Check file permissions
auto perms = std::filesystem::status(plugin_path).permissions();
if ((perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
    qWarning() << "Plugin file is not readable:" << plugin_path.c_str();
    return;
}
```

#### Issue: "Invalid plugin format"

**Symptoms:**

```
Error: Invalid plugin format
Plugin appears to be corrupted or not a valid QtPlugin
```

**Causes:**

- File is not a valid dynamic library
- Plugin was built with incompatible compiler
- File corruption during transfer

**Solutions:**

```bash
# Check if file is a valid dynamic library
file myplugin.qtplugin
# Should show: ELF 64-bit LSB shared object (Linux)
# or: Mach-O 64-bit dynamically linked shared library (macOS)
# or: PE32+ executable (DLL) (Windows)

# Check plugin dependencies
ldd myplugin.qtplugin  # Linux
otool -L myplugin.qtplugin  # macOS
dumpbin /dependents myplugin.qtplugin  # Windows

# Verify plugin exports required symbols
nm -D myplugin.qtplugin | grep qt_plugin  # Linux/macOS
dumpbin /exports myplugin.qtplugin  # Windows
```

### 2. Initialization Problems

#### Issue: Plugin initialization fails

**Symptoms:**

```
Plugin loaded successfully but initialization failed
Error: Configuration validation failed
```

**Debugging Steps:**

```cpp
// Enable detailed logging
qtplugin::PluginManager manager;
manager.set_log_level(qtplugin::LogLevel::Debug);

// Load plugin with detailed error reporting
auto result = manager.load_plugin(plugin_path);
if (!result) {
    const auto& error = result.error();
    qDebug() << "Load failed:";
    qDebug() << "  Code:" << static_cast<int>(error.code);
    qDebug() << "  Message:" << error.message.c_str();
    qDebug() << "  Details:" << error.details.c_str();
}

// Test plugin initialization separately
auto plugin = manager.get_plugin(plugin_id);
if (plugin) {
    qDebug() << "Plugin metadata:";
    qDebug() << "  Name:" << plugin->name().data();
    qDebug() << "  Version:" << plugin->version().to_string().c_str();
    qDebug() << "  State:" << static_cast<int>(plugin->state());
    
    // Try initialization with error details
    auto init_result = plugin->initialize();
    if (!init_result) {
        qDebug() << "Initialization failed:" << init_result.error().message.c_str();
        
        // Check plugin's last error
        auto last_error = plugin->last_error();
        if (!last_error.empty()) {
            qDebug() << "Plugin error:" << last_error.c_str();
        }
    }
}
```

### 3. Dependency Issues

#### Issue: Missing dependencies

**Symptoms:**

```
Error: Required dependency not found: com.example.base-plugin
Plugin cannot be loaded due to unresolved dependencies
```

**Solutions:**

```cpp
// Check plugin dependencies before loading
class DependencyChecker {
public:
    static std::vector<std::string> check_missing_dependencies(
        const std::string& plugin_id,
        qtplugin::PluginManager& manager) {
        
        std::vector<std::string> missing;
        
        auto plugin = manager.get_plugin(plugin_id);
        if (!plugin) return missing;
        
        auto dependencies = plugin->dependencies();
        for (const auto& dep : dependencies) {
            if (!manager.is_plugin_loaded(dep)) {
                missing.push_back(dep);
            }
        }
        
        return missing;
    }
    
    static bool resolve_dependencies(const std::string& plugin_id,
                                   qtplugin::PluginManager& manager) {
        auto missing = check_missing_dependencies(plugin_id, manager);
        
        for (const auto& dep : missing) {
            qDebug() << "Attempting to load dependency:" << dep.c_str();
            
            // Try to find and load dependency
            auto dep_path = find_plugin_file(dep);
            if (dep_path.empty()) {
                qWarning() << "Dependency not found:" << dep.c_str();
                return false;
            }
            
            auto result = manager.load_plugin(dep_path);
            if (!result) {
                qWarning() << "Failed to load dependency:" << dep.c_str()
                          << "Error:" << result.error().message.c_str();
                return false;
            }
        }
        
        return true;
    }
};
```

### 4. Configuration Problems

#### Issue: Invalid configuration

**Symptoms:**

```
Configuration validation failed
Invalid value for parameter 'max_connections': -1
```

**Solutions:**

```cpp
// Validate configuration before applying
bool validate_plugin_config(const QJsonObject& config) {
    // Check required fields
    const QStringList required_fields = {"name", "version", "enabled"};
    for (const auto& field : required_fields) {
        if (!config.contains(field)) {
            qWarning() << "Missing required field:" << field;
            return false;
        }
    }
    
    // Validate field types and values
    if (config.contains("max_connections")) {
        int max_conn = config["max_connections"].toInt(-1);
        if (max_conn < 0 || max_conn > 10000) {
            qWarning() << "Invalid max_connections value:" << max_conn;
            return false;
        }
    }
    
    if (config.contains("timeout")) {
        int timeout = config["timeout"].toInt(-1);
        if (timeout < 100 || timeout > 300000) {  // 100ms to 5 minutes
            qWarning() << "Invalid timeout value:" << timeout;
            return false;
        }
    }
    
    return true;
}

// Provide default configuration
QJsonObject get_default_config() {
    QJsonObject config;
    config["name"] = "Default Plugin";
    config["version"] = "1.0.0";
    config["enabled"] = true;
    config["max_connections"] = 100;
    config["timeout"] = 30000;  // 30 seconds
    config["log_level"] = "info";
    return config;
}
```

### 5. Performance Issues

#### Issue: Slow plugin loading

**Symptoms:**

- Application freezes during plugin loading
- Long startup times
- High CPU usage during initialization

**Debugging:**

```cpp
// Profile plugin loading time
class PluginProfiler {
public:
    static void profile_plugin_loading(qtplugin::PluginManager& manager,
                                     const std::string& plugin_path) {
        QElapsedTimer timer;
        
        qDebug() << "Starting plugin load profile for:" << plugin_path.c_str();
        
        timer.start();
        auto load_result = manager.load_plugin(plugin_path);
        auto load_time = timer.elapsed();
        
        if (!load_result) {
            qWarning() << "Plugin load failed after" << load_time << "ms";
            return;
        }
        
        qDebug() << "Plugin loaded in" << load_time << "ms";
        
        auto plugin = manager.get_plugin(load_result.value());
        if (plugin) {
            timer.restart();
            auto init_result = plugin->initialize();
            auto init_time = timer.elapsed();
            
            if (init_result) {
                qDebug() << "Plugin initialized in" << init_time << "ms";
                qDebug() << "Total time:" << (load_time + init_time) << "ms";
            } else {
                qWarning() << "Plugin initialization failed after" << init_time << "ms";
            }
        }
    }
};

// Use asynchronous loading for better performance
void load_plugins_async(qtplugin::PluginManager& manager,
                       const std::vector<std::string>& plugin_paths) {
    std::vector<std::future<void>> futures;
    
    for (const auto& path : plugin_paths) {
        auto future = std::async(std::launch::async, [&manager, path]() {
            auto result = manager.load_plugin(path);
            if (result) {
                qDebug() << "Loaded plugin:" << result.value().c_str();
            } else {
                qWarning() << "Failed to load:" << path.c_str();
            }
        });
        futures.push_back(std::move(future));
    }
    
    // Wait for all plugins to load
    for (auto& future : futures) {
        future.wait();
    }
}
```

## Debugging Techniques

### 1. Enable Debug Logging

```cpp
// Enable comprehensive logging
qtplugin::LibraryInitializer init;
init.set_log_level(qtplugin::LogLevel::Debug);
init.enable_file_logging("qtplugin_debug.log");

// Log plugin manager operations
qtplugin::PluginManager manager;
manager.set_debug_mode(true);
manager.enable_operation_logging(true);
```

### 2. Use Debugger Effectively

```cpp
// Add debugging breakpoints in plugin code
qtplugin::expected<void, qtplugin::PluginError> MyPlugin::initialize() {
    qDebug() << "MyPlugin::initialize() called";
    
    // Set breakpoint here for debugging
    if (m_debug_mode) {
        qDebug() << "Debug mode enabled, pausing for inspection";
    }
    
    try {
        // Plugin initialization code
        setup_resources();
        validate_configuration();
        initialize_components();
        
        qDebug() << "MyPlugin initialization completed successfully";
        return qtplugin::make_success();
        
    } catch (const std::exception& e) {
        qCritical() << "MyPlugin initialization failed:" << e.what();
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed,
            e.what()
        );
    }
}
```

### 3. Memory Debugging

```bash
# Use Valgrind for memory leak detection (Linux)
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
         --track-origins=yes ./my_qtplugin_app

# Use AddressSanitizer (GCC/Clang)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address -g" ..
make
./my_qtplugin_app

# Use Application Verifier (Windows)
appverif -enable Heaps Handles Locks -for my_qtplugin_app.exe
```

### 4. Plugin State Monitoring

```cpp
// Monitor plugin states
class PluginStateMonitor : public QObject {
    Q_OBJECT

public:
    void monitor_plugin(std::shared_ptr<qtplugin::IPlugin> plugin) {
        m_plugins[plugin->id()] = plugin;
        
        // Start monitoring timer
        if (!m_timer.isActive()) {
            m_timer.start(1000);  // Check every second
        }
    }

private slots:
    void check_plugin_states() {
        for (auto& [id, plugin] : m_plugins) {
            auto current_state = plugin->state();
            auto previous_state = m_previous_states[id];
            
            if (current_state != previous_state) {
                qDebug() << "Plugin" << id.c_str() << "state changed:"
                        << static_cast<int>(previous_state) << "->"
                        << static_cast<int>(current_state);
                
                if (current_state == qtplugin::PluginState::Failed) {
                    qWarning() << "Plugin" << id.c_str() << "entered failed state";
                    qWarning() << "Last error:" << plugin->last_error().c_str();
                }
            }
            
            m_previous_states[id] = current_state;
        }
    }

private:
    std::unordered_map<std::string, std::shared_ptr<qtplugin::IPlugin>> m_plugins;
    std::unordered_map<std::string, qtplugin::PluginState> m_previous_states;
    QTimer m_timer;
};
```

## Platform-Specific Issues

### Windows

**Issue: DLL loading fails**

```cpp
// Check Windows-specific DLL issues
#ifdef _WIN32
void diagnose_dll_loading(const std::string& dll_path) {
    HMODULE handle = LoadLibraryA(dll_path.c_str());
    if (!handle) {
        DWORD error = GetLastError();
        qWarning() << "LoadLibrary failed with error:" << error;
        
        // Common Windows DLL errors
        switch (error) {
            case ERROR_MOD_NOT_FOUND:
                qWarning() << "DLL or one of its dependencies not found";
                break;
            case ERROR_BAD_EXE_FORMAT:
                qWarning() << "Invalid executable format (32/64-bit mismatch?)";
                break;
            case ERROR_ACCESS_DENIED:
                qWarning() << "Access denied - check file permissions";
                break;
        }
    } else {
        FreeLibrary(handle);
        qDebug() << "DLL can be loaded successfully";
    }
}
#endif
```

### macOS

**Issue: Code signing problems**

```bash
# Check code signing status
codesign -dv --verbose=4 myplugin.dylib

# Re-sign if necessary
codesign --force --sign "Developer ID Application: Your Name" myplugin.dylib

# Check for quarantine attribute
xattr -l myplugin.dylib
# Remove quarantine if present
xattr -d com.apple.quarantine myplugin.dylib
```

### Linux

**Issue: Missing shared libraries**

```bash
# Check library dependencies
ldd myplugin.so

# Find missing libraries
ldconfig -p | grep libname

# Set library path
export LD_LIBRARY_PATH=/path/to/libs:$LD_LIBRARY_PATH
```

## Getting Help

### 1. Collect Debug Information

Before seeking help, collect:

- QtPlugin version
- Qt version
- Compiler and version
- Operating system
- Complete error messages
- Minimal reproduction case

### 2. Community Resources

- **GitHub Issues**: Report bugs and request features
- **Documentation**: Check latest documentation
- **Examples**: Review working examples
- **Stack Overflow**: Search for similar issues

### 3. Professional Support

For commercial applications:

- Priority support channels
- Custom consulting services
- Training and workshops
- Enterprise licensing options

---

**Next**: [FAQ](faq.md) for frequently asked questions.
