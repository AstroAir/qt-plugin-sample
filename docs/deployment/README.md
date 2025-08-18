# QtPlugin Deployment Guide

This guide covers deploying applications that use the QtPlugin library across different platforms, including plugin distribution, dependency management, and packaging strategies.

## Table of Contents

1. [Deployment Overview](#deployment-overview)
2. [Platform-Specific Deployment](#platform-specific-deployment)
3. [Plugin Distribution](#plugin-distribution)
4. [Dependency Management](#dependency-management)
5. [Packaging Strategies](#packaging-strategies)
6. [Security Considerations](#security-considerations)

## Deployment Overview

### Deployment Components

A typical QtPlugin application deployment includes:

```
MyApplication/
├── bin/
│   ├── MyApplication(.exe)          # Main application
│   ├── qtplugin-core(.dll/.so/.dylib) # QtPlugin library
│   └── Qt6Core(.dll/.so/.dylib)     # Qt libraries
├── plugins/
│   ├── plugin1.qtplugin             # Plugin libraries
│   ├── plugin2.qtplugin
│   └── metadata/                    # Plugin metadata
│       ├── plugin1.json
│       └── plugin2.json
├── config/
│   ├── app.conf                     # Application configuration
│   └── plugins.conf                 # Plugin configuration
└── resources/
    ├── icons/
    ├── themes/
    └── translations/
```

### Deployment Checklist

- [ ] Application executable
- [ ] QtPlugin library (qtplugin-core)
- [ ] Qt6 runtime libraries
- [ ] Plugin libraries (.qtplugin files)
- [ ] Plugin metadata files
- [ ] Configuration files
- [ ] Resource files
- [ ] Platform-specific dependencies

## Platform-Specific Deployment

### Windows Deployment

#### Using Qt Deployment Tool

```cmd
# Navigate to build output directory
cd build\Release

# Deploy Qt dependencies
windeployqt.exe MyApplication.exe --dir deployment --qmldir ..\..\qml

# Copy QtPlugin library
copy qtplugin-core.dll deployment\

# Copy plugins
mkdir deployment\plugins
copy plugins\*.qtplugin deployment\plugins\
copy plugins\*.json deployment\plugins\
```

#### Manual Deployment

```cmd
# Create deployment directory
mkdir deployment
cd deployment

# Copy application
copy ..\MyApplication.exe .

# Copy Qt libraries (minimum required)
copy "C:\Qt\6.5.0\msvc2019_64\bin\Qt6Core.dll" .
copy "C:\Qt\6.5.0\msvc2019_64\bin\Qt6Gui.dll" .
copy "C:\Qt\6.5.0\msvc2019_64\bin\Qt6Widgets.dll" .

# Copy Visual C++ Redistributable (if needed)
copy "C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.29.30133\x64\Microsoft.VC142.CRT\*.dll" .

# Copy QtPlugin library
copy ..\qtplugin-core.dll .

# Copy plugins
mkdir plugins
copy ..\plugins\*.qtplugin plugins\
copy ..\plugins\*.json plugins\
```

#### Creating Windows Installer

Using NSIS:

```nsis
; QtPlugin Application Installer
!define APP_NAME "MyApplication"
!define APP_VERSION "1.0.0"

OutFile "${APP_NAME}-${APP_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"

Section "Main Application"
    SetOutPath "$INSTDIR"
    File "MyApplication.exe"
    File "qtplugin-core.dll"
    File "Qt6*.dll"
    
    SetOutPath "$INSTDIR\plugins"
    File "plugins\*.qtplugin"
    File "plugins\*.json"
    
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd
```

### macOS Deployment

#### Using macdeployqt

```bash
# Build application bundle
cmake --build . --target package

# Deploy Qt dependencies
macdeployqt MyApplication.app -verbose=2

# Copy QtPlugin library into bundle
cp libqtplugin-core.dylib MyApplication.app/Contents/Frameworks/

# Copy plugins
mkdir -p MyApplication.app/Contents/PlugIns/qtplugin
cp plugins/*.qtplugin MyApplication.app/Contents/PlugIns/qtplugin/
cp plugins/*.json MyApplication.app/Contents/PlugIns/qtplugin/

# Fix library paths
install_name_tool -change libqtplugin-core.dylib @executable_path/../Frameworks/libqtplugin-core.dylib MyApplication.app/Contents/MacOS/MyApplication
```

#### Creating DMG Package

```bash
# Create temporary directory
mkdir dmg_temp
cp -R MyApplication.app dmg_temp/

# Create DMG
hdiutil create -volname "MyApplication" -srcfolder dmg_temp -ov -format UDZO MyApplication-1.0.0.dmg

# Clean up
rm -rf dmg_temp
```

#### App Store Distribution

For Mac App Store distribution:

1. Enable sandboxing in entitlements
2. Use hardened runtime
3. Notarize the application
4. Follow App Store guidelines for plugin loading

### Linux Deployment

#### Using linuxdeployqt

```bash
# Install linuxdeployqt
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod +x linuxdeployqt-continuous-x86_64.AppImage

# Create AppDir structure
mkdir -p MyApplication.AppDir/usr/bin
mkdir -p MyApplication.AppDir/usr/lib
mkdir -p MyApplication.AppDir/usr/share/applications
mkdir -p MyApplication.AppDir/usr/share/icons

# Copy application
cp MyApplication MyApplication.AppDir/usr/bin/

# Copy QtPlugin library
cp libqtplugin-core.so MyApplication.AppDir/usr/lib/

# Copy plugins
mkdir -p MyApplication.AppDir/usr/lib/qtplugin
cp plugins/*.qtplugin MyApplication.AppDir/usr/lib/qtplugin/
cp plugins/*.json MyApplication.AppDir/usr/lib/qtplugin/

# Deploy Qt dependencies
./linuxdeployqt-continuous-x86_64.AppImage MyApplication.AppDir/usr/bin/MyApplication -appimage
```

#### Creating DEB Package

```bash
# Create package structure
mkdir -p myapp-1.0.0/DEBIAN
mkdir -p myapp-1.0.0/usr/bin
mkdir -p myapp-1.0.0/usr/lib/myapp
mkdir -p myapp-1.0.0/usr/share/applications

# Create control file
cat > myapp-1.0.0/DEBIAN/control << EOF
Package: myapp
Version: 1.0.0
Section: utils
Priority: optional
Architecture: amd64
Depends: libqt6core6, libqt6gui6, libqt6widgets6
Maintainer: Your Name <your.email@example.com>
Description: My QtPlugin Application
 A sample application using QtPlugin library.
EOF

# Copy files
cp MyApplication myapp-1.0.0/usr/bin/
cp libqtplugin-core.so myapp-1.0.0/usr/lib/myapp/
cp plugins/*.qtplugin myapp-1.0.0/usr/lib/myapp/
cp plugins/*.json myapp-1.0.0/usr/lib/myapp/

# Build package
dpkg-deb --build myapp-1.0.0
```

#### Creating RPM Package

```spec
Name:           myapp
Version:        1.0.0
Release:        1%{?dist}
Summary:        My QtPlugin Application

License:        MIT
URL:            https://example.com/myapp
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.21
BuildRequires:  qt6-qtbase-devel
Requires:       qt6-qtbase

%description
A sample application using QtPlugin library.

%prep
%autosetup

%build
%cmake
%cmake_build

%install
%cmake_install

%files
%{_bindir}/MyApplication
%{_libdir}/myapp/
%{_datadir}/applications/myapp.desktop

%changelog
* Mon Jan 01 2024 Your Name <your.email@example.com> - 1.0.0-1
- Initial package
```

## Plugin Distribution

### Plugin Package Structure

```
MyPlugin-1.0.0/
├── plugin/
│   ├── myplugin.qtplugin           # Plugin library
│   └── metadata.json              # Plugin metadata
├── config/
│   └── default.conf               # Default configuration
├── resources/
│   ├── icons/
│   └── translations/
├── docs/
│   ├── README.md
│   └── CHANGELOG.md
└── install.sh                     # Installation script
```

### Plugin Installation Script

```bash
#!/bin/bash
# Plugin installation script

PLUGIN_NAME="myplugin"
PLUGIN_VERSION="1.0.0"
INSTALL_DIR="${HOME}/.local/share/myapp/plugins"

echo "Installing ${PLUGIN_NAME} v${PLUGIN_VERSION}..."

# Create plugin directory
mkdir -p "${INSTALL_DIR}/${PLUGIN_NAME}"

# Copy plugin files
cp plugin/${PLUGIN_NAME}.qtplugin "${INSTALL_DIR}/${PLUGIN_NAME}/"
cp plugin/metadata.json "${INSTALL_DIR}/${PLUGIN_NAME}/"

# Copy configuration
if [ -f config/default.conf ]; then
    cp config/default.conf "${INSTALL_DIR}/${PLUGIN_NAME}/"
fi

# Copy resources
if [ -d resources ]; then
    cp -r resources "${INSTALL_DIR}/${PLUGIN_NAME}/"
fi

echo "Plugin installed successfully to ${INSTALL_DIR}/${PLUGIN_NAME}"
```

### Plugin Registry

Create a plugin registry for easy discovery:

```json
{
    "registry_version": "1.0",
    "plugins": [
        {
            "id": "com.example.myplugin",
            "name": "My Plugin",
            "version": "1.0.0",
            "description": "Example plugin",
            "author": "Plugin Developer",
            "download_url": "https://example.com/plugins/myplugin-1.0.0.zip",
            "checksum": "sha256:abc123...",
            "dependencies": [],
            "platforms": ["windows", "macos", "linux"],
            "qt_version": "6.2.0"
        }
    ]
}
```

## Dependency Management

### Qt Dependencies

#### Minimal Qt Dependencies

```
Qt6Core - Always required
Qt6Gui - For GUI applications
Qt6Widgets - For widget-based UI
Qt6Network - For network plugins
Qt6Concurrent - For threading support
```

#### Dependency Detection Script

```bash
#!/bin/bash
# Detect Qt dependencies

BINARY="$1"
if [ -z "$BINARY" ]; then
    echo "Usage: $0 <binary>"
    exit 1
fi

echo "Analyzing dependencies for: $BINARY"

case "$(uname)" in
    Linux)
        ldd "$BINARY" | grep -i qt
        ;;
    Darwin)
        otool -L "$BINARY" | grep -i qt
        ;;
    *)
        echo "Unsupported platform"
        exit 1
        ;;
esac
```

### Plugin Dependencies

#### Dependency Resolution

```cpp
// Example dependency resolution in application
class PluginDependencyResolver {
public:
    bool resolve_dependencies(const std::string& plugin_id) {
        auto plugin_info = get_plugin_info(plugin_id);
        
        for (const auto& dep : plugin_info.dependencies) {
            if (!is_plugin_loaded(dep)) {
                if (!load_plugin(dep)) {
                    return false;
                }
            }
        }
        
        return true;
    }
};
```

## Packaging Strategies

### Container Deployment

#### Docker Container

```dockerfile
FROM ubuntu:22.04

# Install Qt6 runtime
RUN apt-get update && apt-get install -y \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    && rm -rf /var/lib/apt/lists/*

# Copy application
COPY MyApplication /usr/local/bin/
COPY libqtplugin-core.so /usr/local/lib/
COPY plugins/ /usr/local/lib/qtplugin/

# Set library path
ENV LD_LIBRARY_PATH=/usr/local/lib

# Run application
CMD ["/usr/local/bin/MyApplication"]
```

### Snap Package

```yaml
name: myapp
version: '1.0.0'
summary: My QtPlugin Application
description: A sample application using QtPlugin library

base: core22
confinement: strict

apps:
  myapp:
    command: bin/MyApplication
    plugs: [home, network]

parts:
  myapp:
    plugin: cmake
    source: .
    build-packages:
      - qtbase6-dev
    stage-packages:
      - libqt6core6
      - libqt6gui6
      - libqt6widgets6
```

### Flatpak Package

```json
{
    "app-id": "com.example.MyApp",
    "runtime": "org.kde.Platform",
    "runtime-version": "6.5",
    "sdk": "org.kde.Sdk",
    "command": "MyApplication",
    "finish-args": [
        "--share=ipc",
        "--socket=x11",
        "--filesystem=home"
    ],
    "modules": [
        {
            "name": "myapp",
            "buildsystem": "cmake",
            "sources": [
                {
                    "type": "dir",
                    "path": "."
                }
            ]
        }
    ]
}
```

## Security Considerations

### Plugin Validation

```cpp
// Example plugin validation
class PluginValidator {
public:
    bool validate_plugin(const std::string& plugin_path) {
        // Check file signature
        if (!verify_signature(plugin_path)) {
            return false;
        }
        
        // Check plugin metadata
        if (!validate_metadata(plugin_path)) {
            return false;
        }
        
        // Check for malicious code patterns
        if (!scan_for_threats(plugin_path)) {
            return false;
        }
        
        return true;
    }
};
```

### Sandboxing

```cpp
// Example plugin sandboxing
class PluginSandbox {
public:
    void configure_sandbox(const std::string& plugin_id) {
        // Restrict file system access
        restrict_filesystem_access(plugin_id);
        
        // Limit network access
        restrict_network_access(plugin_id);
        
        // Set resource limits
        set_resource_limits(plugin_id);
    }
};
```

### Code Signing

#### Windows Code Signing

```cmd
# Sign executable
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com MyApplication.exe

# Sign plugin
signtool sign /f certificate.pfx /p password /t http://timestamp.digicert.com myplugin.qtplugin
```

#### macOS Code Signing

```bash
# Sign application
codesign --force --verify --verbose --sign "Developer ID Application: Your Name" MyApplication.app

# Sign plugin
codesign --force --verify --verbose --sign "Developer ID Application: Your Name" myplugin.qtplugin
```

## Deployment Testing

### Automated Testing Script

```bash
#!/bin/bash
# Deployment testing script

DEPLOYMENT_DIR="$1"
if [ -z "$DEPLOYMENT_DIR" ]; then
    echo "Usage: $0 <deployment_directory>"
    exit 1
fi

echo "Testing deployment in: $DEPLOYMENT_DIR"

# Test application startup
cd "$DEPLOYMENT_DIR"
timeout 10s ./MyApplication --test-mode
if [ $? -eq 0 ]; then
    echo "✓ Application starts successfully"
else
    echo "✗ Application failed to start"
    exit 1
fi

# Test plugin loading
if [ -d plugins ]; then
    echo "✓ Plugins directory exists"
    plugin_count=$(ls plugins/*.qtplugin 2>/dev/null | wc -l)
    echo "✓ Found $plugin_count plugin(s)"
else
    echo "✗ Plugins directory missing"
fi

# Test dependencies
echo "Checking dependencies..."
case "$(uname)" in
    Linux)
        ldd MyApplication | grep "not found" && echo "✗ Missing dependencies" || echo "✓ All dependencies satisfied"
        ;;
    Darwin)
        otool -L MyApplication | grep "not found" && echo "✗ Missing dependencies" || echo "✓ All dependencies satisfied"
        ;;
esac

echo "Deployment test completed"
```

## Next Steps

After deployment:

1. Test on target platforms
2. Monitor application performance
3. Set up update mechanisms
4. Collect user feedback
5. Plan maintenance releases

## See Also

- [Build Guide](../build/README.md) - Building from source
- [Security Guide](../security/README.md) - Security considerations
- [Performance Guide](../performance/README.md) - Performance optimization
- [Troubleshooting](../troubleshooting/README.md) - Common deployment issues
