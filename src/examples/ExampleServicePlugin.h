// ExampleServicePlugin.h - Example Service Plugin demonstrating background services
#pragma once

#include "../core/PluginInterface.h"
#include "../core/AdvancedInterfaces.h"
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QJsonObject>
#include <QUuid>
#include <QVersionNumber>
#include <QDateTime>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>

class ServiceWorker;
class TaskProcessor;
class FileMonitor;
class NetworkMonitor;

class ExampleServicePlugin : public QObject, public IServicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.IPlugin/2.0" FILE "ExampleServicePlugin.json")
    Q_INTERFACES(IPlugin IServicePlugin)

public:
    ExampleServicePlugin(QObject* parent = nullptr);
    ~ExampleServicePlugin() override;

    // IPlugin interface
    QString name() const override { return "Example Service Plugin"; }
    QString description() const override { return "Demonstrates background service capabilities including task processing, file monitoring, and network operations"; }
    QVersionNumber version() const override { return QVersionNumber(1, 1, 0); }
    QString author() const override { return "Plugin Framework Team"; }
    QUuid uuid() const override { return QUuid("{87654321-4321-8765-dcba-876543210fed}"); }
    QString category() const override { return "Service"; }
    QString homepage() const override { return "https://example.com/service-plugin"; }
    QString license() const override { return "MIT"; }

    PluginCapabilities capabilities() const override {
        return PluginCapability::Service | PluginCapability::Threading | 
               PluginCapability::FileSystem | PluginCapability::Network;
    }

    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override { return m_initialized; }
    PluginStatus status() const override { return m_status; }

    // Configuration
    QJsonObject defaultConfiguration() const override;
    bool configure(const QJsonObject& config) override;
    QJsonObject currentConfiguration() const override;

    // IServicePlugin interface
    bool startService() override;
    bool stopService() override;
    bool pauseService() override;
    bool resumeService() override;
    bool isServiceRunning() const override { return m_serviceRunning; }
    ServiceStatus serviceStatus() const override { return m_serviceStatus; }
    QJsonObject getServiceMetrics() const override;

    // Commands
    QVariant executeCommand(const QString& command, const QVariantMap& params = {}) override;
    QStringList availableCommands() const override;

signals:
    void serviceStarted();
    void serviceStopped();
    void servicePaused();
    void serviceResumed();
    void taskCompleted(const QString& taskId, const QJsonObject& result);
    void fileChanged(const QString& filePath);
    void networkStatusChanged(bool connected);
    void metricsUpdated(const QJsonObject& metrics);

public slots:
    void processTask(const QString& taskId, const QJsonObject& taskData);
    void scheduleTask(const QString& taskId, const QJsonObject& taskData, int delayMs = 0);
    void cancelTask(const QString& taskId);
    void addFileWatch(const QString& filePath);
    void removeFileWatch(const QString& filePath);
    void checkNetworkStatus();

private slots:
    void onWorkerFinished();
    void onTaskProcessed(const QString& taskId, const QJsonObject& result);
    void onFileChanged(const QString& path);
    void onNetworkReplyFinished();
    void updateMetrics();
    void performPeriodicTasks();

private:
    void setupWorkers();
    void cleanupWorkers();
    void startMetricsCollection();
    void stopMetricsCollection();
    void logActivity(const QString& activity);

    bool m_initialized = false;
    bool m_serviceRunning = false;
    PluginStatus m_status = PluginStatus::Unknown;
    ServiceStatus m_serviceStatus = ServiceStatus::Stopped;
    QJsonObject m_configuration;

    // Service components
    std::unique_ptr<ServiceWorker> m_worker;
    std::unique_ptr<TaskProcessor> m_taskProcessor;
    std::unique_ptr<FileMonitor> m_fileMonitor;
    std::unique_ptr<NetworkMonitor> m_networkMonitor;

    // Timers
    QTimer* m_metricsTimer = nullptr;
    QTimer* m_periodicTimer = nullptr;

    // Metrics
    QDateTime m_startTime;
    int m_tasksProcessed = 0;
    int m_tasksQueued = 0;
    int m_filesWatched = 0;
    bool m_networkConnected = false;
    QStringList m_activityLog;

    // Threading
    QThread* m_workerThread = nullptr;
    QMutex m_metricsMutex;
};

// Service Worker Class
class ServiceWorker : public QObject
{
    Q_OBJECT

public:
    explicit ServiceWorker(QObject* parent = nullptr);
    ~ServiceWorker() override;

    bool isRunning() const { return m_running; }
    void setConfiguration(const QJsonObject& config);

public slots:
    void start();
    void stop();
    void pause();
    void resume();
    void processTask(const QString& taskId, const QJsonObject& taskData);

signals:
    void started();
    void stopped();
    void paused();
    void resumed();
    void taskCompleted(const QString& taskId, const QJsonObject& result);
    void errorOccurred(const QString& error);

private slots:
    void doWork();

private:
    bool m_running = false;
    bool m_paused = false;
    QTimer* m_workTimer = nullptr;
    QQueue<QPair<QString, QJsonObject>> m_taskQueue;
    QMutex m_queueMutex;
    QJsonObject m_config;
    int m_workInterval = 1000;
};

// Task Processor Class
class TaskProcessor : public QObject
{
    Q_OBJECT

public:
    explicit TaskProcessor(QObject* parent = nullptr);

    void addTask(const QString& taskId, const QJsonObject& taskData);
    void cancelTask(const QString& taskId);
    int queueSize() const;

signals:
    void taskCompleted(const QString& taskId, const QJsonObject& result);
    void taskFailed(const QString& taskId, const QString& error);

private slots:
    void processNextTask();

private:
    struct Task {
        QString id;
        QJsonObject data;
        QDateTime created;
        int priority = 0;
    };

    QQueue<Task> m_taskQueue;
    QTimer* m_processingTimer = nullptr;
    QMutex m_queueMutex;
    bool m_processing = false;

    QJsonObject processTask(const Task& task);
};

// File Monitor Class
class FileMonitor : public QObject
{
    Q_OBJECT

public:
    explicit FileMonitor(QObject* parent = nullptr);
    ~FileMonitor() override;

    void addPath(const QString& path);
    void removePath(const QString& path);
    QStringList watchedPaths() const;

signals:
    void fileChanged(const QString& path);
    void directoryChanged(const QString& path);

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& path);

private:
    QFileSystemWatcher* m_watcher = nullptr;
    QStringList m_watchedFiles;
    QStringList m_watchedDirectories;
};

// Network Monitor Class
class NetworkMonitor : public QObject
{
    Q_OBJECT

public:
    explicit NetworkMonitor(QObject* parent = nullptr);
    ~NetworkMonitor() override;

    void startMonitoring();
    void stopMonitoring();
    bool isConnected() const { return m_connected; }

signals:
    void statusChanged(bool connected);
    void responseReceived(const QJsonObject& response);

public slots:
    void checkConnection();
    void sendRequest(const QString& url, const QJsonObject& data = {});

private slots:
    void onNetworkReplyFinished();

private:
    QNetworkAccessManager* m_networkManager = nullptr;
    QTimer* m_checkTimer = nullptr;
    bool m_connected = false;
    QString m_testUrl = "https://httpbin.org/get";
};

// Service Status Enum
enum class ServiceStatus {
    Stopped,
    Starting,
    Running,
    Pausing,
    Paused,
    Stopping,
    Error
};
