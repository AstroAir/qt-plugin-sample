# Component Usage Guide - QtPlugin v3.0.0

## Overview

QtPlugin v3.0.0 introduces a revolutionary modular component architecture that allows developers to use individual components directly for advanced scenarios. This guide covers how to leverage the component system for maximum flexibility and performance.

## Component Access Levels

### Level 1: Standard Manager API (Recommended)

Most developers should use the high-level manager APIs:

```cpp
#include <qtplugin/qtplugin.hpp>

// Standard usage - recommended for most applications
qtplugin::PluginManager manager;
auto result = manager.load_plugin("my_plugin.so");
```

### Level 2: Component Access (Advanced)

For advanced scenarios, access components directly:

```cpp
#include <qtplugin/qtplugin.hpp>      // Core functionality
#include <qtplugin/components.hpp>    // Component access

// Advanced usage - direct component access
auto registry = std::make_unique<qtplugin::PluginRegistry>();
auto validator = std::make_unique<qtplugin::SecurityValidator>();
```

### Level 3: Custom Components (Expert)

Create custom component implementations:

```cpp
class CustomPluginRegistry : public qtplugin::IPluginRegistry {
    // Custom implementation
};
```

## Component Categories and Usage

### Core Components

#### PluginRegistry - Plugin Storage and Lookup

**Use Cases**:
- Custom plugin discovery logic
- Advanced plugin filtering
- Performance-critical plugin lookup

**Example**:
```cpp
#include <qtplugin/components.hpp>

auto registry = std::make_unique<qtplugin::PluginRegistry>();

// Register a plugin
qtplugin::PluginInfo plugin_info;
plugin_info.id = "com.example.plugin";
plugin_info.file_path = "/path/to/plugin.so";
plugin_info.state = qtplugin::PluginState::Unloaded;

auto result = registry->register_plugin(plugin_info);
if (result) {
    qDebug() << "Plugin registered successfully";
}

// Advanced search
auto search_criteria = qtplugin::PluginSearchCriteria{};
search_criteria.capabilities = {qtplugin::PluginCapability::Service};
search_criteria.min_version = qtplugin::Version{1, 0, 0};

auto matching_plugins = registry->search_plugins(search_criteria);
```

#### PluginDependencyResolver - Dependency Management

**Use Cases**:
- Complex dependency scenarios
- Custom load ordering
- Dependency conflict resolution

**Example**:
```cpp
auto resolver = std::make_unique<qtplugin::PluginDependencyResolver>();

// Add plugins with dependencies
qtplugin::PluginInfo plugin_a;
plugin_a.id = "plugin.a";
plugin_a.metadata.dependencies = {};

qtplugin::PluginInfo plugin_b;
plugin_b.id = "plugin.b";
plugin_b.metadata.dependencies = {"plugin.a"};

resolver->add_plugin(plugin_a);
resolver->add_plugin(plugin_b);

// Resolve load order
auto load_order = resolver->resolve_load_order();
if (load_order) {
    for (const auto& plugin_id : *load_order) {
        qDebug() << "Load order:" << plugin_id;
    }
}
```

### Security Components

#### SecurityValidator - File and Metadata Validation

**Use Cases**:
- Custom validation rules
- Performance-critical validation
- Integration with external security systems

**Example**:
```cpp
auto validator = std::make_unique<qtplugin::SecurityValidator>();

// Configure validation rules
qtplugin::ValidationRules rules;
rules.require_metadata = true;
rules.check_file_integrity = true;
rules.validate_api_version = true;
validator->set_validation_rules(rules);

// Validate a plugin file
auto validation_result = validator->validate_file_integrity("/path/to/plugin.so");
if (validation_result.is_valid) {
    qDebug() << "Plugin validation passed";
} else {
    for (const auto& error : validation_result.errors) {
        qWarning() << "Validation error:" << error;
    }
}
```

#### PermissionManager - Access Control

**Use Cases**:
- Fine-grained permission control
- Custom permission policies
- Runtime permission management

