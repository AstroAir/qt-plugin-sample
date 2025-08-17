// MainWindow.h - Main application window
#pragma once

#include <QMainWindow>
#include <QSplitter>
#include <QTabWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QActionGroup>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <QQuickWidget>
#include <memory>

namespace qtplugin { class PluginManager; }
class PluginRegistry;
class ThemeManager;
class PluginListWidget;
class PluginDetailsWidget;
class DashboardWidget;
class PluginStoreWidget;
class LogViewer;
class PerformanceMonitorWidget;
class SecurityManager;
class PluginStoreWidget;
class ConsoleWidget;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QToolButton;
class QComboBox;
class QLineEdit;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(bool pluginManagerVisible READ isPluginManagerVisible WRITE setPluginManagerVisible NOTIFY pluginManagerVisibilityChanged)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // **Properties**
    QString currentTheme() const;
    void setCurrentTheme(const QString& theme);
    
    bool isPluginManagerVisible() const;
    void setPluginManagerVisible(bool visible);
    
    // **Plugin management**
    void setPluginPath(const QString& path);
    void loadPluginsFromPath(const QString& path);
    void refreshPluginList();
    
    // **UI management**
    void showPluginStore();
    void showSecuritySettings();
    void showPerformanceMonitor();
    void showAboutDialog();

public slots:
    void onPluginLoaded(const QString& pluginName);
    void onPluginUnloaded(const QString& pluginName);
    void onPluginError(const QString& pluginName, const QString& error);
    void onThemeChanged(const QString& theme);
    void onSettingsChanged();

signals:
    void currentThemeChanged(const QString& theme);
    void pluginManagerVisibilityChanged(bool visible);
    void pluginPathChanged(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;

private slots:
    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void exportConfiguration();
    void importConfiguration();
    void showPreferences();
    void toggleFullScreen();
    void showStatusMessage(const QString& message, int timeout = 0);
    void updateStatusBar();
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onPluginSelectionChanged();
    void onSearchTextChanged(const QString& text);
    void onFilterChanged();
    void checkForUpdates();
    void installPluginFromFile();
    void uninstallSelectedPlugin();
    void enableSelectedPlugin();
    void disableSelectedPlugin();
    void configureSelectedPlugin();
    void showPluginDetails();
    void showPluginConsole();
    void clearLogs();
    void exportLogs();
    void toggleDarkMode();
    void resetLayout();
    void showWelcomeScreen();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWidgets();
    void createCentralWidget();
    void createSystemTrayActions();
    void setupConnections();
    void setupShortcuts();
    void loadSettings();
    void saveSettings();
    void updateRecentFiles();
    void updateWindowTitle();
    void applyTheme(const QString& theme);
    void setupPluginIntegration();
    void initializeComponents();
    void setupPerformanceMonitoring();
    
    // **UI Creation Methods**
    QWidget* createPluginManagerWidget();
    QWidget* createDashboardWidget();
    QWidget* createWelcomeWidget();
    QDockWidget* createPluginListDock();
    QDockWidget* createPluginDetailsDock();
    QDockWidget* createLogViewerDock();
    QDockWidget* createPerformanceMonitorDock();
    QDockWidget* createConsoleDock();
    QDockWidget* createToolboxDock();
    
    // **Core Components**
    std::unique_ptr<qtplugin::PluginManager> m_pluginManager;
    std::unique_ptr<PluginRegistry> m_pluginRegistry;
    std::unique_ptr<ThemeManager> m_themeManager;
    std::unique_ptr<SecurityManager> m_securityManager;
    
    // **Central Widget**
    QTabWidget* m_centralTabs;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    // **Plugin Management Widgets**
    PluginListWidget* m_pluginListWidget;
    PluginDetailsWidget* m_pluginDetailsWidget;
    PluginStoreWidget* m_pluginStoreWidget;
    
    // **Monitoring Widgets**
    LogViewer* m_logViewer;
    PerformanceMonitorWidget* m_performanceMonitor;
    ConsoleWidget* m_consoleWidget;
    
    // **QML Integration**
    QQuickWidget* m_qmlWidget;
    
    // **Dock Widgets**
    QDockWidget* m_pluginListDock;
    QDockWidget* m_pluginDetailsDock;
    QDockWidget* m_logViewerDock;
    QDockWidget* m_performanceMonitorDock;
    QDockWidget* m_consoleDock;
    QDockWidget* m_toolboxDock;
    
    // **Menu and Toolbar**
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_pluginMenu;
    QMenu* m_toolsMenu;
    QMenu* m_helpMenu;
    QMenu* m_recentFilesMenu;
    QMenu* m_themeMenu;
    
    QToolBar* m_mainToolBar;
    QToolBar* m_pluginToolBar;
    QToolBar* m_viewToolBar;
    
    // **Actions**
    QAction* m_newAction;
    QAction* m_openAction;
    QAction* m_saveAction;
    QAction* m_saveAsAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
    QAction* m_aboutQtAction;
    QAction* m_preferencesAction;
    QAction* m_fullScreenAction;
    
    // **Plugin Actions**
    QAction* m_refreshPluginsAction;
    QAction* m_installPluginAction;
    QAction* m_uninstallPluginAction;
    QAction* m_enablePluginAction;
    QAction* m_disablePluginAction;
    QAction* m_configurePluginAction;
    QAction* m_pluginStoreAction;
    QAction* m_securitySettingsAction;
    
    // **View Actions**
    QActionGroup* m_themeActionGroup;
    QAction* m_darkThemeAction;
    QAction* m_lightThemeAction;
    QAction* m_autoThemeAction;
    QAction* m_resetLayoutAction;
    
    // **System Tray**
    QSystemTrayIcon* m_systemTrayIcon;
    QMenu* m_trayIconMenu;
    
    // **Status Bar Widgets**
    QLabel* m_statusLabel;
    QLabel* m_pluginCountLabel;
    QLabel* m_memoryUsageLabel;
    QProgressBar* m_progressBar;
    QToolButton* m_settingsButton;
    
    // **Search and Filter**
    QLineEdit* m_searchLineEdit;
    QComboBox* m_filterComboBox;
    QComboBox* m_categoryComboBox;
    
    // **Settings and State**
    std::unique_ptr<QSettings> m_settings;
    QStringList m_recentFiles;
    QString m_currentProjectFile;
    QString m_currentTheme;
    bool m_pluginManagerVisible;
    
    // **Monitoring and Updates**
    QTimer* m_statusUpdateTimer;
    QTimer* m_performanceTimer;
    QFileSystemWatcher* m_configWatcher;
    QNetworkAccessManager* m_networkManager;

    // **Enhanced Widgets**
    DashboardWidget* m_dashboardWidget;

    // **Constants**
    static constexpr int MaxRecentFiles = 10;
    static constexpr int StatusBarTimeout = 5000;
    static constexpr int PerformanceUpdateInterval = 1000;
};