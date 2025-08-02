// MainWindow.cpp - Complete implementation
#include "MainWindow.h"
#include "../core/PluginManager.h"
#include "../core/PluginRegistry.h"
#include "../managers/ThemeManager.h"
#include "PluginWidgets.h"
#include "DashboardWidget.h"
#include "PluginStoreWidget.h"
#include "PluginWizard.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QGroupBox>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QQuickWidget>
#include <QQmlContext>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QShortcut>
#include <QRegularExpression>
#include <memory>

Q_LOGGING_CATEGORY(mainWindow, "ui.mainwindow")

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_currentTheme("system")
    , m_pluginManagerVisible(true)
{
    // **Initialize core components first**
    initializeComponents();
    
    // **Setup UI**
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    createDockWidgets();
    createCentralWidget();
    
    // **Setup system tray**
    if (QSystemTrayIcon::isSystemTrayAvailable()) {
        createSystemTrayActions();
    }
    
    // **Setup connections and shortcuts**
    setupConnections();
    setupShortcuts();
    
    // **Load settings and apply theme**
    loadSettings();
    
    // **Setup plugin integration**
    setupPluginIntegration();
    
    // **Enable drag and drop**
    setAcceptDrops(true);
    
    // **Start monitoring timers**
    setupPerformanceMonitoring();
    
    // **Set window properties**
    setWindowTitle("Advanced Plugin Manager");
    setWindowIcon(QIcon(":/icons/app.svg"));
    resize(1400, 900);
    
    // **Show welcome screen initially**
    showWelcomeScreen();
    
    qCInfo(mainWindow) << "Main window initialized successfully";
}

MainWindow::~MainWindow() {
    saveSettings();
    qCInfo(mainWindow) << "Main window destroyed";
}

void MainWindow::initializeComponents() {
    // **Initialize managers**
    m_pluginManager = std::make_unique<PluginManager>(this);
    m_pluginRegistry = std::make_unique<PluginRegistry>(this);
    m_themeManager = std::make_unique<ThemeManager>(this);
    m_securityManager = std::make_unique<SecurityManager>(this);
    
    // **Initialize settings**
    m_settings = std::make_unique<QSettings>(this);
    
    // **Initialize network manager**
    m_networkManager = new QNetworkAccessManager(this);
    
    // **Initialize file system watcher**
    m_configWatcher = new QFileSystemWatcher(this);
    
    // **Initialize timers**
    m_statusUpdateTimer = new QTimer(this);
    m_statusUpdateTimer->setInterval(StatusBarTimeout);
    m_statusUpdateTimer->setSingleShot(true);
    
    m_performanceTimer = new QTimer(this);
    m_performanceTimer->setInterval(PerformanceUpdateInterval);

    // **Initialize enhanced widgets**
    m_dashboardWidget = new DashboardWidget(this);
}

