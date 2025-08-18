# QtPlugin Examples

This directory contains comprehensive examples demonstrating all features of the QtPlugin system. Each example is fully functional and well-documented to help you understand different aspects of plugin development and usage.

## Example Categories

### 🚀 Basic Examples
- **[Basic Plugin](basic/README.md)** - Simple plugin demonstrating core concepts
- **[Hello World](hello-world/README.md)** - Minimal plugin implementation
- **[Plugin Lifecycle](lifecycle/README.md)** - Complete lifecycle management

### 🔧 Advanced Features
- **[Service Plugin](service-plugin/README.md)** - Background service with timer
- **[UI Plugin](ui-plugin/README.md)** - Custom widget integration
- **[Network Plugin](network-plugin/README.md)** - Network operations and protocols
- **[Data Provider](data-provider/README.md)** - Data processing and transformation
- **[Scripting Plugin](scripting-plugin/README.md)** - Script execution engine

### 🏗️ Architecture Patterns
- **[Plugin Dependencies](dependencies/README.md)** - Dependency resolution and management
- **[Inter-Plugin Communication](communication/README.md)** - Plugin-to-plugin messaging
- **[Configuration Management](configuration/README.md)** - Advanced configuration handling
- **[Hot Reloading](hot-reloading/README.md)** - Dynamic plugin reloading

### 🔒 Security and Validation
- **[Plugin Security](security/README.md)** - Security validation and sandboxing
- **[Digital Signatures](signatures/README.md)** - Plugin signing and verification
- **[Trusted Plugins](trusted-plugins/README.md)** - Trust management system

### ⚡ Performance and Monitoring
- **[Performance Monitoring](performance/README.md)** - Plugin performance tracking
- **[Resource Management](resources/README.md)** - Memory and resource monitoring
- **[Async Operations](async/README.md)** - Asynchronous plugin operations

### 🌐 Cross-Platform
- **[Platform Specific](platform-specific/README.md)** - Platform-specific implementations
- **[Deployment](deployment/README.md)** - Cross-platform deployment strategies
- **[Native Integration](native-integration/README.md)** - OS-specific features

### 🧪 Testing and Debugging
- **[Plugin Testing](testing/README.md)** - Comprehensive testing strategies
- **[Debug Plugin](debug/README.md)** - Debugging tools and techniques
- **[Mock Plugins](mocking/README.md)** - Mock plugins for testing

### 🎯 Real-World Applications
- **[Text Editor Plugins](text-editor/README.md)** - Text editor with plugin system
- **[Media Player](media-player/README.md)** - Media player with codec plugins
- **[Database Tools](database-tools/README.md)** - Database connectivity plugins
- **[Web Browser](web-browser/README.md)** - Browser with extension system

## Quick Start Examples

### 1. Hello World Plugin
The simplest possible plugin:

```cpp
class HelloWorldPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(HelloWorldPlugin, "com.example.HelloWorld/1.0", "metadata.json")

public:
    std::string_view name() const noexcept override { return "Hello World"; }
    std::string_view description() const noexcept override { return "A simple hello world plugin"; }
    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }
    std::string_view author() const noexcept override { return "QtPlugin Team"; }
    std::string id() const noexcept override { return "com.example.helloworld"; }

    qtplugin::expected<void, qtplugin::PluginError> initialize() override {
        return qtplugin::make_success();
    }
    
    void shutdown() noexcept override {}
    qtplugin::PluginState state() const noexcept override { return qtplugin::PluginState::Running; }
    qtplugin::PluginCapabilities capabilities() const noexcept override { return qtplugin::PluginCapability::Service; }
    
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override {
        if (command == "hello") {
            QJsonObject result;
            result["message"] = "Hello, World!";
            return result;
        }
        return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound, "Unknown command");
    }
    
    std::vector<std::string> available_commands() const override { return {"hello"}; }
};
```

### 2. Service Plugin with Timer
A background service that performs periodic tasks:

```cpp
class TimerServicePlugin : public QObject, public qtplugin::IServicePlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(TimerServicePlugin, "com.example.TimerService/1.0", "metadata.json", qtplugin::IServicePlugin)

private:
    std::unique_ptr<QTimer> m_timer;
    int m_tick_count = 0;

public:
    qtplugin::expected<void, qtplugin::PluginError> start_service() override {
        m_timer = std::make_unique<QTimer>(this);
        connect(m_timer.get(), &QTimer::timeout, this, &TimerServicePlugin::on_timer_tick);
        m_timer->start(1000); // 1 second interval
        return qtplugin::make_success();
    }
    
    qtplugin::expected<void, qtplugin::PluginError> stop_service() override {
        if (m_timer) {
            m_timer->stop();
            m_timer.reset();
        }
        return qtplugin::make_success();
    }

private slots:
    void on_timer_tick() {
        ++m_tick_count;
        // Perform periodic task
    }
};
```

