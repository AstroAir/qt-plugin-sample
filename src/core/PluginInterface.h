// PluginInterface.h - Enhanced base interface with advanced features
#pragma once
#include <QString>
#include <QVersionNumber>
#include <QJsonObject>
#include <QUuid>
#include <QLoggingCategory>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(pluginCore)

class QWidget;
class QSettings;

// **Plugin validation helpers**
// Note: C++20 concepts removed for compatibility

// **Plugin capabilities flags**
enum class PluginCapability : uint32_t {
    None = 0x0000,
    UI = 0x0001,
    Service = 0x0002,
    Filter = 0x0004,
    Network = 0x0008,
    Database = 0x0010,
    FileSystem = 0x0020,
    Scripting = 0x0040,
    HotReload = 0x0080,
    AsyncInit = 0x0100,
    Configuration = 0x0200,
    Logging = 0x0400,
    Security = 0x0800,
    Threading = 0x1000
};
Q_DECLARE_FLAGS(PluginCapabilities, PluginCapability)
Q_DECLARE_OPERATORS_FOR_FLAGS(PluginCapabilities)

// **Plugin status enumeration**
enum class PluginStatus {
    Unknown,
    Discovered,
    Loading,
    Loaded,
    Initializing,
    Running,
    Paused,
    Stopping,
    Stopped,
    Error,
    Unloading
};

// **Plugin security levels**
enum class SecurityLevel {
    Unrestricted,
    Sandbox,
    LimitedAccess,
    ReadOnly,
    Restricted
};

class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // **Core plugin metadata**
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QVersionNumber version() const = 0;
    virtual QString author() const = 0;
    virtual QUuid uuid() const = 0;
    virtual QString category() const { return "General"; }
    virtual QString homepage() const { return {}; }
    virtual QString license() const { return "Unknown"; }
    
    // **Capabilities and requirements**
    virtual PluginCapabilities capabilities() const { return PluginCapability::None; }
    virtual SecurityLevel requiredSecurityLevel() const { return SecurityLevel::Sandbox; }
    virtual QStringList requiredPermissions() const { return {}; }
    
    // **Lifecycle management**
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    virtual bool isInitialized() const = 0;
    virtual PluginStatus status() const { return PluginStatus::Unknown; }
    
    // **Async lifecycle (optional)**
    virtual bool supportsAsyncInit() const { return false; }
    
    // **Configuration support**
    virtual QJsonObject defaultConfiguration() const { return {}; }
    virtual bool configure(const QJsonObject& config) { Q_UNUSED(config); return true; }
    virtual QJsonObject currentConfiguration() const { return {}; }
    virtual bool validateConfiguration(const QJsonObject& config) const { Q_UNUSED(config); return true; }
    
    // **Dependency management**
    virtual QStringList dependencies() const { return {}; }
    virtual QVersionNumber minimumHostVersion() const { return QVersionNumber(1, 0, 0); }
    virtual QStringList conflicts() const { return {}; }
    virtual QStringList optionalDependencies() const { return {}; }
    
    // **Runtime control**
    virtual bool pause() { return false; }
    virtual bool resume() { return false; }
    virtual bool restart() { return false; }
    virtual QJsonObject getMetrics() const { return {}; }
    
    // **Settings persistence**
    virtual void saveSettings(QSettings& settings) const { Q_UNUSED(settings); }
    virtual void loadSettings(const QSettings& settings) { Q_UNUSED(settings); }

    // **Plugin communication**
    virtual QVariant executeCommand(const QString& command, const QVariantMap& params = {}) {
        Q_UNUSED(command);
        Q_UNUSED(params);
        return QVariant();
    }
    virtual QStringList availableCommands() const { return {}; }
    
    // **Error handling**
    virtual QString lastError() const { return {}; }
    virtual QStringList errorLog() const { return {}; }
    virtual void clearErrors() {}
};

Q_DECLARE_INTERFACE(IPlugin, "com.example.IPlugin/2.0")