# Deployment Guide

Comprehensive guide for building, packaging, and deploying QtPlugin-based applications and plugins across different platforms.

## Overview

This guide covers the complete deployment pipeline from development to production, including:

- **Building** - Cross-platform build configurations
- **Packaging** - Creating distributable packages
- **Distribution** - Deployment strategies and channels
- **Maintenance** - Updates and version management

## Cross-Platform Building

### Windows Deployment

#### MSVC Build Configuration

```cmake
# Windows-specific configuration
if(WIN32)
    # Set Windows-specific properties
    set_target_properties(MyPlugin PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
    
    # Windows-specific compile definitions
    target_compile_definitions(MyPlugin PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
    )
    
    # Link Windows-specific libraries
    target_link_libraries(MyPlugin PRIVATE
        kernel32
        user32
        shell32
    )
endif()
```

#### Windows Installer (NSIS)

```nsis
; QtPlugin Application Installer
!define APP_NAME "MyQtPluginApp"
!define APP_VERSION "1.0.0"
!define PUBLISHER "My Company"

Name "${APP_NAME}"
OutFile "${APP_NAME}-${APP_VERSION}-Setup.exe"
InstallDir "$PROGRAMFILES64\${PUBLISHER}\${APP_NAME}"

Section "Main Application"
    SetOutPath "$INSTDIR"
    File "MyQtPluginApp.exe"
    File "*.dll"
    
    SetOutPath "$INSTDIR\plugins"
    File /r "plugins\*"
    
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
                     "UninstallString" "$INSTDIR\uninstall.exe"
    
    WriteUninstaller "$INSTDIR\uninstall.exe"
SectionEnd
```

### macOS Deployment

#### App Bundle Structure

```
MyApp.app/
├── Contents/
│   ├── Info.plist
│   ├── MacOS/
│   │   └── MyApp
│   ├── Resources/
│   │   ├── icon.icns
│   │   └── plugins/
│   │       ├── plugin1.dylib
│   │       └── plugin2.dylib
│   └── Frameworks/
│       ├── QtCore.framework/
│       └── QtPlugin.framework/
```

#### CMake macOS Configuration

```cmake
if(APPLE)
    set_target_properties(MyApp PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "MyApp"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
    
    # Code signing
    if(APPLE_CODESIGN_IDENTITY)
        set_target_properties(MyApp PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "${APPLE_CODESIGN_IDENTITY}"
            XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${APPLE_DEVELOPMENT_TEAM}"
        )
    endif()
endif()
```

#### macOS Deployment Script

```bash
#!/bin/bash
# macOS deployment script

APP_NAME="MyApp"
APP_BUNDLE="${APP_NAME}.app"
DMG_NAME="${APP_NAME}-${VERSION}.dmg"

# Create app bundle
cmake --build build --target install

# Deploy Qt frameworks
macdeployqt "${APP_BUNDLE}" -verbose=2

# Copy plugins
mkdir -p "${APP_BUNDLE}/Contents/Resources/plugins"
cp -r plugins/*.dylib "${APP_BUNDLE}/Contents/Resources/plugins/"

# Fix plugin dependencies
for plugin in "${APP_BUNDLE}/Contents/Resources/plugins"/*.dylib; do
    install_name_tool -change @rpath/QtCore.framework/Versions/6/QtCore \
                      @executable_path/../Frameworks/QtCore.framework/Versions/6/QtCore \
                      "$plugin"
done

# Create DMG
hdiutil create -volname "${APP_NAME}" -srcfolder "${APP_BUNDLE}" -ov -format UDZO "${DMG_NAME}"

# Code sign DMG (if certificate available)
if [ -n "$APPLE_CODESIGN_IDENTITY" ]; then
    codesign --force --verify --verbose --sign "$APPLE_CODESIGN_IDENTITY" "${DMG_NAME}"
fi
```

### Linux Deployment

#### AppImage Creation