**Example**:
```cpp
auto permission_manager = std::make_unique<qtplugin::PermissionManager>();

// Define custom permissions
qtplugin::Permission file_access_permission;
file_access_permission.name = "file.read";
file_access_permission.description = "Read file system access";
file_access_permission.level = qtplugin::PermissionLevel::Medium;

permission_manager->register_permission(file_access_permission);

// Grant permission to a plugin
auto grant_result = permission_manager->grant_permission(
    "com.example.plugin", 
    "file.read"
);

// Check permission at runtime
bool has_permission = permission_manager->check_permission(
    "com.example.plugin", 
    "file.read"
);
```

### Configuration Components

#### ConfigurationStorage - File I/O Operations

**Use Cases**:
- Custom configuration formats
- Performance-critical configuration access
- Configuration encryption/decryption

**Example**:
```cpp
auto storage = std::make_unique<qtplugin::ConfigurationStorage>();

// Configure storage options
qtplugin::StorageOptions options;
options.format = qtplugin::ConfigurationFormat::JSON;
options.enable_backup = true;
options.atomic_writes = true;
storage->set_storage_options(options);

// Load configuration
auto config_result = storage->load_configuration("/path/to/config.json");
if (config_result) {
    auto config = config_result.value();
    // Process configuration
}

// Save configuration with atomic operation
QJsonObject new_config;
new_config["setting"] = "value";
auto save_result = storage->save_configuration("/path/to/config.json", new_config);
```

#### ConfigurationWatcher - File Monitoring

**Use Cases**:
- Real-time configuration updates
- Custom change detection logic
- Performance-optimized monitoring

**Example**:
```cpp
auto watcher = std::make_unique<qtplugin::ConfigurationWatcher>();

// Configure monitoring
qtplugin::WatcherConfig config;
config.poll_interval = std::chrono::milliseconds(500);
config.enable_debouncing = true;
config.debounce_delay = std::chrono::milliseconds(100);
watcher->set_watcher_config(config);

// Connect to change signals
QObject::connect(watcher.get(), &qtplugin::ConfigurationWatcher::fileChanged,
                [](const std::filesystem::path& path) {
                    qDebug() << "Configuration changed:" << path;
                });

// Start monitoring
auto watch_result = watcher->watch_file("/path/to/config.json");
```

### Resource Components

#### ResourcePool - Resource Management

**Use Cases**:
- Custom resource types
- Performance-critical resource allocation
- Resource lifecycle optimization

**Example**:
```cpp
// Create a typed resource pool
auto memory_pool = std::make_unique<qtplugin::ResourcePool<std::vector<char>>>(
    "memory_buffers", 
    qtplugin::ResourceType::Memory
);

// Configure pool settings
qtplugin::ResourceQuota quota;
quota.max_instances = 100;
quota.max_memory_bytes = 10 * 1024 * 1024; // 10MB
quota.max_lifetime = std::chrono::minutes(30);
memory_pool->set_quota(quota);

// Set resource factory
memory_pool->set_factory([]() {
    return std::make_unique<std::vector<char>>(4096); // 4KB buffers
});

// Acquire and use resource
auto resource_result = memory_pool->acquire_resource("my_plugin");
if (resource_result) {
    auto [handle, buffer] = std::move(resource_result.value());
    
    // Use the buffer
    buffer->resize(1024);
    std::fill(buffer->begin(), buffer->end(), 'A');
    
    // Release back to pool
    memory_pool->release_resource(handle, std::move(buffer));
}
```

#### ResourceMonitor - Usage Monitoring

**Use Cases**:
- Performance monitoring
- Resource leak detection
- Custom alerting systems

**Example**:
```cpp
auto monitor = std::make_unique<qtplugin::ResourceMonitor>();

// Configure monitoring
qtplugin::MonitoringConfig config;
config.monitoring_interval = std::chrono::seconds(1);
config.enable_leak_detection = true;
config.leak_detection_threshold = std::chrono::minutes(5);
monitor->set_monitoring_config(config);

// Add custom alerts
qtplugin::ResourceAlert memory_alert;
memory_alert.name = "high_memory_usage";
memory_alert.resource_type = qtplugin::ResourceType::Memory;
memory_alert.condition = "memory_usage > 80%";
memory_alert.callback = [](const qtplugin::AlertContext& context) {
    qWarning() << "High memory usage detected:" << context.current_value;
};

monitor->add_alert(memory_alert);

// Start monitoring
monitor->start_monitoring();
```

