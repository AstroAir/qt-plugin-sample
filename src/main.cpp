// main.cpp - Application entry point
#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include "ui/MainWindow.h"
#include "managers/ApplicationManager.h"

Q_LOGGING_CATEGORY(appLog, "application")

int main(int argc, char *argv[])
{
    // **Enable high DPI scaling** (Qt6 handles this automatically)
    
    QApplication app(argc, argv);
    
    // **Set application properties**
    app.setApplicationName("Advanced Plugin Manager");
    app.setApplicationVersion("3.0.0");
    app.setOrganizationName("Example Corporation");
    app.setOrganizationDomain("example.com");
    app.setApplicationDisplayName("Advanced Plugin Manager v3.0.0 - Component Architecture");
    
    // **Parse command line arguments**
    QCommandLineParser parser;
    parser.setApplicationDescription("Advanced Plugin Management System");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Enable debug output");
    parser.addOption(debugOption);
    
    QCommandLineOption pluginPathOption(QStringList() << "p" << "plugin-path", 
                                       "Plugin search path", "path");
    parser.addOption(pluginPathOption);
    
    QCommandLineOption noSplashOption("no-splash", "Disable splash screen");
    parser.addOption(noSplashOption);
    
    parser.process(app);
    
    // **Setup logging**
    if (parser.isSet(debugOption)) {
        QLoggingCategory::setFilterRules("*.debug=true");
    }
    
    // **Show splash screen**
    QSplashScreen* splash = nullptr;
    if (!parser.isSet(noSplashOption)) {
        QPixmap pixmap(":/images/splash.png");
        splash = new QSplashScreen(pixmap);
        splash->show();
        splash->showMessage("Initializing...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
    }
    
    // **Set application style** (removed QQuickStyle for compatibility)
    
    try {
        // **Initialize application manager**
        ApplicationManager appManager;
        
        if (splash) {
            splash->showMessage("Loading plugins...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
            app.processEvents();
        }
        
        // **Initialize and show main window**
        MainWindow mainWindow;
        
        // **Set plugin path if specified**
        if (parser.isSet(pluginPathOption)) {
            QString pluginPath = parser.value(pluginPathOption);
            mainWindow.setPluginPath(pluginPath);
        }
        
        if (splash) {
            splash->showMessage("Starting application...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
            app.processEvents();
            
            QTimer::singleShot(2000, [splash, &mainWindow]() {
                splash->finish(&mainWindow);
                splash->deleteLater();
                mainWindow.show();
            });
        } else {
            mainWindow.show();
        }
        
        qCInfo(appLog) << "Application started successfully";

        return app.exec();

    } catch (const std::exception& e) {
        qCCritical(appLog) << "Fatal error:" << e.what();
        if (splash) splash->deleteLater();
        return -1;
    }
}