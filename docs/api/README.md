# QtPlugin API Reference

This comprehensive API reference covers all public interfaces, classes, and functions in the QtPlugin library.

## Table of Contents

### Core Interfaces

- [IPlugin](interfaces/iplugin.md) - Base plugin interface
- [IServicePlugin](interfaces/iserviceplugin.md) - Service plugin interface
- [IUIPlugin](interfaces/iuiplugin.md) - UI plugin interface
- [INetworkPlugin](interfaces/inetworkplugin.md) - Network plugin interface
- [IDataProviderPlugin](interfaces/idataproviderplugin.md) - Data provider interface
- [IScriptingPlugin](interfaces/iscriptingplugin.md) - Scripting plugin interface

### Core Classes

- [PluginManager](classes/pluginmanager.md) - Plugin management
- [PluginLoader](classes/pluginloader.md) - Plugin loading
- [PluginRegistry](classes/pluginregistry.md) - Plugin registry
- [LibraryInitializer](classes/libraryinitializer.md) - Library initialization

### Utility Classes

- [Version](classes/version.md) - Version handling
- [PluginError](classes/pluginerror.md) - Error handling
- [Expected](classes/expected.md) - Result type
- [PluginMetadata](classes/pluginmetadata.md) - Plugin metadata

### Communication

- [MessageBus](classes/messagebus.md) - Inter-plugin communication
- [MessageTypes](classes/messagetypes.md) - Message type definitions

### Security

- [SecurityManager](classes/securitymanager.md) - Plugin security
- [PluginValidator](classes/pluginvalidator.md) - Plugin validation

### Configuration

- [ConfigurationManager](classes/configurationmanager.md) - Configuration management
- [PluginConfiguration](classes/pluginconfiguration.md) - Plugin configuration

## Quick Reference

### Essential Types

```cpp
namespace qtplugin {
    // Core interfaces
    class IPlugin;
    class IServicePlugin;
    class IUIPlugin;
    
    // Management classes
    class PluginManager;
    class PluginLoader;
    class LibraryInitializer;
    
    // Utility types
    class Version;
    class PluginError;
    template<typename T, typename E> class expected;
    
    // Enumerations
    enum class PluginState;
    enum class PluginCapability;
    enum class ServiceState;
    enum class PluginErrorCode;
}
```

### Core Enumerations

#### PluginState

```cpp
enum class PluginState : int {
    Unloaded = 0,      // Plugin not loaded
    Loaded = 1,        // Plugin loaded but not initialized
    Initializing = 2,  // Plugin is being initialized
    Running = 3,       // Plugin is active and running
    Paused = 4,        // Plugin is temporarily paused
    Error = 5,         // Plugin encountered an error
    ShuttingDown = 6   // Plugin is being shut down
};
```

#### PluginCapability

```cpp
enum class PluginCapability : uint32_t {
    None = 0x0000,
    UI = 0x0001,              // Provides user interface
    Service = 0x0002,         // Background service
    Network = 0x0004,         // Network operations
    DataProcessing = 0x0008,  // Data processing
    Scripting = 0x0010,       // Script execution
    FileSystem = 0x0020,      // File operations
    Database = 0x0040,        // Database access
    AsyncInit = 0x0080,       // Asynchronous initialization
    HotReload = 0x0100,       // Hot reloading support
    Configuration = 0x0200,   // Configuration management
    Logging = 0x0400,         // Logging capabilities
    Security = 0x0800,        // Security features
    Threading = 0x1000,       // Multi-threading
    Monitoring = 0x2000       // Performance monitoring
};
```

#### ServiceState

```cpp
enum class ServiceState : int {
    Stopped = 0,       // Service is stopped
    Starting = 1,      // Service is starting
    Running = 2,       // Service is running
    Pausing = 3,       // Service is pausing
    Paused = 4,        // Service is paused
    Resuming = 5,      // Service is resuming
    Stopping = 6,      // Service is stopping
    Error = 7          // Service encountered an error
};
```

### Basic Usage Patterns

#### Plugin Loading

```cpp
#include <qtplugin/qtplugin.hpp>

// Initialize library
qtplugin::LibraryInitializer init;
if (!init.is_initialized()) {
    // Handle error
    return -1;
}

// Create manager
qtplugin::PluginManager manager;

// Load plugin
auto result = manager.load_plugin("./plugins/example.qtplugin");
if (!result) {
    // Handle error: result.error().message
    return -1;
}

// Get plugin
auto plugin = manager.get_plugin(result.value());
if (plugin) {
    // Use plugin
    auto init_result = plugin->initialize();
    // ...
}
```

#### Plugin Creation

```cpp
#include <qtplugin/qtplugin.hpp>
#include <QObject>

class MyPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(MyPlugin, "com.example.MyPlugin/1.0", "metadata.json")

public:
    // Implement required methods
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override;
    
    std::vector<std::string> available_commands() const override;
};
```

#### Error Handling

```cpp
// Using expected<T, E> for error handling
auto result = plugin->initialize();
if (!result) {
    // Handle error
    qtplugin::PluginError error = result.error();
    std::cout << "Error: " << error.message << std::endl;
    std::cout << "Code: " << static_cast<int>(error.code) << std::endl;
    return;
}

// Success case
std::cout << "Plugin initialized successfully" << std::endl;
```

#### Configuration Management

```cpp
// Get default configuration
auto default_config = plugin->default_configuration();
if (default_config) {
    QJsonObject config = default_config.value();
    // Modify configuration
    config["setting"] = "new_value";
    
    // Apply configuration
    auto config_result = plugin->configure(config);
    if (!config_result) {
        // Handle configuration error
    }
}
```

#### Service Plugin Usage

```cpp
// Cast to service plugin interface
auto service = qobject_cast<qtplugin::IServicePlugin*>(plugin.get());
if (service) {
    // Start service
    auto start_result = service->start_service();
    if (start_result) {
        std::cout << "Service started" << std::endl;
    }
    
    // Check service state
    if (service->is_service_running()) {
        // Service is running
    }
    
    // Stop service
    auto stop_result = service->stop_service();
}
```

## Version Information

- **Library Version**: 3.0.0
- **API Version**: 3.0
- **Minimum Qt Version**: 6.2.0
- **C++ Standard**: C++20

## Compatibility

### Qt Versions

- Qt 6.2.x: ✅ Fully supported
- Qt 6.3.x: ✅ Fully supported  
- Qt 6.4.x: ✅ Fully supported
- Qt 6.5.x: ✅ Fully supported
- Qt 6.6.x: ✅ Fully supported

### Compilers

- **GCC**: 10.0+ ✅
- **Clang**: 12.0+ ✅
- **MSVC**: 2019 16.11+ ✅

### Platforms

- **Windows**: 10+ ✅
- **macOS**: 10.15+ ✅
- **Linux**: Ubuntu 20.04+, CentOS 8+ ✅

## Migration Guide

### From Version 2.x to 3.x

- Updated to C++20 requirements
- New `expected<T, E>` error handling
- Enhanced plugin capabilities system
- Improved security validation

### Breaking Changes

- `PluginResult<T>` replaced with `expected<T, PluginError>`
- Plugin interface methods now use `std::string_view`
- Configuration methods now use `QJsonObject`

## See Also

- [User Guide](../user-guide/README.md) - Getting started
- [Developer Guide](../developer-guide/README.md) - Plugin development
- [Examples](../examples/README.md) - Code examples
- [Tutorials](../tutorials/README.md) - Step-by-step guides
