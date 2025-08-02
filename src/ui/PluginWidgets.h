// PluginWidgets.h - UI widgets for plugin management
#pragma once

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QDialog>

// Plugin List Widget
class PluginListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit PluginListWidget(QWidget* parent = nullptr);
    
    void refreshPlugins();
    void filterPlugins(const QString& filter);

signals:
    void pluginSelected(const QString& pluginName);
    void pluginDoubleClicked(const QString& pluginName);

private slots:
    void onItemSelectionChanged();
    void onItemDoubleClicked();
};

// Forward declarations for classes defined in other headers
class PluginDetailsWidget;
class PluginStoreWidget;

// Log Viewer Widget
class LogViewer : public QTextEdit {
    Q_OBJECT

public:
    explicit LogViewer(QWidget* parent = nullptr);
    
    void addLogEntry(const QString& level, const QString& message);
    void clearLogs();
    void exportLogs(const QString& fileName);

public slots:
    void onLogMessage(const QString& message);

private:
    int m_maxLines;
    
    void limitLines();
};

// Performance Monitor Widget
class PerformanceMonitorWidget : public QWidget {
    Q_OBJECT

public:
    explicit PerformanceMonitorWidget(QWidget* parent = nullptr);
    
    void updateMetrics();
    void startMonitoring();
    void stopMonitoring();

private:
    QLabel* m_cpuLabel;
    QLabel* m_memoryLabel;
    QProgressBar* m_cpuBar;
    QProgressBar* m_memoryBar;
    
    void setupUI();
};

// Console Widget
class ConsoleWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConsoleWidget(QWidget* parent = nullptr);
    
    void executeCommand(const QString& command);
    void addOutput(const QString& output);
    void clearConsole();

signals:
    void commandExecuted(const QString& command);

private:
    QTextEdit* m_output;
    class QLineEdit* m_input;
    
    void setupUI();

private slots:
    void onReturnPressed();
};

// Security Manager Widget
class SecurityManager : public QWidget {
    Q_OBJECT

public:
    explicit SecurityManager(QWidget* parent = nullptr);
    
    void updateSecurityStatus();
    void showSecuritySettings();

private:
    QLabel* m_statusLabel;
    QPushButton* m_settingsButton;
    
    void setupUI();
};

// Preferences Dialog
class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);
    
    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    void createGeneralPage();
    void createPluginsPage();
    void createThemePage();
};

// About Dialog
class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    void setupUI();
};
