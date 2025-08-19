# User Guide

This comprehensive user guide covers everything you need to know to use QtPlugin in your applications.

## Overview

QtPlugin is a modern, pure C++ plugin system designed for Qt applications that provides:

- **🔌 Dynamic Plugin Loading** - Load and unload plugins at runtime
- **🔄 Lifecycle Management** - Complete plugin lifecycle control
- **🔗 Dependency Resolution** - Automatic dependency management
- **⚙️ Configuration Management** - Flexible plugin configuration
- **🔒 Security Validation** - Plugin security and sandboxing
- **📊 Performance Monitoring** - Real-time performance tracking
- **🔥 Hot Reloading** - Dynamic plugin reloading
- **🧵 Thread Safety** - Safe concurrent operations

## Table of Contents

<div class="grid cards" markdown>

- :material-rocket-launch:{ .lg .middle } **Getting Started**

    ---

    Quick integration guide and basic usage patterns

    [:octicons-arrow-right-24: Integration Guide](integration.md)

- :material-cog:{ .lg .middle } **Configuration**

    ---

    Plugin configuration, settings, and management

    [:octicons-arrow-right-24: Configuration Guide](configuration.md)

- :material-view-dashboard:{ .lg .middle } **Plugin Management**

    ---

    Loading, unloading, and managing plugin lifecycles

    [:octicons-arrow-right-24: Plugin Management](plugin-management.md)

- :material-shield-check:{ .lg .middle } **Security**

    ---

    Plugin security, validation, and trust management

    [:octicons-arrow-right-24: Security Guide](security.md)

- :material-speedometer:{ .lg .middle } **Performance**

    ---

    Performance optimization and monitoring

    [:octicons-arrow-right-24: Performance Guide](performance.md)

- :material-help-circle:{ .lg .middle } **Troubleshooting**

    ---

    Common issues, solutions, and debugging tips

    [:octicons-arrow-right-24: Troubleshooting](troubleshooting.md)

</div>

## Key Concepts

### Plugin Interface

All plugins implement the `IPlugin` interface, which provides:

- Metadata (name, version, author, etc.)
- Lifecycle methods (initialize, shutdown)
- Command execution
- Configuration management
- Error handling

### Plugin Manager

The `PluginManager` class is the central component that:

- Loads and unloads plugins
- Manages plugin lifecycle
- Handles dependencies
- Provides security validation
- Monitors performance

### Plugin States

Plugins can be in one of several states:

- `Unloaded`: Plugin not loaded
- `Loaded`: Plugin loaded but not initialized
- `Initializing`: Plugin is being initialized
- `Running`: Plugin is active and running
- `Paused`: Plugin is temporarily paused
- `Error`: Plugin encountered an error
- `Shutting Down`: Plugin is being shut down

### Plugin Capabilities

Plugins declare their capabilities using flags:

- `UI`: Provides user interface components
- `Service`: Background service functionality
- `Network`: Network-related operations
- `DataProcessing`: Data processing capabilities
- `Scripting`: Script execution support
- `FileSystem`: File system operations
- `Database`: Database connectivity
- `Configuration`: Configuration management
- `Security`: Security features
- `Threading`: Multi-threading support
- `Monitoring`: Performance monitoring

## Basic Usage Pattern

```cpp
#include <qtplugin/qtplugin.hpp>

// 1. Initialize the library
qtplugin::LibraryInitializer init;
if (!init.is_initialized()) {
    // Handle initialization error
    return -1;
}

// 2. Create plugin manager
qtplugin::PluginManager manager;

// 3. Load a plugin
auto result = manager.load_plugin("./plugins/example.qtplugin");
if (!result) {
    // Handle load error
    return -1;
}

// 4. Get and use the plugin
auto plugin = manager.get_plugin(result.value());
if (plugin) {
    // Initialize the plugin
    auto init_result = plugin->initialize();
    if (init_result) {
        // Plugin is ready to use
        auto cmd_result = plugin->execute_command("status");
        // Handle command result
    }
}
```

## Next Steps

- Follow the [Quick Start Guide](quick-start.md) for a hands-on introduction
- Read the [Integration Guide](integration.md) to integrate QtPlugin into your application
- Explore the [Configuration Guide](configuration.md) for advanced configuration options
- Check out the [Examples](../examples/README.md) for practical code samples

## Support

If you encounter issues or have questions:

1. Check the [Troubleshooting Guide](troubleshooting.md)
2. Review the [FAQ](faq.md)
3. Search existing GitHub issues
4. Create a new issue with detailed information

## Related Documentation

- [Developer Guide](../developer-guide/README.md) - For plugin developers
- [API Reference](../api/README.md) - Complete API documentation
- [Architecture Overview](../architecture/README.md) - System design and architecture