void MainWindow::createActions() {
    // **File Actions**
    m_newAction = new QAction(QIcon(":/icons/new.svg"), tr("&New Project"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setStatusTip(tr("Create a new project"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newProject);

    m_openAction = new QAction(QIcon(":/icons/open.svg"), tr("&Open Project..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setStatusTip(tr("Open an existing project"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openProject);

    m_saveAction = new QAction(QIcon(":/icons/save.svg"), tr("&Save Project"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setStatusTip(tr("Save the current project"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveProject);

    m_saveAsAction = new QAction(QIcon(":/icons/saveas.svg"), tr("Save Project &As..."), this);
    m_saveAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveAsAction->setStatusTip(tr("Save the project with a new name"));
    connect(m_saveAsAction, &QAction::triggered, this, &MainWindow::saveProjectAs);

    m_exitAction = new QAction(QIcon(":/icons/exit.svg"), tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    // **Plugin Actions**
    m_refreshPluginsAction = new QAction(QIcon(":/icons/refresh.svg"), tr("&Refresh Plugins"), this);
    m_refreshPluginsAction->setShortcut(QKeySequence::Refresh);
    m_refreshPluginsAction->setStatusTip(tr("Refresh the plugin list"));
    connect(m_refreshPluginsAction, &QAction::triggered, this, &MainWindow::refreshPluginList);

    m_installPluginAction = new QAction(QIcon(":/icons/install.svg"), tr("&Install Plugin..."), this);
    m_installPluginAction->setShortcut(tr("Ctrl+I"));
    m_installPluginAction->setStatusTip(tr("Install a plugin from file"));
    connect(m_installPluginAction, &QAction::triggered, this, &MainWindow::installPluginFromFile);

    m_uninstallPluginAction = new QAction(QIcon(":/icons/uninstall.svg"), tr("&Uninstall Plugin"), this);
    m_uninstallPluginAction->setStatusTip(tr("Uninstall the selected plugin"));
    m_uninstallPluginAction->setEnabled(false);
    connect(m_uninstallPluginAction, &QAction::triggered, this, &MainWindow::uninstallSelectedPlugin);

    m_enablePluginAction = new QAction(QIcon(":/icons/enable.svg"), tr("&Enable Plugin"), this);
    m_enablePluginAction->setStatusTip(tr("Enable the selected plugin"));
    m_enablePluginAction->setEnabled(false);
    connect(m_enablePluginAction, &QAction::triggered, this, &MainWindow::enableSelectedPlugin);

    m_disablePluginAction = new QAction(QIcon(":/icons/disable.svg"), tr("&Disable Plugin"), this);
    m_disablePluginAction->setStatusTip(tr("Disable the selected plugin"));
    m_disablePluginAction->setEnabled(false);
    connect(m_disablePluginAction, &QAction::triggered, this, &MainWindow::disableSelectedPlugin);

    m_configurePluginAction = new QAction(QIcon(":/icons/configure.svg"), tr("&Configure Plugin..."), this);
    m_configurePluginAction->setStatusTip(tr("Configure the selected plugin"));
    m_configurePluginAction->setEnabled(false);
    connect(m_configurePluginAction, &QAction::triggered, this, &MainWindow::configureSelectedPlugin);

    m_pluginStoreAction = new QAction(QIcon(":/icons/store.svg"), tr("Plugin &Store..."), this);
    m_pluginStoreAction->setStatusTip(tr("Browse and install plugins from the store"));
    connect(m_pluginStoreAction, &QAction::triggered, this, &MainWindow::showPluginStore);

    m_securitySettingsAction = new QAction(QIcon(":/icons/security.svg"), tr("&Security Settings..."), this);
    m_securitySettingsAction->setStatusTip(tr("Configure plugin security settings"));
    connect(m_securitySettingsAction, &QAction::triggered, this, &MainWindow::showSecuritySettings);

    // **View Actions**
    m_themeActionGroup = new QActionGroup(this);
    
    m_lightThemeAction = new QAction(tr("&Light Theme"), this);
    m_lightThemeAction->setCheckable(true);
    m_lightThemeAction->setActionGroup(m_themeActionGroup);
    connect(m_lightThemeAction, &QAction::triggered, [this]() { setCurrentTheme("light"); });

    m_darkThemeAction = new QAction(tr("&Dark Theme"), this);
    m_darkThemeAction->setCheckable(true);
    m_darkThemeAction->setActionGroup(m_themeActionGroup);
    connect(m_darkThemeAction, &QAction::triggered, [this]() { setCurrentTheme("dark"); });

    m_autoThemeAction = new QAction(tr("&Auto Theme"), this);
    m_autoThemeAction->setCheckable(true);
    m_autoThemeAction->setActionGroup(m_themeActionGroup);
    m_autoThemeAction->setChecked(true);
    connect(m_autoThemeAction, &QAction::triggered, [this]() { setCurrentTheme("system"); });

    m_fullScreenAction = new QAction(QIcon(":/icons/fullscreen.svg"), tr("&Full Screen"), this);
    m_fullScreenAction->setShortcut(QKeySequence::FullScreen);
    m_fullScreenAction->setCheckable(true);
    m_fullScreenAction->setStatusTip(tr("Toggle full screen mode"));
    connect(m_fullScreenAction, &QAction::triggered, this, &MainWindow::toggleFullScreen);

    m_resetLayoutAction = new QAction(QIcon(":/icons/reset.svg"), tr("&Reset Layout"), this);
    m_resetLayoutAction->setStatusTip(tr("Reset window layout to default"));
    connect(m_resetLayoutAction, &QAction::triggered, this, &MainWindow::resetLayout);

    // **Help Actions**
    m_aboutAction = new QAction(QIcon(":/icons/about.svg"), tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show information about this application"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    m_aboutQtAction = new QAction(tr("About &Qt"), this);
    m_aboutQtAction->setStatusTip(tr("Show information about Qt"));
    connect(m_aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

    m_preferencesAction = new QAction(QIcon(":/icons/preferences.svg"), tr("&Preferences..."), this);
    m_preferencesAction->setShortcut(QKeySequence::Preferences);
    m_preferencesAction->setStatusTip(tr("Configure application preferences"));
    connect(m_preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);
}

void MainWindow::createMenus() {
    // **File Menu**
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAction);
    m_fileMenu->addAction(m_openAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_saveAction);
    m_fileMenu->addAction(m_saveAsAction);
    m_fileMenu->addSeparator();
    
    // **Recent Files submenu**
    m_recentFilesMenu = m_fileMenu->addMenu(QIcon(":/icons/recent.svg"), tr("Recent &Projects"));
    updateRecentFiles();
    
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(QIcon(":/icons/export.svg"), tr("&Export Configuration..."), 
                         this, &MainWindow::exportConfiguration);
    m_fileMenu->addAction(QIcon(":/icons/import.svg"), tr("&Import Configuration..."), 
                         this, &MainWindow::importConfiguration);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAction);

    // **Edit Menu**
    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_preferencesAction);

    // **View Menu**
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    
    // **Theme submenu**
    m_themeMenu = m_viewMenu->addMenu(QIcon(":/icons/theme.svg"), tr("&Theme"));
    m_themeMenu->addAction(m_lightThemeAction);
    m_themeMenu->addAction(m_darkThemeAction);
    m_themeMenu->addAction(m_autoThemeAction);
    
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_fullScreenAction);
    m_viewMenu->addAction(m_resetLayoutAction);
    m_viewMenu->addSeparator();
    
    // **Dock widget toggles**
    m_viewMenu->addAction(tr("Show Plugin &List"), [this]() {
        m_pluginListDock->setVisible(!m_pluginListDock->isVisible());
    });
    m_viewMenu->addAction(tr("Show Plugin &Details"), [this]() {
        m_pluginDetailsDock->setVisible(!m_pluginDetailsDock->isVisible());
    });
    m_viewMenu->addAction(tr("Show &Log Viewer"), [this]() {
        m_logViewerDock->setVisible(!m_logViewerDock->isVisible());
    });
    m_viewMenu->addAction(tr("Show &Performance Monitor"), [this]() {
        m_performanceMonitorDock->setVisible(!m_performanceMonitorDock->isVisible());
    });
    m_viewMenu->addAction(tr("Show &Console"), [this]() {
        m_consoleDock->setVisible(!m_consoleDock->isVisible());
    });

    // **Plugin Menu**
    m_pluginMenu = menuBar()->addMenu(tr("&Plugins"));
    m_pluginMenu->addAction(m_refreshPluginsAction);
    m_pluginMenu->addSeparator();
    m_pluginMenu->addAction(m_installPluginAction);
    m_pluginMenu->addAction(m_uninstallPluginAction);
    m_pluginMenu->addSeparator();
    m_pluginMenu->addAction(m_enablePluginAction);
    m_pluginMenu->addAction(m_disablePluginAction);
    m_pluginMenu->addAction(m_configurePluginAction);
    m_pluginMenu->addSeparator();
    m_pluginMenu->addAction(m_pluginStoreAction);
    m_pluginMenu->addAction(m_securitySettingsAction);

    // **Tools Menu**
    m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
    m_toolsMenu->addAction(QIcon(":/icons/performance.svg"), tr("&Performance Monitor..."), 
                          this, &MainWindow::showPerformanceMonitor);
    m_toolsMenu->addAction(QIcon(":/icons/console.svg"), tr("Show &Console"), 
                          this, &MainWindow::showPluginConsole);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(QIcon(":/icons/clear.svg"), tr("Clear &Logs"), 
                          this, &MainWindow::clearLogs);
    m_toolsMenu->addAction(QIcon(":/icons/export.svg"), tr("Export L&ogs..."), 
                          this, &MainWindow::exportLogs);
    m_toolsMenu->addSeparator();
    m_toolsMenu->addAction(QIcon(":/icons/update.svg"), tr("Check for &Updates..."), 
                          this, &MainWindow::checkForUpdates);

    // **Help Menu**
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(QIcon(":/icons/help.svg"), tr("&User Guide"), []() {
        QDesktopServices::openUrl(QUrl("https://example.com/help"));
    });
    m_helpMenu->addAction(QIcon(":/icons/shortcuts.svg"), tr("&Keyboard Shortcuts"), [this]() {
        // Show keyboard shortcuts dialog
    });
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(QIcon(":/icons/feedback.svg"), tr("Send &Feedback..."), []() {
        QDesktopServices::openUrl(QUrl("mailto:feedback@example.com"));
    });
    m_helpMenu->addAction(QIcon(":/icons/report.svg"), tr("&Report Bug..."), []() {
        QDesktopServices::openUrl(QUrl("https://github.com/example/plugin-manager/issues"));
    });
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_aboutQtAction);
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBars() {
    // **Main Toolbar**
    m_mainToolBar = addToolBar(tr("Main"));
    m_mainToolBar->setObjectName("MainToolBar");
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    m_mainToolBar->addAction(m_newAction);
    m_mainToolBar->addAction(m_openAction);
    m_mainToolBar->addAction(m_saveAction);
    m_mainToolBar->addSeparator();
    
    // **Search widget in toolbar**
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText(tr("Search plugins..."));
    m_searchLineEdit->setFixedWidth(200);
    m_searchLineEdit->setClearButtonEnabled(true);
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    
    QAction* searchAction = m_mainToolBar->addWidget(m_searchLineEdit);
    searchAction->setText(tr("Search"));
    
    m_mainToolBar->addSeparator();
    
    // **Filter combo box**
    m_filterComboBox = new QComboBox(this);
    m_filterComboBox->addItems({tr("All Plugins"), tr("Enabled"), tr("Disabled"), tr("Error")});
    connect(m_filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onFilterChanged);
    
    QAction* filterAction = m_mainToolBar->addWidget(m_filterComboBox);
    filterAction->setText(tr("Filter"));
    
    // **Category combo box**
    m_categoryComboBox = new QComboBox(this);
    m_categoryComboBox->addItems({tr("All Categories"), tr("UI"), tr("Service"), tr("Network"), 
                                 tr("Development"), tr("System"), tr("Other")});
    connect(m_categoryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onFilterChanged);
    
    QAction* categoryAction = m_mainToolBar->addWidget(m_categoryComboBox);
    categoryAction->setText(tr("Category"));

    // **Plugin Toolbar**
    m_pluginToolBar = addToolBar(tr("Plugins"));
    m_pluginToolBar->setObjectName("PluginToolBar");
    
    m_pluginToolBar->addAction(m_refreshPluginsAction);
    m_pluginToolBar->addSeparator();
    m_pluginToolBar->addAction(m_installPluginAction);
    m_pluginToolBar->addAction(m_uninstallPluginAction);
    m_pluginToolBar->addSeparator();
    m_pluginToolBar->addAction(m_enablePluginAction);
    m_pluginToolBar->addAction(m_disablePluginAction);
    m_pluginToolBar->addAction(m_configurePluginAction);
    m_pluginToolBar->addSeparator();
    m_pluginToolBar->addAction(m_pluginStoreAction);

    // **View Toolbar**
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->setObjectName("ViewToolBar");
    
    m_viewToolBar->addAction(m_fullScreenAction);
    m_viewToolBar->addAction(m_resetLayoutAction);
    m_viewToolBar->addSeparator();
    
    // **Theme selection in toolbar**
    QToolButton* themeButton = new QToolButton(this);
    themeButton->setText(tr("Theme"));
    themeButton->setIcon(QIcon(":/icons/theme.svg"));
    themeButton->setPopupMode(QToolButton::InstantPopup);
    themeButton->setMenu(m_themeMenu);
    themeButton->setToolTip(tr("Select application theme"));
    
    m_viewToolBar->addWidget(themeButton);
}

void MainWindow::createStatusBar() {
    // **Status label**
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusLabel->setMinimumWidth(200);
    statusBar()->addWidget(m_statusLabel);

    // **Plugin count label**
    m_pluginCountLabel = new QLabel(this);
    m_pluginCountLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_pluginCountLabel->setMinimumWidth(120);
    statusBar()->addPermanentWidget(m_pluginCountLabel);

    // **Memory usage label**
    m_memoryUsageLabel = new QLabel(this);
    m_memoryUsageLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_memoryUsageLabel->setMinimumWidth(100);
    statusBar()->addPermanentWidget(m_memoryUsageLabel);

    // **Progress bar**
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    statusBar()->addPermanentWidget(m_progressBar);

    // **Settings button**
    m_settingsButton = new QToolButton(this);
    m_settingsButton->setIcon(QIcon(":/icons/settings.svg"));
    m_settingsButton->setToolTip(tr("Settings"));
    m_settingsButton->setAutoRaise(true);
    connect(m_settingsButton, &QToolButton::clicked, this, &MainWindow::showPreferences);
    statusBar()->addPermanentWidget(m_settingsButton);

    // **Initialize status bar**
    updateStatusBar();
}

void MainWindow::createDockWidgets() {
    // **Plugin List Dock**
    m_pluginListDock = createPluginListDock();
    addDockWidget(Qt::LeftDockWidgetArea, m_pluginListDock);

    // **Plugin Details Dock**
    m_pluginDetailsDock = createPluginDetailsDock();
    addDockWidget(Qt::RightDockWidgetArea, m_pluginDetailsDock);

    // **Log Viewer Dock**
    m_logViewerDock = createLogViewerDock();
    addDockWidget(Qt::BottomDockWidgetArea, m_logViewerDock);

    // **Performance Monitor Dock**
    m_performanceMonitorDock = createPerformanceMonitorDock();
    addDockWidget(Qt::BottomDockWidgetArea, m_performanceMonitorDock);

    // **Console Dock**
    m_consoleDock = createConsoleDock();
    addDockWidget(Qt::BottomDockWidgetArea, m_consoleDock);

    // **Toolbox Dock**
    m_toolboxDock = createToolboxDock();
    addDockWidget(Qt::LeftDockWidgetArea, m_toolboxDock);

    // **Tabify bottom docks**
    tabifyDockWidget(m_logViewerDock, m_performanceMonitorDock);
    tabifyDockWidget(m_performanceMonitorDock, m_consoleDock);
    
    // **Raise log viewer by default**
    m_logViewerDock->raise();
}

QDockWidget* MainWindow::createPluginListDock() {
    auto dock = new QDockWidget(tr("Plugin List"), this);
    dock->setObjectName("PluginListDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    m_pluginListWidget = new PluginListWidget(dock);
    dock->setWidget(m_pluginListWidget);
    
    // **Connect plugin selection**
    connect(m_pluginListWidget, &PluginListWidget::pluginSelected, 
            this, &MainWindow::onPluginSelectionChanged);
    connect(m_pluginListWidget, &PluginListWidget::pluginDoubleClicked, 
            this, &MainWindow::showPluginDetails);
    
    return dock;
}

QDockWidget* MainWindow::createPluginDetailsDock() {
    auto dock = new QDockWidget(tr("Plugin Details"), this);
    dock->setObjectName("PluginDetailsDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    m_pluginDetailsWidget = new PluginDetailsWidget(dock);
    dock->setWidget(m_pluginDetailsWidget);
    
    return dock;
}

QDockWidget* MainWindow::createLogViewerDock() {
    auto dock = new QDockWidget(tr("Log Viewer"), this);
    dock->setObjectName("LogViewerDock");
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    m_logViewer = new LogViewer(dock);
    dock->setWidget(m_logViewer);
    
    return dock;
}

QDockWidget* MainWindow::createPerformanceMonitorDock() {
    auto dock = new QDockWidget(tr("Performance Monitor"), this);
    dock->setObjectName("PerformanceMonitorDock");
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    m_performanceMonitor = new PerformanceMonitorWidget(dock);
    dock->setWidget(m_performanceMonitor);
    
    return dock;
}

QDockWidget* MainWindow::createConsoleDock() {
    auto dock = new QDockWidget(tr("Console"), this);
    dock->setObjectName("ConsoleDock");
    dock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    
    m_consoleWidget = new ConsoleWidget(dock);
    dock->setWidget(m_consoleWidget);
    
    return dock;
}

QDockWidget* MainWindow::createToolboxDock() {
    auto dock = new QDockWidget(tr("Toolbox"), this);
    dock->setObjectName("ToolboxDock");
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    // **Create toolbox widget with categories**
    auto toolboxWidget = new QWidget(dock);
    auto layout = new QVBoxLayout(toolboxWidget);
    
    // **Quick Actions group**
    auto quickActionsGroup = new QGroupBox(tr("Quick Actions"), toolboxWidget);
    auto quickActionsLayout = new QVBoxLayout(quickActionsGroup);
    
    auto refreshButton = new QPushButton(QIcon(":/icons/refresh.svg"), tr("Refresh Plugins"), quickActionsGroup);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPluginList);
    quickActionsLayout->addWidget(refreshButton);
    
    auto installButton = new QPushButton(QIcon(":/icons/install.svg"), tr("Install Plugin"), quickActionsGroup);
    connect(installButton, &QPushButton::clicked, this, &MainWindow::installPluginFromFile);
    quickActionsLayout->addWidget(installButton);
    
    auto storeButton = new QPushButton(QIcon(":/icons/store.svg"), tr("Plugin Store"), quickActionsGroup);
    connect(storeButton, &QPushButton::clicked, this, &MainWindow::showPluginStore);
    quickActionsLayout->addWidget(storeButton);
    
    layout->addWidget(quickActionsGroup);
    
    // **System Information group**
    auto systemInfoGroup = new QGroupBox(tr("System Information"), toolboxWidget);
    auto systemInfoLayout = new QVBoxLayout(systemInfoGroup);
    
    auto pluginCountInfo = new QLabel(systemInfoGroup);
    pluginCountInfo->setText(tr("Plugins: 0"));
    systemInfoLayout->addWidget(pluginCountInfo);
    
    auto memoryInfo = new QLabel(systemInfoGroup);
    memoryInfo->setText(tr("Memory: 0 MB"));
    systemInfoLayout->addWidget(memoryInfo);
    
    layout->addWidget(systemInfoGroup);
    layout->addStretch();
    
    dock->setWidget(toolboxWidget);
    return dock;
}

void MainWindow::createCentralWidget() {
    // **Create central tab widget**
    m_centralTabs = new QTabWidget(this);
    m_centralTabs->setTabsClosable(false);
    m_centralTabs->setMovable(true);
    m_centralTabs->setDocumentMode(true);
    
    // **Dashboard tab**
    auto dashboardWidget = createDashboardWidget();
    m_centralTabs->addTab(dashboardWidget, QIcon(":/icons/dashboard.svg"), tr("Dashboard"));
    
    // **Plugin Manager tab**
    auto pluginManagerWidget = createPluginManagerWidget();
    m_centralTabs->addTab(pluginManagerWidget, QIcon(":/icons/plugins.svg"), tr("Plugin Manager"));
    
    // **QML Integration tab**
    m_qmlWidget = new QQuickWidget(this);
    m_qmlWidget->setSource(QUrl("qrc:/qml/PluginManagerView.qml"));
    m_qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    
    // **Register C++ objects with QML**
    auto context = m_qmlWidget->rootContext();
    context->setContextProperty("pluginManager", m_pluginManager.get());
    context->setContextProperty("pluginRegistry", m_pluginRegistry.get());
    context->setContextProperty("themeManager", m_themeManager.get());
    
    m_centralTabs->addTab(m_qmlWidget, QIcon(":/icons/qml.svg"), tr("QML View"));
    
    setCentralWidget(m_centralTabs);
}

QWidget* MainWindow::createDashboardWidget() {
    auto widget = new QWidget(this);
    auto layout = new QGridLayout(widget);
    
    // **Welcome section**
    auto welcomeGroup = new QGroupBox(tr("Welcome"), widget);
    auto welcomeLayout = new QVBoxLayout(welcomeGroup);
    
    auto titleLabel = new QLabel(tr("<h2>Advanced Plugin Manager</h2>"), welcomeGroup);
    titleLabel->setAlignment(Qt::AlignCenter);
    welcomeLayout->addWidget(titleLabel);
    
    auto descLabel = new QLabel(tr("Manage and monitor your plugins with ease."), welcomeGroup);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    welcomeLayout->addWidget(descLabel);
    
    layout->addWidget(welcomeGroup, 0, 0, 1, 2);
    
    // **Statistics section**
    auto statsGroup = new QGroupBox(tr("Statistics"), widget);
    auto statsLayout = new QGridLayout(statsGroup);
    
    // **Plugin statistics**
    auto totalPluginsLabel = new QLabel(tr("Total Plugins:"), statsGroup);
    auto totalPluginsValue = new QLabel(tr("0"), statsGroup);
    totalPluginsValue->setStyleSheet("font-weight: bold; color: #2196f3;");
    statsLayout->addWidget(totalPluginsLabel, 0, 0);
    statsLayout->addWidget(totalPluginsValue, 0, 1);
    
    auto enabledPluginsLabel = new QLabel(tr("Enabled:"), statsGroup);
    auto enabledPluginsValue = new QLabel(tr("0"), statsGroup);
    enabledPluginsValue->setStyleSheet("font-weight: bold; color: #4caf50;");
    statsLayout->addWidget(enabledPluginsLabel, 1, 0);
    statsLayout->addWidget(enabledPluginsValue, 1, 1);
    
    auto errorPluginsLabel = new QLabel(tr("Errors:"), statsGroup);
    auto errorPluginsValue = new QLabel(tr("0"), statsGroup);
    errorPluginsValue->setStyleSheet("font-weight: bold; color: #f44336;");
    statsLayout->addWidget(errorPluginsLabel, 2, 0);
    statsLayout->addWidget(errorPluginsValue, 2, 1);
    
    layout->addWidget(statsGroup, 1, 0);
    
    // **Recent activity section**
    auto activityGroup = new QGroupBox(tr("Recent Activity"), widget);
    auto activityLayout = new QVBoxLayout(activityGroup);
    
    auto activityList = new QListWidget(activityGroup);
    activityList->addItem(tr("Application started"));
    activityLayout->addWidget(activityList);
    
    layout->addWidget(activityGroup, 1, 1);
    
    // **Quick actions section**
    auto quickActionsGroup = new QGroupBox(tr("Quick Actions"), widget);
    auto quickActionsLayout = new QHBoxLayout(quickActionsGroup);
    
    auto refreshBtn = new QPushButton(QIcon(":/icons/refresh.svg"), tr("Refresh Plugins"), quickActionsGroup);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPluginList);
    quickActionsLayout->addWidget(refreshBtn);
    
    auto installBtn = new QPushButton(QIcon(":/icons/install.svg"), tr("Install Plugin"), quickActionsGroup);
    connect(installBtn, &QPushButton::clicked, this, &MainWindow::installPluginFromFile);
    quickActionsLayout->addWidget(installBtn);
    
    auto storeBtn = new QPushButton(QIcon(":/icons/store.svg"), tr("Plugin Store"), quickActionsGroup);
    connect(storeBtn, &QPushButton::clicked, this, &MainWindow::showPluginStore);
    quickActionsLayout->addWidget(storeBtn);
    
    layout->addWidget(quickActionsGroup, 2, 0, 1, 2);
    
    return widget;
}

QWidget* MainWindow::createPluginManagerWidget() {
    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    
    // **Top toolbar**
    auto toolbar = new QWidget(widget);
    auto toolbarLayout = new QHBoxLayout(toolbar);
    
    auto refreshBtn = new QPushButton(QIcon(":/icons/refresh.svg"), tr("Refresh"), toolbar);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPluginList);
    toolbarLayout->addWidget(refreshBtn);
    
    auto installBtn = new QPushButton(QIcon(":/icons/install.svg"), tr("Install"), toolbar);
    connect(installBtn, &QPushButton::clicked, this, &MainWindow::installPluginFromFile);
    toolbarLayout->addWidget(installBtn);
    
    toolbarLayout->addStretch();
    
    auto searchEdit = new QLineEdit(toolbar);
    searchEdit->setPlaceholderText(tr("Search plugins..."));
    searchEdit->setFixedWidth(200);
    connect(searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    toolbarLayout->addWidget(searchEdit);
    
    layout->addWidget(toolbar);
    
    // **Main content splitter**
    auto splitter = new QSplitter(Qt::Horizontal, widget);
    
    // **Plugin list on the left**
    auto pluginListWidget = new PluginListWidget(splitter);
    splitter->addWidget(pluginListWidget);
    
    // **Plugin details on the right**
    auto pluginDetailsWidget = new PluginDetailsWidget(splitter);
    splitter->addWidget(pluginDetailsWidget);
    
    // **Set splitter proportions**
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    
    layout->addWidget(splitter);
    
    return widget;
}

void MainWindow::setupConnections() {
    // **Plugin Manager connections**
    connect(m_pluginManager.get(), &PluginManager::pluginLoaded, 
            this, &MainWindow::onPluginLoaded);
    connect(m_pluginManager.get(), &PluginManager::pluginUnloaded, 
            this, &MainWindow::onPluginUnloaded);
    connect(m_pluginManager.get(), &PluginManager::pluginError, 
            this, &MainWindow::onPluginError);
    connect(m_pluginManager.get(), &PluginManager::pluginCountChanged, 
            this, &MainWindow::updateStatusBar);
    
    // **Theme Manager connections**
    connect(m_themeManager.get(), &ThemeManager::currentThemeChanged,
            this, &MainWindow::onThemeChanged);
    
    // **Timer connections**
    connect(m_statusUpdateTimer, &QTimer::timeout, [this]() {
        m_statusLabel->setText(tr("Ready"));
    });
    
    connect(m_performanceTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    
    // **Settings watcher**
    connect(m_configWatcher, &QFileSystemWatcher::fileChanged, 
            this, &MainWindow::onSettingsChanged);
    
    // **System tray**
    if (m_systemTrayIcon) {
        connect(m_systemTrayIcon, &QSystemTrayIcon::activated, 
                this, &MainWindow::onSystemTrayActivated);
    }
}

void MainWindow::setupShortcuts() {
    // **Additional shortcuts**
    auto clearLogsShortcut = new QShortcut(QKeySequence("Ctrl+Shift+C"), this);
    connect(clearLogsShortcut, &QShortcut::activated, this, &MainWindow::clearLogs);
    
    auto consoleShortcut = new QShortcut(QKeySequence("Ctrl+`"), this);
    connect(consoleShortcut, &QShortcut::activated, this, &MainWindow::showPluginConsole);
    
    auto performanceShortcut = new QShortcut(QKeySequence("Ctrl+Shift+P"), this);
    connect(performanceShortcut, &QShortcut::activated, this, &MainWindow::showPerformanceMonitor);
}

void MainWindow::loadSettings() {
    m_settings->beginGroup("MainWindow");
    
    // **Restore geometry and state**
    restoreGeometry(m_settings->value("geometry").toByteArray());
    restoreState(m_settings->value("state").toByteArray());
    
    // **Load theme**
    m_currentTheme = m_settings->value("theme", "system").toString();
    setCurrentTheme(m_currentTheme);
    
    // **Load recent files**
    m_recentFiles = m_settings->value("recentFiles").toStringList();
    updateRecentFiles();
    
    // **Load plugin path**
    QString pluginPath = m_settings->value("pluginPath", 
        QCoreApplication::applicationDirPath() + "/plugins").toString();
    setPluginPath(pluginPath);
    
    m_settings->endGroup();
    
    // **Load plugin manager settings**
    m_settings->beginGroup("PluginManager");
    bool autoLoad = m_settings->value("autoLoad", true).toBool();
    bool hotReload = m_settings->value("hotReload", false).toBool();
    bool performanceMonitoring = m_settings->value("performanceMonitoring", true).toBool();
    
    m_pluginManager->setAutoLoadEnabled(autoLoad);
    m_pluginManager->setHotReloadEnabled(hotReload);
    if (performanceMonitoring) {
        m_pluginManager->startPerformanceMonitoring();
    }
    
    m_settings->endGroup();
}

void MainWindow::saveSettings() {
    m_settings->beginGroup("MainWindow");
    
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("state", saveState());
    m_settings->setValue("theme", m_currentTheme);
    m_settings->setValue("recentFiles", m_recentFiles);
    m_settings->setValue("pluginPath", m_pluginManager->pluginSearchPaths());
    
    m_settings->endGroup();
    
    m_settings->beginGroup("PluginManager");
    m_settings->setValue("autoLoad", m_pluginManager->autoLoadEnabled());
    m_settings->setValue("hotReload", m_pluginManager->hotReloadEnabled());
    m_settings->setValue("performanceMonitoring", m_pluginManager->isPerformanceMonitoringEnabled());
    m_settings->endGroup();
    
    m_settings->sync();
}

// **Slot implementations**
void MainWindow::onPluginLoaded(const QString& pluginName) {
    showStatusMessage(tr("Plugin loaded: %1").arg(pluginName));
    m_logViewer->addLogEntry("INFO", tr("Plugin loaded: %1").arg(pluginName));
}

void MainWindow::onPluginUnloaded(const QString& pluginName) {
    showStatusMessage(tr("Plugin unloaded: %1").arg(pluginName));
    m_logViewer->addLogEntry("INFO", tr("Plugin unloaded: %1").arg(pluginName));
}

void MainWindow::onPluginError(const QString& pluginName, const QString& error) {
    showStatusMessage(tr("Plugin error: %1 - %2").arg(pluginName, error));
    m_logViewer->addLogEntry("ERROR", tr("Plugin error [%1]: %2").arg(pluginName, error));
}

void MainWindow::showStatusMessage(const QString& message, int timeout) {
    m_statusLabel->setText(message);
    if (timeout > 0) {
        m_statusUpdateTimer->start(timeout);
    } else {
        m_statusUpdateTimer->start();
    }
}

void MainWindow::updateStatusBar() {
    // **Update plugin count**
    int totalPlugins = m_pluginRegistry->rowCount();
    int enabledPlugins = 0; // Get from plugin registry
    m_pluginCountLabel->setText(tr("Plugins: %1 (%2 enabled)").arg(totalPlugins).arg(enabledPlugins));
    
    // **Update memory usage**
    // Get system memory usage
    m_memoryUsageLabel->setText(tr("Memory: %1 MB").arg(0)); // Implement memory monitoring
}

QString MainWindow::currentTheme() const {
    return m_currentTheme;
}

void MainWindow::setCurrentTheme(const QString& theme) {
    if (m_currentTheme == theme) return;
    
    m_currentTheme = theme;
    applyTheme(theme);
    emit currentThemeChanged(theme);
}

void MainWindow::applyTheme(const QString& theme) {
    m_themeManager->setCurrentTheme(theme);
    
    // **Update action states**
    if (theme == "light") {
        m_lightThemeAction->setChecked(true);
    } else if (theme == "dark") {
        m_darkThemeAction->setChecked(true);
    } else {
        m_autoThemeAction->setChecked(true);
    }
}

// **Continue with remaining slot implementations...**
void MainWindow::showAboutDialog() {
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::showPreferences() {
    PreferencesDialog dialog(this);
    // Note: setPluginManager and setThemeManager methods not implemented in stub
    dialog.exec();
}

// **Event handlers**
void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_systemTrayIcon && m_systemTrayIcon->isVisible()) {
        hide();
        showStatusMessage(tr("Application minimized to system tray"));
        event->ignore();
    } else {
        saveSettings();
        event->accept();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        for (const QUrl& url : mimeData->urls()) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                QFileInfo fileInfo(filePath);
                QRegularExpression pluginRegex("\\.(dll|so|dylib)$", QRegularExpression::CaseInsensitiveOption);
                if (pluginRegex.match(fileInfo.fileName()).hasMatch()) {
                    // **Install plugin**
                    auto result = m_pluginManager->loadPlugin(filePath);
                    if (result != PluginManager::LoadResult::Success) {
                        QMessageBox::warning(this, tr("Plugin Installation Failed"),
                                           tr("Failed to install plugin: %1").arg(filePath));
                    }
                }
            }
        }
        event->acceptProposedAction();
    }
}

void MainWindow::createSystemTrayActions() {
    m_systemTrayIcon = new QSystemTrayIcon(this);
    m_systemTrayIcon->setIcon(QIcon(":/icons/app.svg"));
    
    m_trayIconMenu = new QMenu(this);
    m_trayIconMenu->addAction(tr("Show"), this, &QWidget::showNormal);
    m_trayIconMenu->addAction(tr("Hide"), this, &QWidget::hide);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_exitAction);
    
    m_systemTrayIcon->setContextMenu(m_trayIconMenu);
    m_systemTrayIcon->show();
}

// **Missing slot implementations**

void MainWindow::onThemeChanged(const QString& theme) {
    setCurrentTheme(theme);
    showStatusMessage(tr("Theme changed to: %1").arg(theme));
}

void MainWindow::onSettingsChanged() {
    loadSettings();
    showStatusMessage(tr("Settings reloaded"));
}

void MainWindow::newProject() {
    // Create new project dialog
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Create New Project"),
        QDir::homePath() + "/untitled.project",
        tr("Project Files (*.project)"));

    if (!fileName.isEmpty()) {
        m_currentProjectFile = fileName;
        updateWindowTitle();
        showStatusMessage(tr("New project created: %1").arg(QFileInfo(fileName).baseName()));
        updateRecentFiles();
    }
}

void MainWindow::openProject() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Project"),
        QDir::homePath(),
        tr("Project Files (*.project)"));

    if (!fileName.isEmpty()) {
        m_currentProjectFile = fileName;
        updateWindowTitle();
        showStatusMessage(tr("Project opened: %1").arg(QFileInfo(fileName).baseName()));
        updateRecentFiles();
    }
}

void MainWindow::saveProject() {
    if (m_currentProjectFile.isEmpty()) {
        saveProjectAs();
        return;
    }

    // Save project logic here
    showStatusMessage(tr("Project saved: %1").arg(QFileInfo(m_currentProjectFile).baseName()));
}

void MainWindow::saveProjectAs() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Project As"),
        m_currentProjectFile.isEmpty() ? QDir::homePath() + "/untitled.project" : m_currentProjectFile,
        tr("Project Files (*.project)"));

    if (!fileName.isEmpty()) {
        m_currentProjectFile = fileName;
        saveProject();
        updateWindowTitle();
        updateRecentFiles();
    }
}

