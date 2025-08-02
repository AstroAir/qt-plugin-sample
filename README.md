# Advanced Plugin Manager

A comprehensive Qt6 application demonstrating an advanced plugin system with modern C++20 features.

## Features

- **Modern Plugin Architecture**: Sophisticated plugin system with interfaces for UI, Service, Network, Scripting, and Data Provider plugins
- **Qt6 Integration**: Full Qt6 support with Widgets, Quick, QML, Network, and Concurrent modules
- **C++20 Features**: Uses concepts, coroutines, and modern C++ patterns
- **Security Management**: Plugin security validation and sandboxing
- **Performance Monitoring**: Real-time plugin performance tracking
- **Communication Bus**: Inter-plugin communication system
- **Hot Reload**: Dynamic plugin loading and unloading
- **QML Integration**: Modern QML-based UI components
- **Cross-Platform**: Supports Windows, macOS, and Linux

## Requirements

- **CMake**: 3.21 or higher
- **Qt6**: 6.2 or higher with the following modules:
  - Qt6Core
  - Qt6Widgets
  - Qt6Quick
  - Qt6QuickWidgets
  - Qt6Network
  - Qt6Concurrent
  - Qt6Script
  - Qt6Qml
  - Qt6Gui
- **C++20 Compiler**:
  - MSVC 2019 16.11 or higher (Windows)
  - GCC 10 or higher (Linux)
  - Clang 12 or higher (macOS/Linux)

## Building

### Quick Start

#### Windows
```bash
# Using the build script
build.bat release

# Or manually
mkdir build
cd build
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

#### Linux/macOS
```bash
# Using the build script
./build.sh release

# Or manually
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### CMake Presets (Recommended)

This project supports CMake presets for easier configuration:

```bash
# List available presets
cmake --list-presets

# Configure with a preset
cmake --preset=release

# Build with a preset
cmake --build --preset=release
```

Available presets:
- `default`: Default configuration with Ninja
- `debug`: Debug build with testing enabled
- `release`: Optimized release build
- `vs2022`: Visual Studio 2022 (Windows only)
- `xcode`: Xcode (macOS only)
- `mingw`: MinGW build (Windows only)

### Build Options

- `BUILD_TESTING`: Enable unit tests (default: OFF)
- `BUILD_DOCUMENTATION`: Build documentation with Doxygen (default: OFF)

Example:
```bash
cmake -DBUILD_TESTING=ON -DBUILD_DOCUMENTATION=ON ..
```

## Project Structure

```
qt-plugin-sample/
├── CMakeLists.txt              # Main CMake configuration
├── CMakePresets.json           # CMake presets for different configurations
├── build.bat                   # Windows build script
├── build.sh                    # Unix build script
├── src/                        # Source code
│   ├── CMakeLists.txt         # Source CMake configuration
│   ├── main.cpp               # Application entry point
│   ├── MainWindow.h/cpp       # Main application window
│   ├── PluginInterface.h      # Base plugin interface
│   ├── AdvancedInterfaces.h   # Extended plugin interfaces
│   ├── PluginManager.h/cpp    # Plugin management system
│   ├── PluginRegistry.h/cpp   # Plugin registry and model
│   ├── PluginCommunicationBus.h/cpp  # Inter-plugin communication
│   └── PluginSecurityManager.h/cpp   # Plugin security management
├── resources/                  # Application resources
│   ├── resources.qrc          # Qt resource file
│   ├── *.svg                  # Icon files
│   └── *.qss                  # Stylesheets
├── qml/                       # QML files
│   └── PluginManagerView.qml  # QML plugin manager interface
└── README.md                  # This file
```

## Usage

After building, run the application:

### Windows
```bash
cd build/Release/src
AdvancedPluginManager.exe
```

### Linux/macOS
```bash
cd build/src
./AdvancedPluginManager
```

The application will create a `plugins` directory where you can place plugin libraries.

## Plugin Development

Plugins should implement one or more of the provided interfaces:

- `IPlugin`: Base plugin interface
- `IUIPlugin`: UI plugins that provide widgets
- `IServicePlugin`: Background service plugins
- `INetworkPlugin`: Network-related plugins
- `IScriptingPlugin`: Scripting engine plugins
- `IDataProviderPlugin`: Data processing plugins

See the header files in `src/` for detailed interface documentation.

## License

This project is provided as a sample/educational resource. See LICENSE file for details.
