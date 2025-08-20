# API Reference

Complete API documentation for QtPlugin v3.0.0 - a modern, pure C++ plugin system for Qt applications.

## Library Information

- **Version**: 3.0.0
- **API Version**: 3.0
- **Minimum Qt Version**: 6.2.0
- **C++ Standard**: C++20
- **License**: MIT

## API Organization

The QtPlugin API is organized into logical modules:

### Core Module (`QtPlugin::Core`)

The core module provides essential plugin system functionality with a modular component architecture:

- **Plugin Interfaces** - Base and specialized plugin interfaces
- **Plugin Manager** - Central plugin orchestration and lifecycle management
- **Plugin Loader** - Low-level plugin loading and unloading
- **Plugin Registry** - Plugin discovery and metadata management (component)
- **Plugin Dependency Resolver** - Dependency resolution and load ordering (component)

#### Core Components (v3.0.0)

- **PluginRegistry** - Centralized plugin storage and lookup management
- **PluginDependencyResolver** - Dependency graph management and resolution

### Monitoring Module (`QtPlugin::Monitoring`)

The monitoring module provides plugin monitoring and hot reload capabilities:

- **Plugin Hot Reload Manager** - Dynamic plugin reloading (component)
- **Plugin Metrics Collector** - Performance metrics and monitoring (component)

#### Monitoring Components (v3.0.0)

- **PluginHotReloadManager** - File watching and hot reload functionality
- **PluginMetricsCollector** - Metrics collection and performance monitoring

### Security Module (`QtPlugin::Security`)

The security module handles plugin validation and trust management with modular components:

- **Security Manager** - Central security policy enforcement and orchestration
- **Security Validator** - Core file and metadata validation (component)
- **Signature Verifier** - Digital signature verification (component)
- **Permission Manager** - Plugin permission and access control (component)
- **Security Policy Engine** - Policy evaluation and enforcement (component)

#### Security Components (v3.0.0)

- **SecurityValidator** - File integrity and metadata validation
- **SignatureVerifier** - Digital signature verification and trust chains
- **PermissionManager** - Access control and permission management
- **SecurityPolicyEngine** - Security policy evaluation and enforcement

### Communication Module (`QtPlugin::Communication`)

The communication module enables inter-plugin messaging:

- **Message Bus** - Central message routing and delivery
- **Message Types** - Type-safe message definitions
- **Event System** - Plugin event broadcasting and handling
- **RPC System** - Remote procedure call support

### Configuration Module (`QtPlugin::Configuration`)

The configuration module provides comprehensive configuration management with modular components:

- **Configuration Manager** - Central configuration orchestration and management
- **Configuration Storage** - File I/O and persistence operations (component)
- **Configuration Validator** - Schema validation and type checking (component)
- **Configuration Merger** - Configuration merging and inheritance (component)
- **Configuration Watcher** - File monitoring and change detection (component)

#### Configuration Components (v3.0.0)

- **ConfigurationStorage** - Configuration file I/O and persistence
- **ConfigurationValidator** - Schema validation and type checking
- **ConfigurationMerger** - Configuration merging and inheritance
- **ConfigurationWatcher** - File monitoring and change detection

### Resource Module (`QtPlugin::Resource`)

The resource module handles plugin resource management with modular components:

- **Resource Manager** - Central resource orchestration and management
- **Resource Pool** - Resource pooling and lifecycle management (component)
- **Resource Allocator** - Allocation strategies and policies (component)
- **Resource Monitor** - Usage monitoring and alerting (component)

#### Resource Components (v3.0.0)

- **ResourcePool** - Template-based resource pooling and lifecycle management
- **ResourceAllocator** - Multi-strategy allocation with quota enforcement
- **ResourceMonitor** - Real-time monitoring, alerting, and leak detection

### Utilities Module (`QtPlugin::Utils`)

The utilities module provides helper classes and functions:

- **Error Handling** - Custom `expected<T,E>` implementation
- **Version Management** - Version comparison and validation
- **Concepts** - C++20 concepts for type safety
- **Helper Functions** - Common utility functions

## Quick Reference

### Essential Classes

