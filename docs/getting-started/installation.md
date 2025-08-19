# Installation Guide

This guide covers all the ways to install and integrate QtPlugin into your project.

## Prerequisites

Before installing QtPlugin, ensure you have the following requirements:

### System Requirements

=== "Windows"

    - **OS**: Windows 10 or later
    - **Compiler**: MSVC 2019 16.11+ or MinGW-w64 with GCC 10+
    - **CMake**: 3.21 or higher
    - **Qt6**: 6.2 or higher

=== "macOS"

    - **OS**: macOS 10.15 (Catalina) or later
    - **Compiler**: Clang 12+ (Xcode 12+)
    - **CMake**: 3.21 or higher
    - **Qt6**: 6.2 or higher

=== "Linux"

    - **OS**: Ubuntu 20.04+, CentOS 8+, or equivalent
    - **Compiler**: GCC 10+ or Clang 12+
    - **CMake**: 3.21 or higher
    - **Qt6**: 6.2 or higher

### Qt6 Modules

QtPlugin requires the following Qt6 modules:

- **Qt6::Core** (required) - Core functionality
- **Qt6::Network** (optional) - For network plugins
- **Qt6::Widgets** (optional) - For UI plugins
- **Qt6::Test** (optional) - For testing

## Installation Methods

### Method 1: CMake FetchContent (Recommended)

The easiest way to integrate QtPlugin is using CMake's FetchContent:

```cmake
cmake_minimum_required(VERSION 3.21)
project(MyApplication)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core)

# Fetch QtPlugin
include(FetchContent)
FetchContent_Declare(
    QtPlugin
    GIT_REPOSITORY https://github.com/example/qt-plugin-sample.git
    GIT_TAG        v3.0.0
    SOURCE_SUBDIR  lib
)
FetchContent_MakeAvailable(QtPlugin)

# Create your application
add_executable(MyApplication main.cpp)

# Link QtPlugin
target_link_libraries(MyApplication 
    PRIVATE 
    QtPlugin::Core
    Qt6::Core
)
```

### Method 2: Git Submodule

Add QtPlugin as a git submodule to your project:

```bash
# Add QtPlugin as submodule
git submodule add https://github.com/example/qt-plugin-sample.git third-party/qtplugin
git submodule update --init --recursive

# In your CMakeLists.txt
add_subdirectory(third-party/qtplugin/lib)
target_link_libraries(MyApplication PRIVATE QtPlugin::Core)
```

### Method 3: Manual Installation

1. **Download and Build QtPlugin**:

```bash
git clone https://github.com/example/qt-plugin-sample.git
cd qt-plugin-sample
mkdir build && cd build
cmake ../lib -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build . --parallel
cmake --install .
```

2. **Use in Your Project**:

```cmake
find_package(QtPlugin REQUIRED)
target_link_libraries(MyApplication PRIVATE QtPlugin::Core)
```

### Method 4: Package Managers

=== "vcpkg"

    ```bash
    # Install QtPlugin via vcpkg
    vcpkg install qtplugin
    ```

    ```cmake
    find_package(QtPlugin CONFIG REQUIRED)
    target_link_libraries(MyApplication PRIVATE QtPlugin::Core)
    ```

=== "Conan"

    ```ini
    # conanfile.txt
    [requires]
    qtplugin/3.0.0
    
    [generators]
    CMakeDeps
    CMakeToolchain
    ```

    ```cmake
    find_package(QtPlugin REQUIRED)
    target_link_libraries(MyApplication PRIVATE QtPlugin::Core)
    ```

## Build Configuration

### CMake Options

QtPlugin provides several CMake options to customize the build:

```cmake
# Core options
option(QTPLUGIN_BUILD_TESTS "Build QtPlugin tests" OFF)
option(QTPLUGIN_BUILD_EXAMPLES "Build QtPlugin examples" OFF)
option(QTPLUGIN_BUILD_DOCS "Build QtPlugin documentation" OFF)

# Feature options
option(QTPLUGIN_ENABLE_SECURITY "Enable security features" ON)
option(QTPLUGIN_ENABLE_NETWORK "Enable network plugin support" ON)
option(QTPLUGIN_ENABLE_UI "Enable UI plugin support" ON)

# Development options
option(QTPLUGIN_ENABLE_LOGGING "Enable debug logging" ON)
option(QTPLUGIN_ENABLE_PROFILING "Enable performance profiling" OFF)
```

### Example Configuration

```cmake
# Configure QtPlugin with custom options
set(QTPLUGIN_BUILD_EXAMPLES ON)
set(QTPLUGIN_ENABLE_NETWORK OFF)  # Disable network support
set(QTPLUGIN_ENABLE_PROFILING ON) # Enable profiling

FetchContent_Declare(
    QtPlugin
    GIT_REPOSITORY https://github.com/example/qt-plugin-sample.git
    GIT_TAG        v3.0.0
    SOURCE_SUBDIR  lib
)
FetchContent_MakeAvailable(QtPlugin)
```

## Verification