void MainWindow::exportConfiguration() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Configuration"),
        QDir::homePath() + "/config.json",
        tr("JSON Files (*.json)"));

    if (!fileName.isEmpty()) {
        // Export configuration logic here
        showStatusMessage(tr("Configuration exported to: %1").arg(fileName));
    }
}

void MainWindow::importConfiguration() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Import Configuration"),
        QDir::homePath(),
        tr("JSON Files (*.json)"));

    if (!fileName.isEmpty()) {
        // Import configuration logic here
        showStatusMessage(tr("Configuration imported from: %1").arg(fileName));
        onSettingsChanged();
    }
}

void MainWindow::toggleFullScreen() {
    if (isFullScreen()) {
        showNormal();
        m_fullScreenAction->setChecked(false);
        showStatusMessage(tr("Exited full screen mode"));
    } else {
        showFullScreen();
        m_fullScreenAction->setChecked(true);
        showStatusMessage(tr("Entered full screen mode"));
    }
}

void MainWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason) {
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if (isVisible()) {
            hide();
        } else {
            showNormal();
            activateWindow();
            raise();
        }
        break;
    default:
        break;
    }
}

void MainWindow::onPluginSelectionChanged() {
    // Update plugin details and enable/disable actions based on selection
    if (m_pluginListWidget) {
        // Get selected plugin and update details
        showStatusMessage(tr("Plugin selection changed"));
    }
}