| Class | Purpose | Header |
|-------|---------|--------|
| `PluginManager` | Central plugin management | `qtplugin/core/plugin_manager.hpp` |
| `IPlugin` | Base plugin interface | `qtplugin/core/plugin_interface.hpp` |
| `LibraryInitializer` | RAII library initialization | `qtplugin/qtplugin.hpp` |
| `PluginLoader` | Low-level plugin loading | `qtplugin/core/plugin_loader.hpp` |
| `SecurityManager` | Plugin security validation | `qtplugin/security/security_manager.hpp` |

### Component Classes (v3.0.0)

| Component | Purpose | Header |
|-----------|---------|--------|
| `PluginRegistry` | Plugin storage and lookup | `qtplugin/core/plugin_registry.hpp` |
| `PluginDependencyResolver` | Dependency resolution | `qtplugin/core/plugin_dependency_resolver.hpp` |
| `PluginHotReloadManager` | Hot reload functionality | `qtplugin/monitoring/plugin_hot_reload_manager.hpp` |
| `PluginMetricsCollector` | Metrics collection | `qtplugin/monitoring/plugin_metrics_collector.hpp` |
| `SecurityValidator` | File and metadata validation | `qtplugin/security/components/security_validator.hpp` |
| `SignatureVerifier` | Digital signature verification | `qtplugin/security/components/signature_verifier.hpp` |
| `PermissionManager` | Access control | `qtplugin/security/components/permission_manager.hpp` |
| `SecurityPolicyEngine` | Policy evaluation | `qtplugin/security/components/security_policy_engine.hpp` |
| `ConfigurationStorage` | Configuration I/O | `qtplugin/managers/components/configuration_storage.hpp` |
| `ConfigurationValidator` | Schema validation | `qtplugin/managers/components/configuration_validator.hpp` |
| `ConfigurationMerger` | Configuration merging | `qtplugin/managers/components/configuration_merger.hpp` |
| `ConfigurationWatcher` | File monitoring | `qtplugin/managers/components/configuration_watcher.hpp` |
| `ResourcePool` | Resource pooling | `qtplugin/managers/components/resource_pool.hpp` |
| `ResourceAllocator` | Resource allocation | `qtplugin/managers/components/resource_allocator.hpp` |
| `ResourceMonitor` | Resource monitoring | `qtplugin/managers/components/resource_monitor.hpp` |

### Key Enumerations

| Enumeration | Purpose | Values |
|-------------|---------|--------|
| `PluginState` | Plugin lifecycle state | `Unloaded`, `Loading`, `Loaded`, `Running`, `Stopping`, `Failed` |
| `PluginCapability` | Plugin capabilities | `UI`, `Service`, `Network`, `DataProcessing`, `Scripting`, etc. |
| `SecurityLevel` | Security trust levels | `Trusted`, `Verified`, `Sandboxed`, `Untrusted` |
| `PluginErrorCode` | Error classification | `LoadFailed`, `InitializationFailed`, `SecurityViolation`, etc. |

### Common Types

```cpp
// Error handling
template<typename T, typename E>
using expected = /* custom implementation */;

using PluginError = /* error type with code and message */;

// Plugin identification
using PluginId = std::string;
using PluginPath = std::filesystem::path;

// Plugin collections
using PluginList = std::vector<std::shared_ptr<IPlugin>>;
using PluginMap = std::unordered_map<PluginId, std::shared_ptr<IPlugin>>;
```

## Usage Patterns

### Basic Plugin Loading

```cpp
#include <qtplugin/qtplugin.hpp>

// Initialize library
qtplugin::LibraryInitializer init;

// Create manager
qtplugin::PluginManager manager;

// Load plugin
auto result = manager.load_plugin("./plugins/example.qtplugin");
if (result) {
    auto plugin = manager.get_plugin(result.value());
    plugin->initialize();
}
```

### Plugin Discovery

```cpp
// Add search paths
manager.add_search_path("./plugins");
manager.add_search_path("/usr/local/lib/plugins");

// Discover plugins
auto discovered = manager.discover_plugins();
for (const auto& path : discovered) {
    auto result = manager.load_plugin(path);
    // Handle result...
}
```

### Capability-Based Plugin Access

