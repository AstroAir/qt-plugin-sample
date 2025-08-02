// RibbonMainWindow.h - Main Window with Modern Ribbon Interface Integration
#pragma once

#include "RibbonInterface.h"
#include "../core/PluginManager.h"
#include "../managers/ApplicationManager.h"
#include "../managers/ThemeManager.h"
#include "../utils/PluginTemplateGenerator.h"
#include "../utils/PluginValidator.h"
#include "../utils/PluginDebugger.h"
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QGroupBox>
#include <QTimer>
#include <QSettings>
#include <QCloseEvent>
#include <QShowEvent>
#include <QResizeEvent>
#include <QJsonObject>
#include <QStringList>
#include <QMap>
#include <memory>

// Forward declarations
class PluginDashboard;
class PluginExplorer;
class PluginEditor;
class PluginConsole;
class PluginProperties;
class RibbonStatusBar;

// Main window with ribbon interface
class RibbonMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit RibbonMainWindow(QWidget* parent = nullptr);
    ~RibbonMainWindow() override;

    // Ribbon access
    RibbonBar* ribbonBar() const;
    
    // Plugin management integration
    void setPluginManager(PluginManager* manager);
    PluginManager* pluginManager() const;
    
    // Application manager integration
    void setApplicationManager(ApplicationManager* manager);
    ApplicationManager* applicationManager() const;
    
    // Central widget management
    void setCentralWidget(QWidget* widget);
    QWidget* centralWidget() const;
    
    // Dock widgets
    void addPluginDockWidget(Qt::DockWidgetArea area, QDockWidget* dockWidget);
    void removePluginDockWidget(QDockWidget* dockWidget);
    QList<QDockWidget*> pluginDockWidgets() const;
    
    // Status bar
    RibbonStatusBar* ribbonStatusBar() const;
    void showStatusMessage(const QString& message, int timeout = 0);
    void setStatusProgress(int value, int maximum = 100);
    void hideStatusProgress();
    
    // Window state
    void saveWindowState();
    void restoreWindowState();
    QJsonObject exportWindowState() const;
    void importWindowState(const QJsonObject& state);
    
    // Theme integration
    void applyRibbonTheme(RibbonTheme theme);
    RibbonTheme currentRibbonTheme() const;

public slots:
    // File operations
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void closeProject();
    void exit();
    
    // Plugin operations
    void createNewPlugin();
    void openPlugin();
    void savePlugin();
    void buildPlugin();
    void debugPlugin();
    void validatePlugin();
    void publishPlugin();
    
    // View operations
    void showPluginDashboard();
    void showPluginExplorer();
    void showPluginEditor();
    void showPluginConsole();
    void showPluginProperties();
    void toggleRibbonMinimized();
    void customizeRibbon();
    
    // Tools
    void showPluginTemplateWizard();
    void showPluginValidator();
    void showPluginDebugger();
    void showThemeManager();
    void showPreferences();
    void showAbout();
    
    // Plugin events
    void onPluginLoaded(const QString& pluginId);
    void onPluginUnloaded(const QString& pluginId);
    void onPluginError(const QString& pluginId, const QString& error);

signals:
    void ribbonThemeChanged(RibbonTheme theme);
    void windowStateChanged();
    void pluginActionRequested(const QString& action, const QString& pluginId);

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;

private slots:
    void onRibbonThemeChanged(RibbonTheme theme);
    void onPluginManagerStateChanged();
    void onApplicationManagerStateChanged();
    void updateRecentFiles();
    void updateRecentProjects();
    void updatePluginActions();
    void updateWindowTitle();

private:
    struct RibbonMainWindowPrivate;
    std::unique_ptr<RibbonMainWindowPrivate> d;
    
    void setupUI();
    void setupRibbon();
    void setupCentralWidget();
    void setupDockWidgets();
    void setupStatusBar();
    void setupConnections();
    void createRibbonTabs();
    void createFileTab();
    void createPluginTab();
    void createViewTab();
    void createToolsTab();
    void createHelpTab();
    void updateRibbonState();
    void loadSettings();
    void saveSettings();
};

// Plugin dashboard widget
class PluginDashboard : public QWidget {
    Q_OBJECT

public:
    explicit PluginDashboard(QWidget* parent = nullptr);
    ~PluginDashboard() override;

    // Dashboard management
    void setPluginManager(PluginManager* manager);
    void refreshDashboard();
    void addDashboardWidget(const QString& title, QWidget* widget);
    void removeDashboardWidget(const QString& title);

signals:
    void pluginSelected(const QString& pluginId);
    void actionRequested(const QString& action);

private:
    struct DashboardPrivate;
    std::unique_ptr<DashboardPrivate> d;
    
    void setupUI();
    void updateStatistics();
    void createOverviewWidget();
    void createRecentPluginsWidget();
    void createQuickActionsWidget();
};

// Plugin explorer widget
class PluginExplorer : public QWidget {
    Q_OBJECT

public:
    explicit PluginExplorer(QWidget* parent = nullptr);
    ~PluginExplorer() override;