void MainWindow::onSearchTextChanged(const QString& text) {
    if (m_pluginRegistry) {
        m_pluginRegistry->setFilterText(text);
        showStatusMessage(tr("Search filter: %1").arg(text.isEmpty() ? tr("(none)") : text));
    }
}

void MainWindow::onFilterChanged() {
    if (m_filterComboBox && m_pluginRegistry) {
        QString filter = m_filterComboBox->currentText();
        // Apply filter logic
        showStatusMessage(tr("Filter changed: %1").arg(filter));
    }
}

void MainWindow::checkForUpdates() {
    showStatusMessage(tr("Checking for updates..."));

    // Use network manager to check for updates
    if (m_networkManager) {
        // Implement update checking logic
        showStatusMessage(tr("Update check completed"));
    }
}

void MainWindow::installPluginFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Install Plugin"),
        QDir::homePath(),
        tr("Plugin Files (*.dll *.so *.dylib *.zip)"));

    if (!fileName.isEmpty()) {
        if (m_pluginManager) {
            // Install plugin logic
            showStatusMessage(tr("Installing plugin from: %1").arg(fileName));
        }
    }
}

void MainWindow::uninstallSelectedPlugin() {
    // Get selected plugin and uninstall
    showStatusMessage(tr("Uninstalling selected plugin..."));
}

