// PluginUpdateManager.h - Automatic Plugin Update Management System
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVersionNumber>
#include <QUrl>
#include <QDateTime>
#include <QSettings>
#include <QCryptographicHash>
#include <QMutex>
#include <QThread>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <memory>

// Forward declarations
class PluginUpdateChecker;
class PluginVersionManager;
class UpdateNotificationWidget;
class UpdateProgressDialog;
class RollbackManager;
class UpdateScheduler;

// Update types
enum class UpdateType {
    Major,          // Breaking changes, manual approval required
    Minor,          // New features, backward compatible
    Patch,          // Bug fixes, security patches
    Hotfix,         // Critical security fixes
    Beta,           // Beta/preview releases
    Development     // Development builds
};

// Update policies
enum class UpdatePolicy {
    Manual,         // User must manually approve all updates
    Automatic,      // Automatically install all updates
    SecurityOnly,   // Only install security updates automatically
    Stable,         // Only install stable releases automatically
    Prompt,         // Prompt user for each update
    Scheduled       // Install updates on schedule
};

// Update channels
enum class UpdateChannel {
    Stable,         // Stable releases only
    Beta,           // Beta and stable releases
    Development,    // All releases including development builds
    Custom          // Custom channel configuration
};

// Update status
enum class UpdateStatus {
    Unknown,
    UpToDate,
    UpdateAvailable,
    Downloading,
    Installing,
    Failed,
    Rollback,
    Scheduled
};

// Update information
struct UpdateInfo {
    QString pluginId;
    QString currentVersion;
    QString availableVersion;
    UpdateType type;
    QString description;
    QString changelog;
    QUrl downloadUrl;
    QString checksum;
    qint64 size;
    QDateTime releaseDate;
    QStringList dependencies;
    QStringList conflicts;
    bool isSecurityUpdate = false;
    bool isBreakingChange = false;
    bool requiresRestart = false;
    QJsonObject metadata;
    
    UpdateInfo() = default;
    UpdateInfo(const QString& id, const QString& current, const QString& available)
        : pluginId(id), currentVersion(current), availableVersion(available) {}
    
    QVersionNumber getCurrentVersionNumber() const { return QVersionNumber::fromString(currentVersion); }
    QVersionNumber getAvailableVersionNumber() const { return QVersionNumber::fromString(availableVersion); }
    bool isNewer() const { return getAvailableVersionNumber() > getCurrentVersionNumber(); }

    // Equality operator for QList operations
    bool operator==(const UpdateInfo& other) const {
        return pluginId == other.pluginId && availableVersion == other.availableVersion;
    }
};

// Update configuration
struct UpdateConfiguration {
    UpdatePolicy policy = UpdatePolicy::Prompt;
    UpdateChannel channel = UpdateChannel::Stable;
    bool checkOnStartup = true;
    bool autoDownload = false;
    bool autoInstall = false;
    bool notifyUpdates = true;
    bool includePrerelease = false;
    bool backupBeforeUpdate = true;
    int checkInterval = 24; // hours
    QTime scheduledTime = QTime(2, 0); // 2:00 AM
    QStringList excludedPlugins;
    QStringList priorityPlugins;
    QString backupDirectory;
    int maxBackups = 5;
    
    UpdateConfiguration() = default;
};

// Main update manager
class PluginUpdateManager : public QObject {
    Q_OBJECT

public:
    explicit PluginUpdateManager(QObject* parent = nullptr);
    ~PluginUpdateManager() override;

    // Configuration
    void setConfiguration(const UpdateConfiguration& config);
    UpdateConfiguration configuration() const;
    void setUpdatePolicy(UpdatePolicy policy);
    UpdatePolicy updatePolicy() const;
    void setUpdateChannel(UpdateChannel channel);
    UpdateChannel updateChannel() const;
    
    // Update checking
    void checkForUpdates();
    void checkForUpdates(const QString& pluginId);
    void checkForUpdates(const QStringList& pluginIds);
    QList<UpdateInfo> availableUpdates() const;
    UpdateInfo getUpdateInfo(const QString& pluginId) const;
    
    // Update operations
    void downloadUpdate(const QString& pluginId);
    void installUpdate(const QString& pluginId);
    void installAllUpdates();
    void scheduleUpdate(const QString& pluginId, const QDateTime& when);
    void cancelUpdate(const QString& pluginId);
    
    // Rollback operations
    void rollbackUpdate(const QString& pluginId);
    void rollbackToVersion(const QString& pluginId, const QString& version);
    QStringList getAvailableVersions(const QString& pluginId) const;
    
    // Status and information
    UpdateStatus getUpdateStatus(const QString& pluginId) const;
    QStringList getUpdatablePlugins() const;
    int getPendingUpdatesCount() const;
    bool hasSecurityUpdates() const;
    