After installation, verify that QtPlugin is working correctly:

### 1. Create a Test Application

```cpp
// test_qtplugin.cpp
#include <qtplugin/qtplugin.hpp>
#include <iostream>

int main() {
    // Initialize QtPlugin
    qtplugin::LibraryInitializer init;
    
    if (!init.is_initialized()) {
        std::cerr << "Failed to initialize QtPlugin" << std::endl;
        return 1;
    }
    
    std::cout << "QtPlugin initialized successfully!" << std::endl;
    std::cout << "Version: " << qtplugin::version() << std::endl;
    
    // Create plugin manager
    qtplugin::PluginManager manager;
    std::cout << "Plugin manager created successfully!" << std::endl;
    
    return 0;
}
```

### 2. Build and Run

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.21)
project(TestQtPlugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

# Add QtPlugin (using your chosen method)
include(FetchContent)
FetchContent_Declare(
    QtPlugin
    GIT_REPOSITORY https://github.com/example/qt-plugin-sample.git
    GIT_TAG        v3.0.0
    SOURCE_SUBDIR  lib
)
FetchContent_MakeAvailable(QtPlugin)

add_executable(TestQtPlugin test_qtplugin.cpp)
target_link_libraries(TestQtPlugin PRIVATE QtPlugin::Core Qt6::Core)
```

```bash
mkdir build && cd build
cmake ..
cmake --build .
./TestQtPlugin  # or TestQtPlugin.exe on Windows
```

Expected output:

```
QtPlugin initialized successfully!
Version: 3.0.0
Plugin manager created successfully!
```

## Troubleshooting

### Common Issues

#### CMake Cannot Find Qt6

```bash
# Set Qt6 installation path
export CMAKE_PREFIX_PATH="/path/to/qt6"
# or on Windows
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
```

#### Compiler Errors About C++20

Ensure your compiler supports C++20:

```cmake
# Force C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For older CMake versions
target_compile_features(MyApplication PRIVATE cxx_std_20)
```

#### Link Errors

Make sure you're linking all required libraries:

```cmake
target_link_libraries(MyApplication 
    PRIVATE 
    QtPlugin::Core
    Qt6::Core
    # Add other Qt modules as needed
    Qt6::Network
    Qt6::Widgets
)
```

#### Runtime Errors

Check that Qt6 libraries are in your PATH:

=== "Windows"

    ```cmd
    set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%PATH%
    ```

=== "Linux/macOS"

    ```bash
    export LD_LIBRARY_PATH=/path/to/qt6/lib:$LD_LIBRARY_PATH
    ```

### Getting Help

If you encounter issues:

1. Check the [FAQ](../reference/faq.md)
2. Review [Troubleshooting Guide](../user-guide/troubleshooting.md)
3. Search [GitHub Issues](https://github.com/example/qt-plugin-sample/issues)
4. Create a new issue with:
   - Your platform and compiler version
   - Qt6 version
   - Complete error messages
   - Minimal reproduction case

## Platform-Specific Setup

### Windows Development

#### Visual Studio Integration

1. **Install Qt6 with Visual Studio support**:

   ```cmd
   # Using Qt Online Installer
   # Select: Qt 6.5.x -> MSVC 2019 64-bit
   # Add Qt VS Tools extension to Visual Studio
   ```

2. **Configure environment variables**:

   ```cmd
   set Qt6_DIR=C:\Qt\6.5.0\msvc2019_64
   set CMAKE_PREFIX_PATH=%Qt6_DIR%
   set PATH=%Qt6_DIR%\bin;%PATH%
   ```

3. **CMake configuration for Visual Studio**:

   ```cmake
   # Use specific generator
   cmake -G "Visual Studio 16 2019" -A x64 ..
   # Or for Visual Studio 2022
   cmake -G "Visual Studio 17 2022" -A x64 ..
   ```

#### MinGW-w64 Setup

1. **Install MinGW-w64**:

   ```cmd
   # Using MSYS2
   pacman -S mingw-w64-x86_64-gcc
   pacman -S mingw-w64-x86_64-cmake
   pacman -S mingw-w64-x86_64-qt6
   ```

2. **Build with MinGW**:

   ```cmd
   mkdir build && cd build
   cmake -G "MinGW Makefiles" ..
   mingw32-make -j4
   ```

### macOS Development

#### Xcode Integration

1. **Install Qt6 via Homebrew**:

   ```bash
   brew install qt6
   brew install cmake
   ```

2. **Configure environment**:

   ```bash
   export Qt6_DIR=$(brew --prefix qt6)
   export CMAKE_PREFIX_PATH=$Qt6_DIR
   export PATH=$Qt6_DIR/bin:$PATH
   ```

3. **Xcode project generation**:

   ```bash
   cmake -G Xcode ..
   open QtPluginProject.xcodeproj
   ```

#### Universal Binaries

For Apple Silicon and Intel support:

```cmake
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
```

### Linux Development

#### Ubuntu/Debian Setup

1. **Install dependencies**:

   ```bash
   sudo apt update
   sudo apt install build-essential cmake
   sudo apt install qt6-base-dev qt6-tools-dev
   sudo apt install libqt6core6 libqt6gui6 libqt6widgets6
   ```

2. **Development packages**:

   ```bash
   sudo apt install qt6-base-dev-tools
   sudo apt install qt6-l10n-tools
   sudo apt install qt6-documentation-tools
   ```

#### CentOS/RHEL/Fedora Setup

1. **Install dependencies**:

   ```bash
   # Fedora
   sudo dnf install gcc-c++ cmake
   sudo dnf install qt6-qtbase-devel qt6-qttools-devel

   # CentOS/RHEL (with EPEL)
   sudo yum install gcc-c++ cmake3
   sudo yum install qt6-qtbase-devel qt6-qttools-devel
   ```

#### Arch Linux Setup

```bash
sudo pacman -S base-devel cmake
sudo pacman -S qt6-base qt6-tools
sudo pacman -S qt6-doc qt6-examples
```

## Advanced Configuration

### Custom Build Options

```cmake
# QtPlugin build options
option(QTPLUGIN_BUILD_TESTS "Build QtPlugin tests" OFF)
option(QTPLUGIN_BUILD_EXAMPLES "Build QtPlugin examples" OFF)
option(QTPLUGIN_BUILD_DOCS "Build QtPlugin documentation" OFF)
option(QTPLUGIN_BUILD_BENCHMARKS "Build performance benchmarks" OFF)

# Feature options
option(QTPLUGIN_ENABLE_SECURITY "Enable security features" ON)
option(QTPLUGIN_ENABLE_NETWORK "Enable network plugin support" ON)
option(QTPLUGIN_ENABLE_UI "Enable UI plugin support" ON)
option(QTPLUGIN_ENABLE_SCRIPTING "Enable scripting support" OFF)

# Development options
option(QTPLUGIN_ENABLE_LOGGING "Enable debug logging" ON)
option(QTPLUGIN_ENABLE_PROFILING "Enable performance profiling" OFF)
option(QTPLUGIN_ENABLE_SANITIZERS "Enable sanitizers" OFF)

# Configure with custom options
cmake -DQTPLUGIN_BUILD_EXAMPLES=ON \
      -DQTPLUGIN_ENABLE_PROFILING=ON \
      -DQTPLUGIN_ENABLE_SANITIZERS=ON \
      ..
```

### Cross-Compilation

#### Android Cross-Compilation

```cmake
# Android toolchain setup
set(CMAKE_TOOLCHAIN_FILE $ENV{ANDROID_NDK}/build/cmake/android.toolchain.cmake)
set(ANDROID_ABI "arm64-v8a")
set(ANDROID_PLATFORM android-21)

# Qt6 for Android
find_package(Qt6 REQUIRED COMPONENTS Core)
```

#### Embedded Linux Cross-Compilation

```cmake
# Cross-compilation toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Qt6 cross-compiled installation
set(CMAKE_PREFIX_PATH /opt/qt6-arm)
```

## Development Environment Setup

### IDE Configuration

#### Qt Creator

1. **Configure Qt Kit**:
   - Tools → Options → Kits
   - Add Qt6 installation
   - Configure compiler and debugger

2. **Project setup**:

   ```cmake
   # .pro file alternative (if using qmake)
   QT += core
   CONFIG += c++20
   TARGET = MyQtPluginApp
   ```

#### Visual Studio Code

1. **Install extensions**:
   - C/C++ Extension Pack
   - CMake Tools
   - Qt tools

2. **Configure settings.json**:

   ```json
   {
       "cmake.configureArgs": [
           "-DCMAKE_PREFIX_PATH=/path/to/qt6"
       ],
       "C_Cpp.default.cppStandard": "c++20",
       "C_Cpp.default.compilerPath": "/usr/bin/g++"
   }
   ```

#### CLion

1. **Configure toolchain**:
   - File → Settings → Build, Execution, Deployment → Toolchains
   - Add system toolchain or custom toolchain

2. **CMake configuration**:
   - File → Settings → Build, Execution, Deployment → CMake
   - Add CMAKE_PREFIX_PATH for Qt6

### Debugging Setup

#### GDB Configuration

```bash
# .gdbinit for Qt debugging
set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set demangle-style gnu-v3
```

#### Valgrind Integration

```bash
# Memory leak detection
valgrind --tool=memcheck --leak-check=full ./your_app

# Performance profiling
valgrind --tool=callgrind ./your_app
```

## Next Steps

Now that QtPlugin is installed and configured:

1. **[Quick Start](quick-start.md)** - Build your first plugin-enabled application
2. **[First Plugin](first-plugin.md)** - Create your first plugin
3. **[User Guide](../user-guide/index.md)** - Learn about plugin management
4. **[Examples](../examples/index.md)** - Explore working examples
5. **[Developer Guide](../developer-guide/index.md)** - Advanced plugin development

---

**Next**: [Quick Start Guide](quick-start.md) to build your first application.