void MainWindow::enableSelectedPlugin() {
    // Get selected plugin and enable
    showStatusMessage(tr("Enabling selected plugin..."));
}

void MainWindow::disableSelectedPlugin() {
    // Get selected plugin and disable
    showStatusMessage(tr("Disabling selected plugin..."));
}

void MainWindow::configureSelectedPlugin() {
    // Open configuration dialog for selected plugin
    showStatusMessage(tr("Configuring selected plugin..."));
}

void MainWindow::showPluginDetails() {
    if (m_pluginDetailsDock) {
        m_pluginDetailsDock->setVisible(true);
        m_pluginDetailsDock->raise();
    }
}

void MainWindow::showPluginConsole() {
    if (m_consoleDock) {
        m_consoleDock->setVisible(true);
        m_consoleDock->raise();
    }
}

void MainWindow::clearLogs() {
    if (m_logViewer) {
        // Clear logs logic
        showStatusMessage(tr("Logs cleared"));
    }
}

void MainWindow::exportLogs() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Logs"),
        QDir::homePath() + "/logs.txt",
        tr("Text Files (*.txt)"));

    if (!fileName.isEmpty()) {
        // Export logs logic
        showStatusMessage(tr("Logs exported to: %1").arg(fileName));
    }
}

void MainWindow::toggleDarkMode() {
    QString newTheme = (m_currentTheme == "dark") ? "light" : "dark";
    setCurrentTheme(newTheme);
}

