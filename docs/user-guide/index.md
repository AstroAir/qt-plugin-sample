# User Guide

Welcome to the QtPlugin User Guide! This comprehensive guide covers everything you need to know to integrate and use QtPlugin in your applications.

## What is QtPlugin?

QtPlugin is a modern, production-ready C++ plugin system built specifically for Qt applications. It provides a robust foundation for creating modular, extensible applications with dynamic plugin loading capabilities.

### Key Benefits

- **🚀 Easy Integration** - Simple API that integrates seamlessly with existing Qt applications
- **🔒 Secure by Design** - Built-in security validation and sandboxing capabilities
- **⚡ High Performance** - Optimized for production workloads with minimal overhead
- **🔧 Developer Friendly** - Clear APIs, comprehensive documentation, and extensive examples
- **🏗️ Extensible Architecture** - Modular design supports easy customization and extension
- **📱 Cross-Platform** - Works consistently on Windows, macOS, and Linux

## Getting Started

### Prerequisites

Before using QtPlugin, ensure you have:

- **Qt6** (6.2 or higher) with Core module
- **CMake** 3.21 or higher
- **C++20 compatible compiler**
- Basic understanding of Qt and C++

### Quick Integration

The fastest way to get started is with CMake FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    QtPlugin
    GIT_REPOSITORY https://github.com/example/qt-plugin-sample.git
    GIT_TAG        v3.0.0
    SOURCE_SUBDIR  lib
)
FetchContent_MakeAvailable(QtPlugin)

target_link_libraries(your_app PRIVATE QtPlugin::Core)
```

### Basic Usage

```cpp
#include <qtplugin/qtplugin.hpp>

