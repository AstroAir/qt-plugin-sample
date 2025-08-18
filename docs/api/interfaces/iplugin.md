# IPlugin Interface

The `IPlugin` interface is the core interface that all plugins must implement. It provides essential functionality for plugin lifecycle management, metadata access, configuration, and command execution.

## Header

```cpp
#include <qtplugin/core/plugin_interface.hpp>
```

## Declaration

```cpp
namespace qtplugin {
    class IPlugin {
    public:
        virtual ~IPlugin() = default;
        
        // === Metadata ===
        virtual std::string_view name() const noexcept = 0;
        virtual std::string_view description() const noexcept = 0;
        virtual Version version() const noexcept = 0;
        virtual std::string_view author() const noexcept = 0;
        virtual std::string id() const noexcept = 0;
        
        // Optional metadata
        virtual std::string_view license() const noexcept { return ""; }
        virtual std::string_view homepage() const noexcept { return ""; }
        virtual std::string_view category() const noexcept { return "General"; }
        virtual PluginMetadata metadata() const;
        
        // === Lifecycle Management ===
        virtual expected<void, PluginError> initialize() = 0;
        virtual void shutdown() noexcept = 0;
        virtual PluginState state() const noexcept = 0;
        virtual bool is_initialized() const noexcept;
        
        // === Capabilities ===
        virtual PluginCapabilities capabilities() const noexcept = 0;
        virtual std::vector<std::string> dependencies() const { return {}; }
        virtual std::vector<std::string> optional_dependencies() const { return {}; }
        
        // === Configuration ===
        virtual std::optional<QJsonObject> default_configuration() const { return std::nullopt; }
        virtual expected<void, PluginError> configure(const QJsonObject& config);
        virtual QJsonObject current_configuration() const { return QJsonObject{}; }
        virtual bool validate_configuration(const QJsonObject& config) const { return true; }
        
        // === Commands ===
        virtual expected<QJsonObject, PluginError>
        execute_command(std::string_view command, const QJsonObject& params = {}) = 0;
        virtual std::vector<std::string> available_commands() const = 0;
        
        // === Error Handling ===
        virtual std::string last_error() const { return ""; }
        virtual std::vector<std::string> error_log() const { return {}; }
        virtual void clear_errors() {}
    };
}
```

## Required Methods

### Metadata Methods

#### `name()`

```cpp
virtual std::string_view name() const noexcept = 0;
```

Returns the human-readable name of the plugin.

**Returns**: Plugin name as a string view
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
std::string_view name() const noexcept override {
    return "My Example Plugin";
}
```

#### `description()`

```cpp
virtual std::string_view description() const noexcept = 0;
```

Returns a brief description of the plugin's functionality.

**Returns**: Plugin description as a string view
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
std::string_view description() const noexcept override {
    return "A plugin that demonstrates QtPlugin features";
}
```

#### `version()`

```cpp
virtual Version version() const noexcept = 0;
```

Returns the plugin version.

**Returns**: Plugin version object
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
Version version() const noexcept override {
    return {1, 2, 3}; // Version 1.2.3
}
```

#### `author()`

```cpp
virtual std::string_view author() const noexcept = 0;
```

Returns the plugin author information.

**Returns**: Author name/information as a string view
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
std::string_view author() const noexcept override {
    return "John Doe <john@example.com>";
}
```

#### `id()`

```cpp
virtual std::string id() const noexcept = 0;
```

Returns a unique identifier for the plugin.

**Returns**: Unique plugin identifier
**Thread Safety**: Must be thread-safe
**Note**: Should follow reverse domain notation (e.g., "com.example.myplugin")
**Example**:

```cpp
std::string id() const noexcept override {
    return "com.example.myplugin";
}
```

### Lifecycle Methods

#### `initialize()`

```cpp
virtual expected<void, PluginError> initialize() = 0;
```

Initializes the plugin. Called after the plugin is loaded.

**Returns**: Success or error information
**Thread Safety**: Called from main thread, implementation should be thread-safe
**Example**:

```cpp
expected<void, PluginError> initialize() override {
    try {
        // Initialize plugin resources
        m_state = PluginState::Running;
        return make_success();
    } catch (const std::exception& e) {
        m_state = PluginState::Error;
        return make_error<void>(PluginErrorCode::InitializationFailed, e.what());
    }
}
```

#### `shutdown()`

```cpp
virtual void shutdown() noexcept = 0;
```

Shuts down the plugin and cleans up resources.

**Thread Safety**: Called from main thread, must not throw exceptions
**Note**: Must clean up all resources and not throw exceptions
**Example**:

```cpp
void shutdown() noexcept override {
    try {
        // Clean up resources
        m_timer.reset();
        m_state = PluginState::Unloaded;
    } catch (...) {
        // Log error but don't throw
    }
}
```