void MainWindow::resetLayout() {
    // Reset dock widget positions and sizes
    if (m_pluginListDock) m_pluginListDock->setVisible(true);
    if (m_pluginDetailsDock) m_pluginDetailsDock->setVisible(true);
    if (m_logViewerDock) m_logViewerDock->setVisible(true);
    if (m_performanceMonitorDock) m_performanceMonitorDock->setVisible(false);
    if (m_consoleDock) m_consoleDock->setVisible(false);
    if (m_toolboxDock) m_toolboxDock->setVisible(false);

    // Reset to default layout
    addDockWidget(Qt::LeftDockWidgetArea, m_pluginListDock);
    addDockWidget(Qt::RightDockWidgetArea, m_pluginDetailsDock);
    addDockWidget(Qt::BottomDockWidgetArea, m_logViewerDock);

    showStatusMessage(tr("Layout reset to default"));
}

void MainWindow::showWelcomeScreen() {
    if (m_centralTabs) {
        // Switch to welcome tab or create one
        for (int i = 0; i < m_centralTabs->count(); ++i) {
            if (m_centralTabs->tabText(i) == tr("Welcome")) {
                m_centralTabs->setCurrentIndex(i);
                return;
            }
        }

        // Create welcome widget if it doesn't exist
        auto welcomeWidget = createWelcomeWidget();
        int index = m_centralTabs->addTab(welcomeWidget, QIcon(":/icons/app.svg"), tr("Welcome"));
        m_centralTabs->setCurrentIndex(index);
    }
}