```bash
#!/bin/bash
# Create AppImage for Linux distribution

APP_NAME="MyQtPluginApp"
APP_DIR="${APP_NAME}.AppDir"

# Create AppDir structure
mkdir -p "${APP_DIR}/usr/bin"
mkdir -p "${APP_DIR}/usr/lib"
mkdir -p "${APP_DIR}/usr/share/applications"
mkdir -p "${APP_DIR}/usr/share/icons/hicolor/256x256/apps"

# Copy application
cp build/MyQtPluginApp "${APP_DIR}/usr/bin/"

# Copy plugins
mkdir -p "${APP_DIR}/usr/lib/plugins"
cp -r build/plugins/* "${APP_DIR}/usr/lib/plugins/"

# Copy Qt libraries
cp -r /opt/qt6/lib/libQt6Core.so* "${APP_DIR}/usr/lib/"
cp -r /opt/qt6/lib/libQt6Gui.so* "${APP_DIR}/usr/lib/"

# Create desktop file
cat > "${APP_DIR}/usr/share/applications/${APP_NAME}.desktop" << EOF
[Desktop Entry]
Type=Application
Name=My Qt Plugin App
Exec=MyQtPluginApp
Icon=myapp
Categories=Utility;
EOF

# Copy icon
cp resources/icon.png "${APP_DIR}/usr/share/icons/hicolor/256x256/apps/myapp.png"

# Create AppRun script
cat > "${APP_DIR}/AppRun" << 'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${HERE}/usr/lib/plugins:${QT_PLUGIN_PATH}"
exec "${HERE}/usr/bin/MyQtPluginApp" "$@"
EOF

chmod +x "${APP_DIR}/AppRun"

# Create AppImage
appimagetool "${APP_DIR}" "${APP_NAME}-x86_64.AppImage"
```

## Plugin Packaging

### Plugin Metadata Validation

```cpp
// Validate plugin before packaging
class PluginValidator {
public:
    struct ValidationResult {
        bool is_valid = false;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    static ValidationResult validate_plugin(const std::filesystem::path& plugin_path) {
        ValidationResult result;
        
        // Check file exists and is readable
        if (!std::filesystem::exists(plugin_path)) {
            result.errors.push_back("Plugin file does not exist");
            return result;
        }
        
        // Load and validate plugin
        qtplugin::PluginLoader loader;
        auto load_result = loader.load(plugin_path);
        if (!load_result) {
            result.errors.push_back("Failed to load plugin: " + load_result.error().message);
            return result;
        }
        
        auto plugin = load_result.value();
        
        // Validate metadata
        if (plugin->name().empty()) {
            result.errors.push_back("Plugin name is empty");
        }
        
        if (plugin->id().empty()) {
            result.errors.push_back("Plugin ID is empty");
        }
        
        if (plugin->version().major() == 0 && plugin->version().minor() == 0) {
            result.warnings.push_back("Plugin version is 0.0.x");
        }
        
        // Test initialization
        auto init_result = plugin->initialize();
        if (!init_result) {
            result.errors.push_back("Plugin initialization failed: " + init_result.error().message);
        } else {
            plugin->shutdown();
        }
        
        result.is_valid = result.errors.empty();
        return result;
    }
};
```

### Plugin Signing

```bash
#!/bin/bash
# Sign plugin for distribution

PLUGIN_FILE="$1"
PRIVATE_KEY="signing_key.pem"
CERTIFICATE="certificate.pem"

if [ ! -f "$PLUGIN_FILE" ]; then
    echo "Plugin file not found: $PLUGIN_FILE"
    exit 1
fi

# Create signature
openssl dgst -sha256 -sign "$PRIVATE_KEY" -out "${PLUGIN_FILE}.sig" "$PLUGIN_FILE"

# Create certificate bundle
cat "$CERTIFICATE" > "${PLUGIN_FILE}.cert"

echo "Plugin signed: $PLUGIN_FILE"
echo "Signature: ${PLUGIN_FILE}.sig"
echo "Certificate: ${PLUGIN_FILE}.cert"
```

## Distribution Strategies

### Plugin Repository

```json
{
    "repository": {
        "name": "Official QtPlugin Repository",
        "version": "1.0",
        "url": "https://plugins.example.com/api/v1"
    },
    "plugins": [
        {
            "id": "com.example.textprocessor",
            "name": "Text Processor",
            "version": "1.2.0",
            "description": "Advanced text processing plugin",
            "author": "Example Corp",
            "license": "MIT",
            "category": "Text Processing",
            "download_url": "https://plugins.example.com/downloads/textprocessor-1.2.0.qtplugin",
            "signature_url": "https://plugins.example.com/downloads/textprocessor-1.2.0.qtplugin.sig",
            "checksum": "sha256:abc123...",
            "size": 1048576,
            "dependencies": [],
            "compatibility": {
                "qtplugin_version": ">=3.0.0",
                "qt_version": ">=6.2.0",
                "platforms": ["windows", "macos", "linux"]
            }
        }
    ]
}
```

### Automatic Updates