#### `state()`

```cpp
virtual PluginState state() const noexcept = 0;
```

Returns the current plugin state.

**Returns**: Current plugin state
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
PluginState state() const noexcept override {
    return m_state.load();
}
```

### Capability Methods

#### `capabilities()`

```cpp
virtual PluginCapabilities capabilities() const noexcept = 0;
```

Returns the plugin's capabilities as a bitwise combination of flags.

**Returns**: Plugin capabilities flags
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
PluginCapabilities capabilities() const noexcept override {
    return PluginCapability::Service | PluginCapability::Configuration;
}
```

### Command Methods

#### `execute_command()`

```cpp
virtual expected<QJsonObject, PluginError>
execute_command(std::string_view command, const QJsonObject& params = {}) = 0;
```

Executes a plugin command with optional parameters.

**Parameters**:

- `command`: Command name to execute
- `params`: Optional command parameters as JSON object

**Returns**: Command result as JSON object or error
**Thread Safety**: Implementation should be thread-safe
**Example**:

```cpp
expected<QJsonObject, PluginError>
execute_command(std::string_view command, const QJsonObject& params) override {
    if (command == "status") {
        QJsonObject result;
        result["state"] = static_cast<int>(state());
        result["uptime"] = get_uptime();
        return result;
    }
    
    return make_error<QJsonObject>(
        PluginErrorCode::CommandNotFound,
        "Unknown command: " + std::string(command)
    );
}
```

#### `available_commands()`

```cpp
virtual std::vector<std::string> available_commands() const = 0;
```

Returns a list of commands supported by the plugin.

**Returns**: Vector of command names
**Thread Safety**: Must be thread-safe
**Example**:

```cpp
std::vector<std::string> available_commands() const override {
    return {"status", "configure", "reset"};
}
```

## Optional Methods

### Configuration Methods

#### `default_configuration()`

```cpp
virtual std::optional<QJsonObject> default_configuration() const;
```

Returns the default configuration for the plugin.

**Returns**: Default configuration as JSON object, or nullopt if not configurable
**Example**:

```cpp
std::optional<QJsonObject> default_configuration() const override {
    QJsonObject config;
    config["interval"] = 1000;
    config["enabled"] = true;
    return config;
}
```

#### `configure()`

```cpp
virtual expected<void, PluginError> configure(const QJsonObject& config);
```

Configures the plugin with the provided settings.

**Parameters**:

- `config`: Configuration data as JSON object

**Returns**: Success or error information
**Example**:

```cpp
expected<void, PluginError> configure(const QJsonObject& config) override {
    if (!validate_configuration(config)) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration, "Invalid configuration");
    }
    
    m_configuration = config;
    apply_configuration();
    return make_success();
}
```

## Usage Examples

### Basic Plugin Implementation

```cpp
class MyPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(MyPlugin, "com.example.MyPlugin/1.0", "metadata.json")

private:
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    QJsonObject m_configuration;

public:
    explicit MyPlugin(QObject* parent = nullptr) : QObject(parent) {}
    
    // Required metadata
    std::string_view name() const noexcept override { return "My Plugin"; }
    std::string_view description() const noexcept override { return "Example plugin"; }
    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }
    std::string_view author() const noexcept override { return "Developer"; }
    std::string id() const noexcept override { return "com.example.myplugin"; }
    
    // Lifecycle
    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        m_state = qtplugin::PluginState::Running;
        return qtplugin::make_success();
    }
    
    void shutdown() noexcept override {
        m_state = qtplugin::PluginState::Unloaded;
    }
    
    qtplugin::PluginState state() const noexcept override {
        return m_state.load();
    }
    
    // Capabilities
    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return qtplugin::PluginCapability::Service;
    }
    
    // Commands
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override {
        if (command == "hello") {
            QJsonObject result;
            result["message"] = "Hello from plugin!";
            return result;
        }
        
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command"
        );
    }
    
    std::vector<std::string> available_commands() const override {
        return {"hello"};
    }
};
```

## Best Practices

1. **Thread Safety**: All const methods should be thread-safe
2. **Exception Safety**: `shutdown()` must never throw exceptions
3. **Resource Management**: Use RAII for resource cleanup
4. **Error Handling**: Provide meaningful error messages
5. **State Management**: Keep plugin state consistent
6. **Configuration**: Validate configuration data thoroughly

## See Also

- [IServicePlugin](iserviceplugin.md) - Service plugin interface
- [PluginManager](../classes/pluginmanager.md) - Plugin management
- [PluginError](../classes/pluginerror.md) - Error handling
- [Version](../classes/version.md) - Version handling
