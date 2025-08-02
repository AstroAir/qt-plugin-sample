// AdvancedInterfaces.h - Extended plugin interfaces
#pragma once
#include "PluginInterface.h"
#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QAbstractItemModel>
#include <QNetworkAccessManager>
#include <QThread>
#include <QJSEngine>

// **Enhanced UI Plugin Interface**
class IUIPlugin : public IPlugin {
public:
    virtual ~IUIPlugin() = default;
    
    // **Widget creation**
    virtual std::unique_ptr<QWidget> createWidget(QWidget* parent = nullptr) = 0;
    virtual std::unique_ptr<QDockWidget> createDockWidget(QWidget* parent = nullptr) { Q_UNUSED(parent); return nullptr; }
    virtual QWidget* createConfigurationWidget(QWidget* parent = nullptr) { Q_UNUSED(parent); return nullptr; }
    
    // **UI integration**
    virtual QList<QAction*> menuActions() const { return {}; }
    virtual QList<QAction*> toolbarActions() const { return {}; }
    virtual QList<QAction*> contextMenuActions() const { return {}; }
    virtual std::unique_ptr<QToolBar> createToolBar(QWidget* parent = nullptr) { Q_UNUSED(parent); return nullptr; }
    virtual QMenu* createMenu(QWidget* parent = nullptr) { Q_UNUSED(parent); return nullptr; }
    
    // **UI setup and theming**
    virtual void setupUI(QWidget* mainWindow) { Q_UNUSED(mainWindow); }
    virtual void applyTheme(const QString& theme) { Q_UNUSED(theme); }
    virtual QStringList supportedThemes() const { return {"default"}; }
    
    // **Layout management**
    virtual QString preferredDockArea() const { return "center"; }
    virtual QSize minimumSize() const { return QSize(200, 150); }
    virtual QSize preferredSize() const { return QSize(400, 300); }
    
    // **Keyboard shortcuts**
    virtual QList<QKeySequence> keyboardShortcuts() const { return {}; }
    virtual void registerShortcuts(QWidget* parent) { Q_UNUSED(parent); }
};

Q_DECLARE_INTERFACE(IUIPlugin, "com.example.IUIPlugin/2.0")

// **Enhanced Service Plugin Interface**
class IServicePlugin : public IPlugin {
public:
    virtual ~IServicePlugin() = default;
    
    // **Service lifecycle**
    virtual bool startService() = 0;
    virtual bool stopService() = 0;
    virtual bool restartService() { stopService(); return startService(); }
    virtual bool isServiceRunning() const = 0;
    virtual QJsonObject serviceStatus() const = 0;
    
    // **Service configuration**
    virtual bool configureService(const QJsonObject& config) { Q_UNUSED(config); return true; }
    virtual QJsonObject serviceConfiguration() const { return {}; }
    
    // **Service monitoring**
    virtual qint64 uptime() const { return 0; }
    virtual QJsonObject performanceMetrics() const { return {}; }
    virtual QStringList serviceLog() const { return {}; }
    
    // **Service dependencies**
    virtual QStringList requiredServices() const { return {}; }
    virtual bool checkServiceHealth() const { return true; }
    
    // **Threading support**
    virtual bool runsInSeparateThread() const { return false; }
    virtual QThread* serviceThread() { return nullptr; }
};

Q_DECLARE_INTERFACE(IServicePlugin, "com.example.IServicePlugin/2.0")

// **Network Plugin Interface**
class INetworkPlugin : public IPlugin {
public:
    virtual ~INetworkPlugin() = default;
    
    virtual QNetworkAccessManager* networkManager() { return nullptr; }
    virtual QStringList supportedProtocols() const = 0;
    virtual bool handleRequest(const QNetworkRequest& request) { Q_UNUSED(request); return false; }
    virtual QJsonObject networkStatus() const { return {}; }
    virtual void setProxy(const QNetworkProxy& proxy) { Q_UNUSED(proxy); }
};

Q_DECLARE_INTERFACE(INetworkPlugin, "com.example.INetworkPlugin/1.0")

// **Scripting Plugin Interface**
class IScriptingPlugin : public IPlugin {
public:
    virtual ~IScriptingPlugin() = default;
    
    virtual QJSEngine* scriptEngine() { return nullptr; }
    virtual QStringList supportedLanguages() const = 0;
    virtual bool executeScript(const QString& script, const QString& language = "javascript") = 0;
    virtual QVariant evaluateExpression(const QString& expression) { Q_UNUSED(expression); return QVariant(); }
    virtual void registerObject(const QString& name, QObject* object) { Q_UNUSED(name); Q_UNUSED(object); }
    virtual QStringList availableObjects() const { return {}; }
};

Q_DECLARE_INTERFACE(IScriptingPlugin, "com.example.IScriptingPlugin/1.0")

// **Data Provider Interface**
class IDataProviderPlugin : public IPlugin {
public:
    virtual ~IDataProviderPlugin() = default;
    
    virtual QAbstractItemModel* createModel() = 0;
    virtual QStringList supportedDataTypes() const = 0;
    virtual bool canHandleData(const QMimeData* data) const = 0;
    virtual QVariant processData(const QVariant& input) = 0;
    virtual bool exportData(const QVariant& data, const QString& format, const QString& destination) = 0;
};

Q_DECLARE_INTERFACE(IDataProviderPlugin, "com.example.IDataProviderPlugin/1.0")