int main() {
    // Initialize QtPlugin
    qtplugin::LibraryInitializer init;
    
    // Create plugin manager
    qtplugin::PluginManager manager;
    
    // Load a plugin
    auto result = manager.load_plugin("./plugins/example.qtplugin");
    if (result) {
        auto plugin = manager.get_plugin(result.value());
        plugin->initialize();
        // Use the plugin...
    }
    
    return 0;
}
```

## Core Concepts

### Plugin Manager

The **Plugin Manager** is the central component that orchestrates all plugin operations:

- **Plugin Discovery** - Automatically finds plugins in specified directories
- **Lifecycle Management** - Handles loading, initialization, and cleanup
- **Dependency Resolution** - Manages plugin dependencies automatically
- **State Monitoring** - Tracks plugin health and performance
- **Security Enforcement** - Applies security policies and validation

### Plugin Interfaces

QtPlugin provides several specialized interfaces for different plugin types:

| Interface | Purpose | Use Cases |
|-----------|---------|-----------|
| `IPlugin` | Base interface | All plugins must implement this |
| `IServicePlugin` | Background services | System monitoring, data processing |
| `IUIPlugin` | User interface | Custom widgets, dialogs, toolbars |
| `INetworkPlugin` | Network operations | Protocol handlers, API clients |
| `IDataProviderPlugin` | Data processing | File formats, database connectors |

### Plugin Capabilities

Plugins declare their capabilities using capability flags:

```cpp
enum class PluginCapability : uint32_t {
    None = 0x0000,
    UI = 0x0001,              // Provides user interface
    Service = 0x0002,         // Background service
    Network = 0x0004,         // Network operations
    DataProcessing = 0x0008,  // Data processing
    Scripting = 0x0010,       // Script execution
    // ... more capabilities
};
```

### Configuration Management

QtPlugin supports flexible JSON-based configuration:

```json
{
    "plugins": {
        "search_paths": ["./plugins", "/usr/local/lib/plugins"],
        "security": {
            "level": "verified",
            "require_signature": true
        },
        "performance": {
            "enable_monitoring": true,
            "max_load_time": 5000
        }
    }
}
```

## User Guide Sections

### 📖 Essential Guides

1. **[Integration Guide](integration.md)** - Integrate QtPlugin into your application
2. **[Configuration Guide](configuration.md)** - Configure plugins and the plugin system
3. **[Plugin Management](plugin-management.md)** - Manage plugin lifecycles and dependencies

### 🔒 Security and Performance

4. **[Security Guide](security.md)** - Secure your plugin system
5. **[Performance Guide](performance.md)** - Optimize plugin performance

### 🛠️ Maintenance and Support

6. **[Troubleshooting](troubleshooting.md)** - Solve common issues
7. **[Migration Guide](../reference/migration-guide.md)** - Upgrade from previous versions
8. **[FAQ](../reference/faq.md)** - Frequently asked questions

## Common Use Cases

### Application Extensions

Create extensible applications where users can add functionality:

```cpp
// Load UI plugins for custom tools
auto ui_plugins = manager.get_plugins_by_capability(PluginCapability::UI);
for (auto plugin : ui_plugins) {
    auto ui_plugin = std::dynamic_pointer_cast<IUIPlugin>(plugin);
    if (ui_plugin) {
        auto widget = ui_plugin->create_widget();
        // Add widget to your application UI
    }
}
```

### Service Architecture

Build service-oriented applications with plugin-based services:

```cpp
// Load and start service plugins
auto service_plugins = manager.get_plugins_by_capability(PluginCapability::Service);
for (auto plugin : service_plugins) {
    auto service = std::dynamic_pointer_cast<IServicePlugin>(plugin);
    if (service) {
        service->initialize();
        service->start_service();
    }
}
```

### Data Processing Pipelines

Create flexible data processing systems:

```cpp
// Load data processing plugins
auto data_plugins = manager.get_plugins_by_capability(PluginCapability::DataProcessing);
for (auto plugin : data_plugins) {
    auto processor = std::dynamic_pointer_cast<IDataProviderPlugin>(plugin);
    if (processor && processor->supports_format("json")) {
        auto data = processor->read_data("input.json");
        // Process data...
    }
}
```

## Best Practices

### 1. Error Handling

Always check return values and handle errors gracefully:

```cpp
auto result = manager.load_plugin(plugin_path);
if (!result) {
    qWarning() << "Failed to load plugin:" << result.error().message.c_str();
    // Handle error appropriately
    return;
}
```

### 2. Resource Management

Use RAII and smart pointers for automatic cleanup:

```cpp
{
    qtplugin::LibraryInitializer init; // Automatic cleanup
    qtplugin::PluginManager manager;
    
    // Plugins automatically cleaned up when manager is destroyed
} // Automatic library cleanup here
```

### 3. Security First

Always validate plugins and use appropriate security levels:

```cpp
qtplugin::PluginLoadOptions options;
options.security_level = qtplugin::SecurityLevel::Verified;
options.validate_signature = true;

auto result = manager.load_plugin(plugin_path, options);
```

### 4. Performance Monitoring

Monitor plugin performance in production:

```cpp
manager.set_performance_monitoring(true);

// Check plugin performance
auto metrics = manager.get_plugin_metrics(plugin_id);
if (metrics.load_time > std::chrono::seconds(5)) {
    qWarning() << "Plugin" << plugin_id.c_str() << "loaded slowly";
}
```

## Getting Help

If you need assistance:

1. **Check the [Troubleshooting Guide](troubleshooting.md)** for common issues
2. **Review the [FAQ](../reference/faq.md)** for frequently asked questions
3. **Explore [Examples](../examples/index.md)** for working code samples
4. **Consult the [API Reference](../api/index.md)** for detailed documentation
5. **Visit the [GitHub Repository](https://github.com/example/qt-plugin-sample)** for community support

## What's Next?

Ready to dive deeper? Here are your next steps:

- **New to QtPlugin?** Start with the [Integration Guide](integration.md)
- **Need to configure plugins?** Check the [Configuration Guide](configuration.md)
- **Want to create plugins?** Visit the [Developer Guide](../developer-guide/index.md)
- **Looking for examples?** Explore the [Examples Section](../examples/index.md)

---

**QtPlugin v3.0.0** - Making Qt applications more modular and extensible.