## Component Composition Patterns

### Pattern 1: Manager Delegation

Create custom managers that delegate to components:

```cpp
class CustomPluginManager {
private:
    std::unique_ptr<qtplugin::IPluginRegistry> m_registry;
    std::unique_ptr<qtplugin::ISecurityValidator> m_validator;
    
public:
    CustomPluginManager() 
        : m_registry(std::make_unique<qtplugin::PluginRegistry>())
        , m_validator(std::make_unique<qtplugin::SecurityValidator>()) {
    }
    
    auto load_plugin_with_custom_logic(const std::string& path) 
        -> qtplugin::expected<std::string, qtplugin::PluginError> {
        
        // Custom pre-validation
        if (!custom_pre_check(path)) {
            return qtplugin::unexpected(qtplugin::PluginError{
                qtplugin::PluginErrorCode::ValidationFailed,
                "Custom pre-check failed"
            });
        }
        
        // Delegate to components
        auto validation = m_validator->validate_file_integrity(path);
        if (!validation.is_valid) {
            return qtplugin::unexpected(qtplugin::PluginError{
                qtplugin::PluginErrorCode::SecurityViolation,
                "Security validation failed"
            });
        }
        
        // Continue with plugin loading...
        return "plugin_id";
    }
};
```

### Pattern 2: Component Pipeline

Chain components for complex operations:

```cpp
class ConfigurationPipeline {
private:
    std::unique_ptr<qtplugin::IConfigurationStorage> m_storage;
    std::unique_ptr<qtplugin::IConfigurationValidator> m_validator;
    std::unique_ptr<qtplugin::IConfigurationMerger> m_merger;
    
public:
    auto process_configuration(const std::filesystem::path& path) 
        -> qtplugin::expected<QJsonObject, qtplugin::PluginError> {
        
        // Pipeline: Load -> Validate -> Merge
        auto loaded = m_storage->load_configuration(path);
        if (!loaded) return qtplugin::unexpected(loaded.error());
        
        auto validated = m_validator->validate_configuration(loaded.value());
        if (!validated) return qtplugin::unexpected(validated.error());
        
        auto merged = m_merger->merge_with_defaults(validated.value());
        return merged;
    }
};
```

## Best Practices

### 1. Choose the Right Abstraction Level

- **Use managers** for standard plugin operations
- **Use components** for specialized requirements
- **Create custom components** only when necessary

### 2. Component Lifecycle Management

```cpp
class ComponentManager {
private:
    std::vector<std::unique_ptr<qtplugin::IComponent>> m_components;
    
public:
    template<typename T, typename... Args>
    T* add_component(Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        m_components.push_back(std::move(component));
        return ptr;
    }
    
    void shutdown_all() {
        // Shutdown in reverse order
        for (auto it = m_components.rbegin(); it != m_components.rend(); ++it) {
            (*it)->shutdown();
        }
        m_components.clear();
    }
};
```

### 3. Error Handling

Always handle component errors appropriately:

```cpp
auto result = component->perform_operation();
if (!result) {
    const auto& error = result.error();
    qCritical() << "Component operation failed:"
                << "Code:" << static_cast<int>(error.code)
                << "Message:" << error.message
                << "Details:" << error.details;
    
    // Handle error appropriately
    return handle_component_error(error);
}
```

### 4. Performance Considerations

- **Reuse components** when possible
- **Configure components** for your specific use case
- **Monitor component performance** in production

## Migration from Manager-Only Usage

### Before (v2.x)
```cpp
qtplugin::PluginManager manager;
// Limited to manager capabilities
```

### After (v3.0.0)
```cpp
// Option 1: Continue using managers (no changes needed)
qtplugin::PluginManager manager;

// Option 2: Use components for advanced scenarios
#include <qtplugin/components.hpp>
auto registry = std::make_unique<qtplugin::PluginRegistry>();
auto validator = std::make_unique<qtplugin::SecurityValidator>();
```

The component architecture provides a smooth migration path while enabling powerful new capabilities for advanced users.
