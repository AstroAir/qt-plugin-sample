// PluginHelpers.h - Helper classes for plugin management
#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Forward declaration - actual implementation in PluginUpdateManager.h
class PluginUpdateManager;

// Plugin Metrics Collector
class PluginMetricsCollector : public QObject {
    Q_OBJECT

public:
    explicit PluginMetricsCollector(QObject* parent = nullptr);
    
    void startMonitoring(const QString& pluginName);
    void stopMonitoring(const QString& pluginName);
    void stopAllMonitoring();
    
    QJsonObject getMetrics(const QString& pluginName) const;
    QJsonObject getAllMetrics() const;
    
    void recordEvent(const QString& pluginName, const QString& event);
    void recordPerformance(const QString& pluginName, const QString& metric, double value);

signals:
    void metricsUpdated(const QString& pluginName);
    void performanceAlert(const QString& pluginName, const QString& metric, double value);

private slots:
    void collectMetrics();

private:
    QTimer* m_metricsTimer;
    QHash<QString, QJsonObject> m_pluginMetrics;
    QHash<QString, bool> m_monitoredPlugins;
    
    void initializeMetrics(const QString& pluginName);
    void updateCpuUsage(const QString& pluginName);
    void updateMemoryUsage(const QString& pluginName);
    void updateEventCounts(const QString& pluginName);
};

// Forward declaration - actual implementation in PluginDependencyManager.h
class PluginDependencyResolver;

// Plugin Configuration Manager
class PluginConfigurationManager : public QObject {
    Q_OBJECT

public:
    explicit PluginConfigurationManager(QObject* parent = nullptr);
    
    QJsonObject getConfiguration(const QString& pluginName) const;
    bool setConfiguration(const QString& pluginName, const QJsonObject& config);
    bool validateConfiguration(const QString& pluginName, const QJsonObject& config);
    
    QJsonObject getDefaultConfiguration(const QString& pluginName) const;
    void resetToDefaults(const QString& pluginName);
    
    void saveConfigurations();
    void loadConfigurations();

signals:
    void configurationChanged(const QString& pluginName);
    void configurationSaved(const QString& pluginName);
    void configurationLoaded(const QString& pluginName);

private:
    QHash<QString, QJsonObject> m_configurations;
    QHash<QString, QJsonObject> m_defaultConfigurations;
    QString m_configFilePath;
    
    QString getConfigFilePath() const;
    void ensureConfigDirectory();
};

// Plugin Backup Manager
class PluginBackupManager : public QObject {
    Q_OBJECT

public:
    explicit PluginBackupManager(QObject* parent = nullptr);
    
    bool createBackup(const QString& pluginName);
    bool restoreBackup(const QString& pluginName, const QString& backupId);
    bool deleteBackup(const QString& pluginName, const QString& backupId);
    
    QStringList getBackups(const QString& pluginName) const;
    QJsonObject getBackupInfo(const QString& pluginName, const QString& backupId) const;
    
    void setMaxBackups(int maxBackups) { m_maxBackups = maxBackups; }
    int maxBackups() const { return m_maxBackups; }

signals:
    void backupCreated(const QString& pluginName, const QString& backupId);
    void backupRestored(const QString& pluginName, const QString& backupId);
    void backupDeleted(const QString& pluginName, const QString& backupId);
    void backupFailed(const QString& pluginName, const QString& error);

private:
    QString m_backupDirectory;
    int m_maxBackups;
    
    QString getBackupDirectory() const;
    QString generateBackupId() const;
    void cleanupOldBackups(const QString& pluginName);
    bool copyPluginFiles(const QString& pluginName, const QString& destination);
};