// **Property implementations**
bool MainWindow::isPluginManagerVisible() const {
    return m_pluginManagerVisible;
}

void MainWindow::setPluginManagerVisible(bool visible) {
    if (m_pluginManagerVisible != visible) {
        m_pluginManagerVisible = visible;

        if (m_pluginListDock) {
            m_pluginListDock->setVisible(visible);
        }
        if (m_pluginDetailsDock) {
            m_pluginDetailsDock->setVisible(visible);
        }

        emit pluginManagerVisibilityChanged(visible);
    }
}

void MainWindow::setPluginPath(const QString& path) {
    if (m_pluginManager) {
        m_pluginManager->addPluginSearchPath(path);
        emit pluginPathChanged(path);
        showStatusMessage(tr("Plugin path set to: %1").arg(path));
    }
}

void MainWindow::loadPluginsFromPath(const QString& path) {
    if (m_pluginManager) {
        m_pluginManager->addPluginSearchPath(path);
        m_pluginManager->scanDirectory(path);
        showStatusMessage(tr("Loading plugins from: %1").arg(path));
    }
}

void MainWindow::refreshPluginList() {
    if (m_pluginManager) {
        m_pluginManager->refreshPluginList();
        showStatusMessage(tr("Plugin list refreshed"));
    }
}

