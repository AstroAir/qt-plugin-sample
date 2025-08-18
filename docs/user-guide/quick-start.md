# Quick Start Guide

Get up and running with QtPlugin in just a few minutes!

## Prerequisites

- **CMake**: 3.21 or higher
- **Qt6**: 6.2 or higher (Core module required)
- **C++20 Compiler**: GCC 10+, Clang 12+, or MSVC 2019+

## Installation

### Option 1: Using CMake FetchContent (Recommended)

Add this to your `CMakeLists.txt`:

```cmake
include(FetchContent)
FetchContent_Declare(
    QtPlugin
    GIT_REPOSITORY https://github.com/example/qtplugin.git
    GIT_TAG        v3.0.0
)
FetchContent_MakeAvailable(QtPlugin)

target_link_libraries(your_app QtPlugin::Core)
```

### Option 2: Using find_package

If QtPlugin is installed system-wide:

```cmake
find_package(QtPlugin REQUIRED COMPONENTS Core)
target_link_libraries(your_app QtPlugin::Core)
```

### Option 3: Building from Source

```bash
git clone https://github.com/example/qtplugin.git
cd qtplugin/lib
mkdir build && cd build
cmake .. -DQTPLUGIN_BUILD_EXAMPLES=ON
cmake --build .
cmake --install . --prefix /usr/local
```

## Your First Application

Create a simple application that loads and uses a plugin:

### main.cpp

```cpp
#include <qtplugin/qtplugin.hpp>
#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // Initialize QtPlugin library
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        std::cerr << "Failed to initialize QtPlugin library" << std::endl;
        return -1;
    }
    
    std::cout << "QtPlugin initialized, version: " << QTPLUGIN_VERSION << std::endl;
    
    // Create plugin manager
    qtplugin::PluginManager manager;
    
    // Load a plugin
    auto load_result = manager.load_plugin("./plugins/basic_plugin.qtplugin");
    if (!load_result) {
        std::cerr << "Failed to load plugin: " << load_result.error().message << std::endl;
        return -1;
    }
    
    std::string plugin_id = load_result.value();
    std::cout << "Plugin loaded with ID: " << plugin_id << std::endl;
    
    // Get the plugin instance
    auto plugin = manager.get_plugin(plugin_id);
    if (!plugin) {
        std::cerr << "Failed to get plugin instance" << std::endl;
        return -1;
    }
    
    // Display plugin information
    std::cout << "Plugin Name: " << plugin->name() << std::endl;
    std::cout << "Plugin Version: " << plugin->version().to_string() << std::endl;
    std::cout << "Plugin Author: " << plugin->author() << std::endl;
    
    // Initialize the plugin
    auto init_result = plugin->initialize();
    if (!init_result) {
        std::cerr << "Failed to initialize plugin: " << init_result.error().message << std::endl;
        return -1;
    }
    
    std::cout << "Plugin initialized successfully!" << std::endl;
    
    // Execute a command
    auto cmd_result = plugin->execute_command("status");
    if (cmd_result) {
        std::cout << "Plugin status: " << cmd_result.value().toJson() << std::endl;
    }
    
    // List available commands
    auto commands = plugin->available_commands();
    std::cout << "Available commands: ";
    for (const auto& cmd : commands) {
        std::cout << cmd << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.21)
project(QtPluginExample)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

# Add QtPlugin
include(FetchContent)
FetchContent_Declare(
    QtPlugin
    GIT_REPOSITORY https://github.com/example/qtplugin.git
    GIT_TAG        v3.0.0
)
FetchContent_MakeAvailable(QtPlugin)

# Create executable
add_executable(QtPluginExample main.cpp)

# Link libraries
target_link_libraries(QtPluginExample 
    Qt6::Core 
    QtPlugin::Core
)

# Copy example plugin to output directory
add_custom_command(TARGET QtPluginExample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:QtPluginExample>/plugins
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        $<TARGET_FILE:basic_plugin> 
        $<TARGET_FILE_DIR:QtPluginExample>/plugins/
)
```

## Building and Running

```bash
mkdir build && cd build
cmake ..
cmake --build .
./QtPluginExample
```

Expected output:

```
QtPlugin initialized, version: 3.0.0
Plugin loaded with ID: com.example.basic_plugin
Plugin Name: Basic Example Plugin
Plugin Version: 1.0.0
Plugin Author: QtPlugin Team
Plugin initialized successfully!
Plugin status: {"state":"running","uptime":0}
Available commands: status echo metrics configure test
```

## Creating Your First Plugin

### plugin.hpp

```cpp
#pragma once

#include <qtplugin/qtplugin.hpp>
#include <QObject>

class MyFirstPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    QTPLUGIN_DECLARE_PLUGIN(MyFirstPlugin, "com.example.MyFirstPlugin/1.0", "metadata.json")

public:
    explicit MyFirstPlugin(QObject* parent = nullptr);
    ~MyFirstPlugin() override;

    // IPlugin interface
    std::string_view name() const noexcept override { return "My First Plugin"; }
    std::string_view description() const noexcept override { return "A simple example plugin"; }
    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }
    std::string_view author() const noexcept override { return "Your Name"; }
    std::string id() const noexcept override { return "com.example.myfirstplugin"; }

    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override { return m_state; }
    qtplugin::PluginCapabilities capabilities() const noexcept override;

    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override;
    
    std::vector<std::string> available_commands() const override;

private:
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
};
```

### plugin.cpp

```cpp
#include "plugin.hpp"
#include <QJsonObject>

MyFirstPlugin::MyFirstPlugin(QObject* parent) : QObject(parent) {}

MyFirstPlugin::~MyFirstPlugin() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, qtplugin::PluginError> MyFirstPlugin::initialize() {
    m_state = qtplugin::PluginState::Running;
    return qtplugin::make_success();
}

void MyFirstPlugin::shutdown() noexcept {
    m_state = qtplugin::PluginState::Unloaded;
}

qtplugin::PluginCapabilities MyFirstPlugin::capabilities() const noexcept {
    return qtplugin::PluginCapability::Service;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError>
MyFirstPlugin::execute_command(std::string_view command, const QJsonObject& params) {
    if (command == "hello") {
        QJsonObject result;
        result["message"] = "Hello from My First Plugin!";
        return result;
    }
    
    return qtplugin::make_error<QJsonObject>(
        qtplugin::PluginErrorCode::CommandNotFound,
        "Unknown command: " + std::string(command)
    );
}

std::vector<std::string> MyFirstPlugin::available_commands() const {
    return {"hello"};
}

#include "plugin.moc"
```

## Next Steps

- Read the [User Guide](README.md) for comprehensive documentation
- Explore [Examples](../examples/README.md) for more complex scenarios
- Check the [API Reference](../api/README.md) for detailed API documentation
- Learn about [Plugin Development](../developer-guide/plugin-development.md)

## Common Issues

### Plugin Not Loading

- Check file path and permissions
- Verify plugin is built for correct architecture
- Check Qt version compatibility

### Initialization Failures

- Review plugin dependencies
- Check configuration requirements
- Verify Qt modules are available

### Build Issues

- Ensure C++20 compiler support
- Verify Qt6 installation
- Check CMake version (3.21+)

For more troubleshooting help, see the [Troubleshooting Guide](troubleshooting.md).