```cpp
class PluginUpdater {
public:
    struct UpdateInfo {
        std::string plugin_id;
        qtplugin::Version current_version;
        qtplugin::Version available_version;
        std::string download_url;
        std::string changelog;
    };
    
    std::vector<UpdateInfo> check_for_updates(const std::vector<std::string>& plugin_ids) {
        std::vector<UpdateInfo> updates;
        
        for (const auto& id : plugin_ids) {
            auto plugin = m_plugin_manager->get_plugin(id);
            if (!plugin) continue;
            
            auto available_version = fetch_latest_version(id);
            if (available_version > plugin->version()) {
                UpdateInfo info;
                info.plugin_id = id;
                info.current_version = plugin->version();
                info.available_version = available_version;
                info.download_url = get_download_url(id, available_version);
                info.changelog = fetch_changelog(id, available_version);
                updates.push_back(info);
            }
        }
        
        return updates;
    }
    
    qtplugin::expected<void, std::string> update_plugin(const UpdateInfo& info) {
        // Download new version
        auto download_result = download_plugin(info.download_url);
        if (!download_result) {
            return qtplugin::make_error<void>("Download failed: " + download_result.error());
        }
        
        // Verify signature
        if (!verify_plugin_signature(download_result.value())) {
            return qtplugin::make_error<void>("Signature verification failed");
        }
        
        // Backup current version
        backup_plugin(info.plugin_id);
        
        // Install new version
        auto install_result = install_plugin(download_result.value());
        if (!install_result) {
            restore_plugin_backup(info.plugin_id);
            return qtplugin::make_error<void>("Installation failed: " + install_result.error());
        }
        
        return qtplugin::make_success();
    }

private:
    qtplugin::PluginManager* m_plugin_manager;
};
```

## Deployment Automation

### CI/CD Pipeline

```yaml
# .github/workflows/deploy.yml
name: Build and Deploy

on:
  release:
    types: [published]

jobs:
  build-windows:
    runs-on: windows-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Qt
      uses: jurplel/install-qt-action@v3
      with:
        version: 6.5.0
        
    - name: Build
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build --config Release
        
    - name: Package
      run: |
        cpack --config build/CPackConfig.cmake
        
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: windows-installer
        path: build/*.exe

  build-macos:
    runs-on: macos-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Setup Qt
      run: brew install qt6
      
    - name: Build and Package
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build
        ./scripts/create-dmg.sh
        
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: macos-dmg
        path: "*.dmg"

  build-linux:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Install Dependencies
      run: |
        sudo apt-get update
        sudo apt-get install qt6-base-dev
        
    - name: Build and Package
      run: |
        cmake -B build -DCMAKE_BUILD_TYPE=Release
        cmake --build build
        ./scripts/create-appimage.sh
        
    - name: Upload Artifacts
      uses: actions/upload-artifact@v3
      with:
        name: linux-appimage
        path: "*.AppImage"

  deploy:
    needs: [build-windows, build-macos, build-linux]
    runs-on: ubuntu-latest
    steps:
    - name: Download Artifacts
      uses: actions/download-artifact@v3
      
    - name: Create Release
      uses: softprops/action-gh-release@v1
      with:
        files: |
          windows-installer/*
          macos-dmg/*
          linux-appimage/*
```

## Version Management

### Semantic Versioning

```cpp
// Version management for plugins
class VersionManager {
public:
    static bool is_compatible(const qtplugin::Version& plugin_version,
                             const qtplugin::Version& required_version) {
        // Major version must match
        if (plugin_version.major() != required_version.major()) {
            return false;
        }
        
        // Minor version must be >= required
        if (plugin_version.minor() < required_version.minor()) {
            return false;
        }
        
        // Patch version can be anything if minor >= required
        return true;
    }
    
    static std::string format_version_range(const qtplugin::Version& min_version,
                                          const qtplugin::Version& max_version) {
        return std::format(">={}.<{}", min_version.to_string(), max_version.to_string());
    }
};
```

## Best Practices

### 1. **Reproducible Builds**

- Use fixed dependency versions
- Document build environment
- Use containerized builds

### 2. **Security**

- Sign all binaries and plugins
- Verify signatures during installation
- Use secure distribution channels

### 3. **Testing**

- Test on all target platforms
- Validate plugin compatibility
- Perform integration testing

### 4. **Documentation**

- Include installation instructions
- Document system requirements
- Provide troubleshooting guides

## See Also

- **[User Guide](../user-guide/index.md)** - Application integration
- **[Developer Guide](../developer-guide/index.md)** - Plugin development
- **[Security Guide](../user-guide/security.md)** - Security considerations
- **[Performance Guide](../user-guide/performance.md)** - Performance optimization

---

**Next**: [Building Guide](building.md) for detailed build instructions.