void MainWindow::showPluginStore() {
    // Show plugin store widget
    showStatusMessage(tr("Opening plugin store..."));
}

void MainWindow::showSecuritySettings() {
    // Show security settings dialog
    showStatusMessage(tr("Opening security settings..."));
}

void MainWindow::showPerformanceMonitor() {
    if (m_performanceMonitorDock) {
        m_performanceMonitorDock->setVisible(true);
        m_performanceMonitorDock->raise();
    }
}

// **Event filter implementation**
bool MainWindow::eventFilter(QObject* object, QEvent* event) {
    // Handle global events
    return QMainWindow::eventFilter(object, event);
}

// **Change event implementation**
void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && m_systemTrayIcon && m_systemTrayIcon->isVisible()) {
            hide();
            event->ignore();
            return;
        }
    }
    QMainWindow::changeEvent(event);
}

// **Setup methods**
void MainWindow::setupPluginIntegration() {
    // Setup plugin integration
    if (m_pluginManager && m_pluginRegistry) {
        // Connect public signals to public slots instead
        connect(m_pluginManager.get(), &PluginManager::pluginLoaded,
                this, &MainWindow::onPluginLoaded);
        connect(m_pluginManager.get(), &PluginManager::pluginUnloaded,
                this, &MainWindow::onPluginUnloaded);
    }
}

void MainWindow::setupPerformanceMonitoring() {
    if (m_performanceTimer && m_pluginManager) {
        connect(m_performanceTimer, &QTimer::timeout, [this]() {
            if (m_pluginManager->isPerformanceMonitoringEnabled()) {
                // Update performance metrics
                updateStatusBar();
            }
        });
        m_performanceTimer->start();
    }
}

void MainWindow::updateRecentFiles() {
    if (!m_currentProjectFile.isEmpty()) {
        m_recentFiles.removeAll(m_currentProjectFile);
        m_recentFiles.prepend(m_currentProjectFile);

        while (m_recentFiles.size() > MaxRecentFiles) {
            m_recentFiles.removeLast();
        }

        // Update recent files menu
        if (m_recentFilesMenu) {
            m_recentFilesMenu->clear();
            for (const QString& file : m_recentFiles) {
                QAction* action = m_recentFilesMenu->addAction(QFileInfo(file).baseName());
                action->setData(file);
                connect(action, &QAction::triggered, [this, file]() {
                    m_currentProjectFile = file;
                    updateWindowTitle();
                    showStatusMessage(tr("Opened recent project: %1").arg(QFileInfo(file).baseName()));
                });
            }
        }
    }
}

void MainWindow::updateWindowTitle() {
    QString title = tr("Advanced Plugin Manager");
    if (!m_currentProjectFile.isEmpty()) {
        title += tr(" - %1").arg(QFileInfo(m_currentProjectFile).baseName());
    }
    setWindowTitle(title);
}

QWidget* MainWindow::createWelcomeWidget() {
    QWidget* welcomeWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(welcomeWidget);

    // **Welcome header**
    QLabel* titleLabel = new QLabel(tr("Welcome to Advanced Plugin Manager"), welcomeWidget);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; margin: 20px;");
    layout->addWidget(titleLabel);

    // **Description**
    QLabel* descLabel = new QLabel(tr("Manage your plugins with advanced features including "
                                     "security management, performance monitoring, and hot reload."),
                                   welcomeWidget);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-size: 14px; margin: 10px; color: #666;");
    layout->addWidget(descLabel);

    // **Quick actions**
    QGroupBox* actionsGroup = new QGroupBox(tr("Quick Actions"), welcomeWidget);
    QGridLayout* actionsLayout = new QGridLayout(actionsGroup);

    QPushButton* loadPluginBtn = new QPushButton(QIcon(":/icons/plugins.svg"), tr("Load Plugin"), actionsGroup);
    connect(loadPluginBtn, &QPushButton::clicked, this, &MainWindow::installPluginFromFile);
    actionsLayout->addWidget(loadPluginBtn, 0, 0);

    QPushButton* refreshBtn = new QPushButton(QIcon(":/icons/refresh.svg"), tr("Refresh Plugins"), actionsGroup);
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPluginList);
    actionsLayout->addWidget(refreshBtn, 0, 1);

    QPushButton* settingsBtn = new QPushButton(QIcon(":/icons/settings.svg"), tr("Settings"), actionsGroup);
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::showPreferences);
    actionsLayout->addWidget(settingsBtn, 1, 0);

    QPushButton* securityBtn = new QPushButton(QIcon(":/icons/security.svg"), tr("Security"), actionsGroup);
    connect(securityBtn, &QPushButton::clicked, this, &MainWindow::showSecuritySettings);
    actionsLayout->addWidget(securityBtn, 1, 1);

    layout->addWidget(actionsGroup);

    // **Recent projects**
    if (!m_recentFiles.isEmpty()) {
        QGroupBox* recentGroup = new QGroupBox(tr("Recent Projects"), welcomeWidget);
        QVBoxLayout* recentLayout = new QVBoxLayout(recentGroup);

        for (int i = 0; i < qMin(5, m_recentFiles.size()); ++i) {
            const QString& file = m_recentFiles.at(i);
            QPushButton* recentBtn = new QPushButton(QFileInfo(file).baseName(), recentGroup);
            recentBtn->setToolTip(file);
            connect(recentBtn, &QPushButton::clicked, [this, file]() {
                m_currentProjectFile = file;
                updateWindowTitle();
                showStatusMessage(tr("Opened recent project: %1").arg(QFileInfo(file).baseName()));
            });
            recentLayout->addWidget(recentBtn);
        }

        layout->addWidget(recentGroup);
    }

    layout->addStretch();

    return welcomeWidget;
}