### 3. UI Plugin with Custom Widget
A plugin that provides custom UI components:

```cpp
class CustomWidgetPlugin : public QObject, public qtplugin::IUIPlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(CustomWidgetPlugin, "com.example.CustomWidget/1.0", "metadata.json", qtplugin::IUIPlugin)

public:
    std::unique_ptr<QWidget> create_widget(const QString& widget_type, QWidget* parent) override {
        if (widget_type == "custom_button") {
            auto button = std::make_unique<QPushButton>("Custom Button", parent);
            button->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
            return button;
        }
        return nullptr;
    }
    
    QStringList available_widgets() const override {
        return {"custom_button"};
    }
    
    QString widget_category() const override {
        return "Custom Controls";
    }
};
```

## 🔨 Building Examples

### Prerequisites

- QtPlugin library built and installed
- Qt6 development environment
- CMake 3.21+
- C++20 compatible compiler

### Build All Examples

```bash
# From project root
cd examples
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_ALL_EXAMPLES=ON
cmake --build . --parallel
```

### Build Specific Example

```bash
# Build service plugin example
cd examples/service-plugin
mkdir build && cd build
cmake ..
cmake --build .
```

### Using QtPlugin Library

Examples automatically find the QtPlugin library if it's:

1. Built in the standard location (`../lib/build`)
2. Installed system-wide
3. Available via `CMAKE_PREFIX_PATH`

## 🚀 Running Examples

### Service Plugin Example

```bash
cd examples/service-plugin/build
./ServicePluginExample

# Expected output:
# QtPlugin initialized, version: 3.0.0
# Plugin loaded: Advanced Service Plugin v1.0.0
# Service started successfully
# [Demonstrates lifecycle, configuration, performance monitoring]
```

### Basic Plugin Example

```bash
cd examples/basic/build
./BasicPluginExample

# Demonstrates:
# - Plugin loading and initialization
# - Command execution
# - Configuration management
# - Error handling
```

### UI Plugin Example

```bash
cd examples/ui-plugin/build
./UIPluginExample

# Demonstrates:
# - Custom widget creation
# - UI integration
# - Theme management
# - Event handling
```

## Example Structure

Each example follows a consistent structure:

```
example-name/
├── README.md              # Example documentation
├── CMakeLists.txt         # Build configuration
├── metadata.json          # Plugin metadata
├── src/                   # Source files
│   ├── plugin.hpp         # Plugin header
│   ├── plugin.cpp         # Plugin implementation
│   └── main.cpp           # Test application (if applicable)
├── include/               # Public headers (if applicable)
├── tests/                 # Unit tests
│   └── test_plugin.cpp
└── docs/                  # Additional documentation
    └── usage.md
```

## Learning Path

### Beginner
1. Start with [Hello World](hello-world/README.md)
2. Study [Basic Plugin](basic/README.md)
3. Understand [Plugin Lifecycle](lifecycle/README.md)

### Intermediate
1. Explore [Service Plugin](service-plugin/README.md)
2. Learn [Configuration Management](configuration/README.md)
3. Study [Inter-Plugin Communication](communication/README.md)

### Advanced
1. Master [Plugin Dependencies](dependencies/README.md)
2. Implement [Hot Reloading](hot-reloading/README.md)
3. Understand [Security](security/README.md)

### Expert
1. Build [Real-World Applications](text-editor/README.md)
2. Optimize [Performance](performance/README.md)
3. Create [Cross-Platform Solutions](platform-specific/README.md)

## Contributing Examples

We welcome contributions of new examples! Please:

1. Follow the established structure
2. Include comprehensive documentation
3. Add unit tests
4. Ensure cross-platform compatibility
5. Provide clear build instructions

See our [Contributing Guide](../CONTRIBUTING.md) for details.

## Support

If you have questions about the examples:
1. Check the example's README.md file
2. Review the [User Guide](../docs/user-guide/README.md)
3. Search existing GitHub issues
4. Create a new issue with the "examples" label