    // Backup management
    void createBackup(const QString& pluginId);
    void restoreBackup(const QString& pluginId, const QString& backupId);
    QStringList getBackups(const QString& pluginId) const;
    void cleanupOldBackups();

signals:
    void updateCheckStarted();
    void updateCheckCompleted(int availableUpdates);
    void updateAvailable(const UpdateInfo& updateInfo);
    void updateDownloadStarted(const QString& pluginId);
    void updateDownloadProgress(const QString& pluginId, int percentage);
    void updateDownloadCompleted(const QString& pluginId);
    void updateInstallStarted(const QString& pluginId);
    void updateInstallProgress(const QString& pluginId, int percentage);
    void updateInstallCompleted(const QString& pluginId);
    void updateFailed(const QString& pluginId, const QString& error);
    void rollbackCompleted(const QString& pluginId);
    void backupCreated(const QString& pluginId, const QString& backupId);

public slots:
    void startPeriodicChecking();
    void stopPeriodicChecking();
    void showUpdateNotification();
    void showUpdateDialog();

private slots:
    void onPeriodicCheckTimer();
    void onUpdateCheckFinished();
    void onUpdateDownloadFinished();
    void onUpdateInstallFinished();

private:
    struct UpdateManagerPrivate;
    std::unique_ptr<UpdateManagerPrivate> d;
    
    void initializeManager();
    void loadConfiguration();
    void saveConfiguration();
    void setupPeriodicChecking();
    void processUpdateQueue();
    bool shouldAutoUpdate(const UpdateInfo& updateInfo);
    void notifyUpdateAvailable(const UpdateInfo& updateInfo);
    QString createBackupId() const;
};

// Update checker for fetching update information
class PluginUpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit PluginUpdateChecker(QObject* parent = nullptr);
    ~PluginUpdateChecker() override;

    // Check operations
    void checkForUpdates(const QStringList& pluginIds, UpdateChannel channel);
    void checkForUpdate(const QString& pluginId, UpdateChannel channel);
    void setUpdateSources(const QList<QUrl>& sources);
    QList<QUrl> updateSources() const;
    
    // Configuration
    void setTimeout(int seconds);
    int timeout() const;
    void setUserAgent(const QString& userAgent);
    QString userAgent() const;

signals:
    void checkStarted(const QStringList& pluginIds);
    void checkProgress(int completed, int total);
    void updateFound(const UpdateInfo& updateInfo);
    void checkCompleted(const QList<UpdateInfo>& updates);
    void checkFailed(const QString& error);

private slots:
    void onNetworkReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager* m_networkManager;
    QList<QUrl> m_updateSources;
    QMap<QNetworkReply*, QString> m_activeRequests;
    QList<UpdateInfo> m_foundUpdates;
    int m_timeout;
    QString m_userAgent;
    
    void processUpdateResponse(const QByteArray& data, const QString& pluginId);
    UpdateInfo parseUpdateInfo(const QJsonObject& json, const QString& pluginId);
    QUrl buildUpdateUrl(const QUrl& baseUrl, const QString& pluginId, UpdateChannel channel);
};

// Version manager for handling plugin versions
class PluginVersionManager : public QObject {
    Q_OBJECT

public:
    explicit PluginVersionManager(QObject* parent = nullptr);
    ~PluginVersionManager() override;

    // Version operations
    void registerVersion(const QString& pluginId, const QString& version, const QString& path);
    void unregisterVersion(const QString& pluginId, const QString& version);
    QStringList getVersions(const QString& pluginId) const;
    QString getCurrentVersion(const QString& pluginId) const;
    QString getVersionPath(const QString& pluginId, const QString& version) const;
    
    // Version comparison
    bool isNewerVersion(const QString& version1, const QString& version2) const;
    QString getLatestVersion(const QString& pluginId) const;
    QStringList getVersionsInRange(const QString& pluginId, const QString& minVersion, const QString& maxVersion) const;
    
    // Version validation
    bool isValidVersion(const QString& version) const;
    bool isCompatibleVersion(const QString& pluginId, const QString& version) const;
    QStringList validateVersionDependencies(const QString& pluginId, const QString& version) const;

signals:
    void versionRegistered(const QString& pluginId, const QString& version);
    void versionUnregistered(const QString& pluginId, const QString& version);
    void currentVersionChanged(const QString& pluginId, const QString& oldVersion, const QString& newVersion);

private:
    struct VersionInfo {
        QString version;
        QString path;
        QDateTime installDate;
        bool isActive = false;
    };
    
    QMap<QString, QList<VersionInfo>> m_versions;
    QMap<QString, QString> m_currentVersions;
    
    void loadVersionRegistry();
    void saveVersionRegistry();
    void cleanupOldVersions(const QString& pluginId);
};

// Update notification widget
class UpdateNotificationWidget : public QWidget {
    Q_OBJECT

public:
    explicit UpdateNotificationWidget(QWidget* parent = nullptr);
    ~UpdateNotificationWidget() override;

    // Notification management
    void showUpdateNotification(const UpdateInfo& updateInfo);
    void showMultipleUpdatesNotification(const QList<UpdateInfo>& updates);
    void hideNotification();
    