```cpp
// Get all UI plugins
auto ui_plugins = manager.get_plugins_by_capability(
    qtplugin::PluginCapability::UI
);

for (auto plugin : ui_plugins) {
    auto ui_plugin = std::dynamic_pointer_cast<qtplugin::IUIPlugin>(plugin);
    if (ui_plugin) {
        auto widget = ui_plugin->create_widget();
        // Use widget...
    }
}
```

### Error Handling

```cpp
auto result = manager.load_plugin(plugin_path);
if (!result) {
    const auto& error = result.error();
    qWarning() << "Plugin load failed:"
               << "Code:" << static_cast<int>(error.code)
               << "Message:" << error.message.c_str();
    
    // Handle specific error types
    switch (error.code) {
        case qtplugin::PluginErrorCode::FileNotFound:
            // Handle missing file
            break;
        case qtplugin::PluginErrorCode::SecurityViolation:
            // Handle security issue
            break;
        default:
            // Handle other errors
            break;
    }
}
```

## API Sections

### 🔌 [Core Interfaces](interfaces/index.md)

Base plugin interface and specialized plugin types:

- `IPlugin` - Base interface all plugins must implement
- `IServicePlugin` - Background service plugins
- `IUIPlugin` - User interface plugins
- `INetworkPlugin` - Network operation plugins
- `IDataProviderPlugin` - Data processing plugins
- `IScriptingPlugin` - Scripting engine plugins

### ⚙️ [Core Classes](core/index.md)

Essential system classes for plugin management:

- `PluginManager` - Central plugin orchestration
- `PluginLoader` - Low-level plugin loading
- `PluginRegistry` - Plugin discovery and metadata
- `LibraryInitializer` - RAII library setup

### 🛡️ [Security API](security/index.md)

Security and validation components:

- `SecurityManager` - Security policy enforcement
- `PluginValidator` - Plugin validation and verification
- `TrustManager` - Trust level management
- `SandboxController` - Plugin execution sandboxing

### 🔧 [Utilities](utilities/index.md)

Helper classes and utility functions:

- `expected<T,E>` - Error handling type
- `Version` - Version management
- `PluginConcepts` - C++20 concepts
- `PluginHelpers` - Utility functions

### 📝 [Examples](examples.md)

Practical code examples and usage patterns:

- Basic plugin loading and usage
- Advanced plugin management
- Security configuration
- Performance optimization
- Error handling patterns

## Compatibility

### Qt Versions

| Qt Version | Support Status | Notes |
|------------|----------------|-------|
| Qt 6.2.x | ✅ Fully supported | Minimum required version |
| Qt 6.3.x | ✅ Fully supported | Recommended |
| Qt 6.4.x | ✅ Fully supported | Recommended |
| Qt 6.5.x | ✅ Fully supported | Latest tested |
| Qt 6.6.x | ✅ Fully supported | Latest tested |

### Compilers

| Compiler | Version | Support Status |
|----------|---------|----------------|
| GCC | 10.0+ | ✅ Fully supported |
| Clang | 12.0+ | ✅ Fully supported |
| MSVC | 2019 16.11+ | ✅ Fully supported |

### Platforms

| Platform | Architecture | Support Status |
|----------|--------------|----------------|
| Windows | x64, x86 | ✅ Fully supported |
| macOS | x64, ARM64 | ✅ Fully supported |
| Linux | x64, ARM64 | ✅ Fully supported |

## Migration Guide

### From Version 2.x to 3.x

Key changes in version 3.0:

1. **C++20 Requirement**: Updated to require C++20 compiler
2. **Error Handling**: Replaced `PluginResult<T>` with `expected<T, PluginError>`
3. **Interface Changes**: Plugin methods now use `std::string_view`
4. **Configuration**: Updated to use `QJsonObject` for configuration

See the [Migration Guide](../reference/migration-guide.md) for detailed upgrade instructions.

## Getting Help

- **[User Guide](../user-guide/index.md)** - Integration and usage guide
- **[Developer Guide](../developer-guide/index.md)** - Plugin development
- **[Examples](../examples/index.md)** - Working code examples
- **[FAQ](../reference/faq.md)** - Frequently asked questions
- **[GitHub Issues](https://github.com/example/qt-plugin-sample/issues)** - Bug reports and feature requests

---

**QtPlugin v3.0.0** - Modern C++ plugin system for Qt applications.