    // Explorer management
    void setPluginManager(PluginManager* manager);
    void refreshExplorer();
    void setCurrentPlugin(const QString& pluginId);
    QString currentPlugin() const;

signals:
    void pluginSelected(const QString& pluginId);
    void pluginDoubleClicked(const QString& pluginId);
    void contextMenuRequested(const QString& pluginId, const QPoint& position);

private:
    struct ExplorerPrivate;
    std::unique_ptr<ExplorerPrivate> d;
    
    void setupUI();
    void populatePluginTree();
    void updatePluginInfo();
};

// Plugin editor widget
class PluginEditor : public QWidget {
    Q_OBJECT

public:
    explicit PluginEditor(QWidget* parent = nullptr);
    ~PluginEditor() override;

    // Editor management
    void openPlugin(const QString& pluginId);
    void closePlugin(const QString& pluginId);
    void savePlugin(const QString& pluginId);
    void saveAllPlugins();
    
    // Current plugin
    QString currentPlugin() const;
    void setCurrentPlugin(const QString& pluginId);
    
    // Editor features
    void setValidationEnabled(bool enabled);
    bool isValidationEnabled() const;
    void setDebuggingEnabled(bool enabled);
    bool isDebuggingEnabled() const;

signals:
    void pluginModified(const QString& pluginId);
    void pluginSaved(const QString& pluginId);
    void validationRequested(const QString& pluginId);
    void debuggingRequested(const QString& pluginId);

private:
    struct EditorPrivate;
    std::unique_ptr<EditorPrivate> d;
    
    void setupUI();
    void createEditorTabs();
    void updateEditorState();
};

// Plugin console widget
class PluginConsole : public QWidget {
    Q_OBJECT

public:
    explicit PluginConsole(QWidget* parent = nullptr);
    ~PluginConsole() override;

    // Console management
    void appendMessage(const QString& message, const QString& category = "Info");
    void appendError(const QString& error);
    void appendWarning(const QString& warning);
    void appendDebug(const QString& debug);
    void clear();
    
    // Filtering
    void setFilterEnabled(bool enabled);
    bool isFilterEnabled() const;
    void setVisibleCategories(const QStringList& categories);
    QStringList visibleCategories() const;

signals:
    void commandEntered(const QString& command);
    void messageDoubleClicked(const QString& message);

private:
    struct ConsolePrivate;
    std::unique_ptr<ConsolePrivate> d;
    
    void setupUI();
    void updateFilter();
};

// Plugin properties widget
class PluginProperties : public QWidget {
    Q_OBJECT

public:
    explicit PluginProperties(QWidget* parent = nullptr);
    ~PluginProperties() override;

    // Properties management
    void setCurrentPlugin(const QString& pluginId);
    QString currentPlugin() const;
    void refreshProperties();
    
    // Property editing
    void setReadOnly(bool readOnly);
    bool isReadOnly() const;

signals:
    void propertyChanged(const QString& pluginId, const QString& property, const QVariant& value);
    void propertiesModified(const QString& pluginId);

private:
    struct PropertiesPrivate;
    std::unique_ptr<PropertiesPrivate> d;
    
    void setupUI();
    void updateProperties();
    void createPropertyEditor();
};

// Enhanced status bar for ribbon interface
class RibbonStatusBar : public QStatusBar {
    Q_OBJECT

public:
    explicit RibbonStatusBar(QWidget* parent = nullptr);
    ~RibbonStatusBar() override;

    // Progress indication
    void setProgress(int value, int maximum = 100);
    void hideProgress();
    bool isProgressVisible() const;
    
    // Status indicators
    void setPluginCount(int count);
    void setActivePluginCount(int count);
    void setThemeIndicator(RibbonTheme theme);
    void setMemoryUsage(qint64 bytes);
    
    // Custom widgets
    void addPermanentWidget(QWidget* widget, int stretch = 0);
    void removePermanentWidget(QWidget* widget);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct StatusBarPrivate;
    std::unique_ptr<StatusBarPrivate> d;
    
    void setupUI();
    void updateIndicators();
};

// Utility functions for ribbon integration
namespace RibbonIntegration {
    // Ribbon setup helpers
    void setupPluginManagementRibbon(RibbonBar* ribbon, PluginManager* pluginManager);
    void setupDevelopmentToolsRibbon(RibbonBar* ribbon);
    void setupViewRibbon(RibbonBar* ribbon, QMainWindow* mainWindow);
    void setupHelpRibbon(RibbonBar* ribbon);
    
    // Action helpers
    QAction* createRibbonAction(const QString& text, const QString& icon, const QString& tooltip, QObject* parent);
    void connectPluginAction(QAction* action, PluginManager* pluginManager, const QString& pluginId, const QString& actionName);
    
    // Theme helpers
    void applyRibbonThemeToApplication(RibbonTheme theme);
    void syncRibbonThemeWithSystem(RibbonBar* ribbon);
    
    // Layout helpers
    QWidget* createRibbonSpacer(int width = -1);
    QWidget* createRibbonSeparator(Qt::Orientation orientation = Qt::Vertical);
    
    // Icon helpers
    QIcon createRibbonIcon(const QString& iconName, RibbonTheme theme = RibbonTheme::Light);
    QIcon createThemedIcon(const QString& lightIcon, const QString& darkIcon, RibbonTheme theme);
}