    // Configuration
    void setAutoHideTimeout(int seconds);
    int autoHideTimeout() const;
    void setPosition(Qt::Corner corner);
    Qt::Corner position() const;

signals:
    void updateRequested(const QString& pluginId);
    void updateAllRequested();
    void updateLaterRequested();
    void notificationDismissed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onAutoHideTimer();
    void onUpdateClicked();
    void onUpdateAllClicked();
    void onLaterClicked();
    void onCloseClicked();

private:
    QList<UpdateInfo> m_updates;
    QTimer* m_autoHideTimer;
    Qt::Corner m_position;
    int m_autoHideTimeout;
    bool m_isHovered;
    
    void setupUI();
    void updatePosition();
    void animateShow();
    void animateHide();
};

// Update progress dialog
class UpdateProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateProgressDialog(const QList<UpdateInfo>& updates, QWidget* parent = nullptr);
    ~UpdateProgressDialog() override;

    // Progress management
    void setCurrentUpdate(const QString& pluginId);
    void setOverallProgress(int percentage);
    void setCurrentProgress(int percentage);
    void setStatus(const QString& status);
    void addLogMessage(const QString& message);
    
    // Control
    void enableCancellation(bool enable);
    bool isCancellationEnabled() const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void updateCancelled();
    void updateCompleted();

private slots:
    void onCancelClicked();
    void onDetailsToggled();

private:
    QList<UpdateInfo> m_updates;
    QProgressBar* m_overallProgress;
    QProgressBar* m_currentProgress;
    QLabel* m_statusLabel;
    QLabel* m_currentUpdateLabel;
    QTextEdit* m_logView;
    QPushButton* m_cancelButton;
    QPushButton* m_detailsButton;
    bool m_cancellationEnabled;
    bool m_detailsVisible;
    
    void setupUI();
    void updateCurrentUpdateInfo();
};

// Rollback manager for handling update rollbacks
class RollbackManager : public QObject {
    Q_OBJECT

public:
    explicit RollbackManager(QObject* parent = nullptr);
    ~RollbackManager() override;

    // Backup operations
    QString createBackup(const QString& pluginId, const QString& version, const QString& path);
    bool restoreBackup(const QString& pluginId, const QString& backupId);
    bool deleteBackup(const QString& pluginId, const QString& backupId);
    QStringList getBackups(const QString& pluginId) const;
    
    // Rollback operations
    bool rollbackToVersion(const QString& pluginId, const QString& version);
    bool rollbackToPreviousVersion(const QString& pluginId);
    QStringList getRollbackHistory(const QString& pluginId) const;
    
    // Cleanup
    void cleanupOldBackups(int maxBackups = 5);
    void cleanupBackupsOlderThan(const QDateTime& cutoff);
    qint64 getBackupSize(const QString& pluginId) const;
    qint64 getTotalBackupSize() const;

signals:
    void backupCreated(const QString& pluginId, const QString& backupId);
    void backupRestored(const QString& pluginId, const QString& backupId);
    void backupDeleted(const QString& pluginId, const QString& backupId);
    void rollbackCompleted(const QString& pluginId, const QString& version);

private:
    struct BackupInfo {
        QString backupId;
        QString pluginId;
        QString version;
        QString backupPath;
        QDateTime createDate;
        qint64 size;
    };
    
    QString m_backupDirectory;
    QList<BackupInfo> m_backups;
    
    void loadBackupRegistry();
    void saveBackupRegistry();
    QString generateBackupId() const;
    bool compressDirectory(const QString& sourcePath, const QString& backupPath);
    bool extractBackup(const QString& backupPath, const QString& destinationPath);
};

// Update scheduler for scheduled updates
class UpdateScheduler : public QObject {
    Q_OBJECT

public:
    explicit UpdateScheduler(QObject* parent = nullptr);
    ~UpdateScheduler() override;

    // Schedule management
    void scheduleUpdate(const QString& pluginId, const QDateTime& when);
    void scheduleRecurringUpdate(const QString& pluginId, const QTime& time, int intervalDays = 1);
    void cancelScheduledUpdate(const QString& pluginId);
    void cancelAllScheduledUpdates();
    
    // Schedule queries
    QDateTime getNextScheduledUpdate(const QString& pluginId) const;
    QList<QString> getScheduledPlugins() const;
    bool hasScheduledUpdates() const;
    
    // Configuration
    void setMaintenanceWindow(const QTime& start, const QTime& end);
    QPair<QTime, QTime> maintenanceWindow() const;
    void setMaxConcurrentUpdates(int count);
    int maxConcurrentUpdates() const;

signals:
    void updateScheduled(const QString& pluginId, const QDateTime& when);
    void updateTriggered(const QString& pluginId);
    void scheduleChanged();

private slots:
    void onScheduleTimer();
    void checkScheduledUpdates();

private:
    struct ScheduledUpdate {
        QString pluginId;
        QDateTime scheduledTime;
        bool isRecurring = false;
        int intervalDays = 1;
        QTime recurringTime;
    };
    
    QList<ScheduledUpdate> m_scheduledUpdates;
    QTimer* m_scheduleTimer;
    QTime m_maintenanceStart;
    QTime m_maintenanceEnd;
    int m_maxConcurrentUpdates;
    
    void loadSchedule();
    void saveSchedule();
    bool isInMaintenanceWindow(const QDateTime& time) const;
    void processScheduledUpdate(const ScheduledUpdate& update);
};
