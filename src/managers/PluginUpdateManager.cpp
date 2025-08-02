// PluginUpdateManager.cpp - Implementation of Plugin Update Management System
#include "PluginUpdateManager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QMessageBox>
#include <QProgressDialog>
#include <QHeaderView>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QTreeWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <QLoggingCategory>
#include <QCryptographicHash>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QSettings>
#include <QRandomGenerator>

Q_LOGGING_CATEGORY(pluginUpdate, "plugin.update")

// PluginUpdateManager Private Implementation
struct PluginUpdateManager::UpdateManagerPrivate {
    UpdateConfiguration config;
    std::unique_ptr<PluginUpdateChecker> updateChecker;
    std::unique_ptr<PluginVersionManager> versionManager;
    std::unique_ptr<RollbackManager> rollbackManager;
    std::unique_ptr<UpdateScheduler> scheduler;
    std::unique_ptr<UpdateNotificationWidget> notificationWidget;
    
    QList<UpdateInfo> availableUpdates;
    QMap<QString, UpdateStatus> updateStatuses;
    QTimer* periodicTimer = nullptr;
    QMutex updateMutex;
    
    explicit UpdateManagerPrivate(PluginUpdateManager* parent) {
        updateChecker = std::make_unique<PluginUpdateChecker>(parent);
        versionManager = std::make_unique<PluginVersionManager>(parent);
        rollbackManager = std::make_unique<RollbackManager>(parent);
        scheduler = std::make_unique<UpdateScheduler>(parent);
        notificationWidget = std::make_unique<UpdateNotificationWidget>();
        
        periodicTimer = new QTimer(parent);
        periodicTimer->setSingleShot(false);
    }
};

// PluginUpdateManager Implementation
PluginUpdateManager::PluginUpdateManager(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<UpdateManagerPrivate>(this))
{
    initializeManager();
    loadConfiguration();
    
    // Connect update checker signals
    connect(d->updateChecker.get(), &PluginUpdateChecker::checkStarted,
            this, &PluginUpdateManager::updateCheckStarted);
    connect(d->updateChecker.get(), &PluginUpdateChecker::updateFound,
            this, [this](const UpdateInfo& updateInfo) {
                QMutexLocker locker(&d->updateMutex);
                d->availableUpdates.append(updateInfo);
                d->updateStatuses[updateInfo.pluginId] = UpdateStatus::UpdateAvailable;
                emit updateAvailable(updateInfo);
                
                if (shouldAutoUpdate(updateInfo)) {
                    downloadUpdate(updateInfo.pluginId);
                } else {
                    notifyUpdateAvailable(updateInfo);
                }
            });
    connect(d->updateChecker.get(), &PluginUpdateChecker::checkCompleted,
            this, [this](const QList<UpdateInfo>& updates) {
                emit updateCheckCompleted(updates.size());
                if (!updates.isEmpty() && d->config.notifyUpdates) {
                    d->notificationWidget->showMultipleUpdatesNotification(updates);
                }
            });
    
    // Connect notification widget signals
    connect(d->notificationWidget.get(), &UpdateNotificationWidget::updateRequested,
            this, &PluginUpdateManager::installUpdate);
    connect(d->notificationWidget.get(), &UpdateNotificationWidget::updateAllRequested,
            this, &PluginUpdateManager::installAllUpdates);
    
    // Connect scheduler signals
    connect(d->scheduler.get(), &UpdateScheduler::updateTriggered,
            this, &PluginUpdateManager::installUpdate);
    
    // Connect periodic timer
    connect(d->periodicTimer, &QTimer::timeout,
            this, &PluginUpdateManager::onPeriodicCheckTimer);
    
    // Setup periodic checking if enabled
    if (d->config.checkOnStartup) {
        QTimer::singleShot(5000, this, [this]() { checkForUpdates(); }); // Check after 5 seconds
    }
    
    setupPeriodicChecking();
    
    qCInfo(pluginUpdate) << "PluginUpdateManager initialized";
}

PluginUpdateManager::~PluginUpdateManager() {
    saveConfiguration();
    qCInfo(pluginUpdate) << "PluginUpdateManager destroyed";
}

void PluginUpdateManager::setConfiguration(const UpdateConfiguration& config) {
    d->config = config;
    
    // Update components with new configuration
    d->rollbackManager->cleanupOldBackups(config.maxBackups);
    
    // Restart periodic checking with new interval
    setupPeriodicChecking();
    
    saveConfiguration();
    qCInfo(pluginUpdate) << "Update configuration changed";
}

UpdateConfiguration PluginUpdateManager::configuration() const {
    return d->config;
}

void PluginUpdateManager::setUpdatePolicy(UpdatePolicy policy) {
    d->config.policy = policy;
    saveConfiguration();
}

UpdatePolicy PluginUpdateManager::updatePolicy() const {
    return d->config.policy;
}

void PluginUpdateManager::setUpdateChannel(UpdateChannel channel) {
    d->config.channel = channel;
    saveConfiguration();
}

UpdateChannel PluginUpdateManager::updateChannel() const {
    return d->config.channel;
}

void PluginUpdateManager::checkForUpdates() {
    // Get list of all installed plugins
    QStringList pluginIds; // TODO: Get from plugin manager
    checkForUpdates(pluginIds);
}

void PluginUpdateManager::checkForUpdates(const QString& pluginId) {
    checkForUpdates(QStringList() << pluginId);
}

void PluginUpdateManager::checkForUpdates(const QStringList& pluginIds) {
    if (pluginIds.isEmpty()) {
        qCWarning(pluginUpdate) << "No plugins to check for updates";
        return;
    }
    
    // Clear previous results
    {
        QMutexLocker locker(&d->updateMutex);
        d->availableUpdates.clear();
        for (const QString& pluginId : pluginIds) {
            d->updateStatuses[pluginId] = UpdateStatus::Unknown;
        }
    }
    
    // Start update check
    d->updateChecker->checkForUpdates(pluginIds, d->config.channel);
    
    qCInfo(pluginUpdate) << "Checking for updates for" << pluginIds.size() << "plugins";
}

QList<UpdateInfo> PluginUpdateManager::availableUpdates() const {
    QMutexLocker locker(&d->updateMutex);
    return d->availableUpdates;
}

UpdateInfo PluginUpdateManager::getUpdateInfo(const QString& pluginId) const {
    QMutexLocker locker(&d->updateMutex);
    for (const UpdateInfo& update : d->availableUpdates) {
        if (update.pluginId == pluginId) {
            return update;
        }
    }
    return UpdateInfo();
}

void PluginUpdateManager::downloadUpdate(const QString& pluginId) {
    UpdateInfo updateInfo = getUpdateInfo(pluginId);
    if (updateInfo.pluginId.isEmpty()) {
        emit updateFailed(pluginId, "Update information not found");
        return;
    }
    
    d->updateStatuses[pluginId] = UpdateStatus::Downloading;
    emit updateDownloadStarted(pluginId);
    
    // TODO: Implement actual download logic
    // For now, simulate download
    QTimer::singleShot(2000, this, [this, pluginId]() {
        d->updateStatuses[pluginId] = UpdateStatus::UpdateAvailable;
        emit updateDownloadCompleted(pluginId);
        
        // Auto-install if configured
        if (d->config.autoInstall) {
            installUpdate(pluginId);
        }
    });
    
    qCInfo(pluginUpdate) << "Started downloading update for plugin:" << pluginId;
}

void PluginUpdateManager::installUpdate(const QString& pluginId) {
    UpdateInfo updateInfo = getUpdateInfo(pluginId);
    if (updateInfo.pluginId.isEmpty()) {
        emit updateFailed(pluginId, "Update information not found");
        return;
    }
    
    // Create backup if configured
    if (d->config.backupBeforeUpdate) {
        createBackup(pluginId);
    }
    
    d->updateStatuses[pluginId] = UpdateStatus::Installing;
    emit updateInstallStarted(pluginId);
    
    // TODO: Implement actual installation logic
    // For now, simulate installation
    QTimer::singleShot(3000, this, [this, pluginId, updateInfo]() {
        // Register new version
        d->versionManager->registerVersion(pluginId, updateInfo.availableVersion, "");
        
        // Remove from available updates
        {
            QMutexLocker locker(&d->updateMutex);
            d->availableUpdates.removeAll(updateInfo);
            d->updateStatuses[pluginId] = UpdateStatus::UpToDate;
        }
        
        emit updateInstallCompleted(pluginId);
        qCInfo(pluginUpdate) << "Successfully updated plugin:" << pluginId << "to version:" << updateInfo.availableVersion;
    });
    
    qCInfo(pluginUpdate) << "Started installing update for plugin:" << pluginId;
}

void PluginUpdateManager::installAllUpdates() {
    QStringList pluginIds;
    {
        QMutexLocker locker(&d->updateMutex);
        for (const UpdateInfo& update : d->availableUpdates) {
            pluginIds.append(update.pluginId);
        }
    }
    
    if (pluginIds.isEmpty()) {
        qCInfo(pluginUpdate) << "No updates available to install";
        return;
    }
    
    // Show progress dialog
    auto progressDialog = new UpdateProgressDialog(d->availableUpdates, qobject_cast<QWidget*>(parent()));
    connect(progressDialog, &UpdateProgressDialog::updateCancelled,
            this, [this, pluginIds]() {
                for (const QString& pluginId : pluginIds) {
                    cancelUpdate(pluginId);
                }
            });
    
    progressDialog->show();
    
    // Install updates sequentially
    for (const QString& pluginId : pluginIds) {
        installUpdate(pluginId);
    }
    
    qCInfo(pluginUpdate) << "Installing all available updates:" << pluginIds.size();
}

void PluginUpdateManager::scheduleUpdate(const QString& pluginId, const QDateTime& when) {
    d->scheduler->scheduleUpdate(pluginId, when);
    d->updateStatuses[pluginId] = UpdateStatus::Scheduled;
    
    qCInfo(pluginUpdate) << "Scheduled update for plugin:" << pluginId << "at:" << when;
}

void PluginUpdateManager::cancelUpdate(const QString& pluginId) {
    UpdateStatus status = d->updateStatuses.value(pluginId, UpdateStatus::Unknown);
    
    if (status == UpdateStatus::Downloading || status == UpdateStatus::Installing) {
        // TODO: Cancel ongoing operation
        d->updateStatuses[pluginId] = UpdateStatus::UpdateAvailable;
        qCInfo(pluginUpdate) << "Cancelled update for plugin:" << pluginId;
    } else if (status == UpdateStatus::Scheduled) {
        d->scheduler->cancelScheduledUpdate(pluginId);
        d->updateStatuses[pluginId] = UpdateStatus::UpdateAvailable;
        qCInfo(pluginUpdate) << "Cancelled scheduled update for plugin:" << pluginId;
    }
}

void PluginUpdateManager::rollbackUpdate(const QString& pluginId) {
    if (d->rollbackManager->rollbackToPreviousVersion(pluginId)) {
        d->updateStatuses[pluginId] = UpdateStatus::UpToDate;
        emit rollbackCompleted(pluginId);
        qCInfo(pluginUpdate) << "Rolled back plugin:" << pluginId;
    } else {
        emit updateFailed(pluginId, "Rollback failed");
        qCWarning(pluginUpdate) << "Failed to rollback plugin:" << pluginId;
    }
}

void PluginUpdateManager::rollbackToVersion(const QString& pluginId, const QString& version) {
    if (d->rollbackManager->rollbackToVersion(pluginId, version)) {
        d->updateStatuses[pluginId] = UpdateStatus::UpToDate;
        emit rollbackCompleted(pluginId);
        qCInfo(pluginUpdate) << "Rolled back plugin:" << pluginId << "to version:" << version;
    } else {
        emit updateFailed(pluginId, "Rollback to version failed");
        qCWarning(pluginUpdate) << "Failed to rollback plugin:" << pluginId << "to version:" << version;
    }
}

QStringList PluginUpdateManager::getAvailableVersions(const QString& pluginId) const {
    return d->versionManager->getVersions(pluginId);
}

UpdateStatus PluginUpdateManager::getUpdateStatus(const QString& pluginId) const {
    return d->updateStatuses.value(pluginId, UpdateStatus::Unknown);
}

QStringList PluginUpdateManager::getUpdatablePlugins() const {
    QStringList updatable;
    QMutexLocker locker(&d->updateMutex);
    
    for (const UpdateInfo& update : d->availableUpdates) {
        updatable.append(update.pluginId);
    }
    
    return updatable;
}

int PluginUpdateManager::getPendingUpdatesCount() const {
    QMutexLocker locker(&d->updateMutex);
    return d->availableUpdates.size();
}

bool PluginUpdateManager::hasSecurityUpdates() const {
    QMutexLocker locker(&d->updateMutex);
    
    for (const UpdateInfo& update : d->availableUpdates) {
        if (update.isSecurityUpdate) {
            return true;
        }
    }
    
    return false;
}

void PluginUpdateManager::createBackup(const QString& pluginId) {
    QString currentVersion = d->versionManager->getCurrentVersion(pluginId);
    QString versionPath = d->versionManager->getVersionPath(pluginId, currentVersion);
    
    if (!versionPath.isEmpty()) {
        QString backupId = d->rollbackManager->createBackup(pluginId, currentVersion, versionPath);
        if (!backupId.isEmpty()) {
            emit backupCreated(pluginId, backupId);
            qCInfo(pluginUpdate) << "Created backup for plugin:" << pluginId << "backup ID:" << backupId;
        }
    }
}

void PluginUpdateManager::restoreBackup(const QString& pluginId, const QString& backupId) {
    if (d->rollbackManager->restoreBackup(pluginId, backupId)) {
        d->updateStatuses[pluginId] = UpdateStatus::UpToDate;
        qCInfo(pluginUpdate) << "Restored backup for plugin:" << pluginId << "backup ID:" << backupId;
    } else {
        qCWarning(pluginUpdate) << "Failed to restore backup for plugin:" << pluginId << "backup ID:" << backupId;
    }
}

QStringList PluginUpdateManager::getBackups(const QString& pluginId) const {
    return d->rollbackManager->getBackups(pluginId);
}

void PluginUpdateManager::cleanupOldBackups() {
    d->rollbackManager->cleanupOldBackups(d->config.maxBackups);
    qCInfo(pluginUpdate) << "Cleaned up old backups";
}

void PluginUpdateManager::startPeriodicChecking() {
    if (d->config.checkInterval > 0) {
        d->periodicTimer->start(d->config.checkInterval * 60 * 60 * 1000); // Convert hours to milliseconds
        qCInfo(pluginUpdate) << "Started periodic update checking every" << d->config.checkInterval << "hours";
    }
}

void PluginUpdateManager::stopPeriodicChecking() {
    d->periodicTimer->stop();
    qCInfo(pluginUpdate) << "Stopped periodic update checking";
}

void PluginUpdateManager::showUpdateNotification() {
    if (!d->availableUpdates.isEmpty()) {
        d->notificationWidget->showMultipleUpdatesNotification(d->availableUpdates);
    }
}

void PluginUpdateManager::showUpdateDialog() {
    // TODO: Implement update dialog
    qCInfo(pluginUpdate) << "Show update dialog requested";
}

void PluginUpdateManager::onPeriodicCheckTimer() {
    qCInfo(pluginUpdate) << "Periodic update check triggered";
    checkForUpdates();
}

void PluginUpdateManager::onUpdateCheckFinished() {
    qCInfo(pluginUpdate) << "Update check finished";
}

void PluginUpdateManager::onUpdateDownloadFinished() {
    qCInfo(pluginUpdate) << "Update download finished";
}

void PluginUpdateManager::onUpdateInstallFinished() {
    qCInfo(pluginUpdate) << "Update install finished";
}

void PluginUpdateManager::initializeManager() {
    // Set default configuration
    d->config.policy = UpdatePolicy::Prompt;
    d->config.channel = UpdateChannel::Stable;
    d->config.checkOnStartup = true;
    d->config.notifyUpdates = true;
    d->config.backupBeforeUpdate = true;
    d->config.checkInterval = 24; // 24 hours
    d->config.maxBackups = 5;

    // Set default backup directory
    d->config.backupDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups";
    QDir().mkpath(d->config.backupDirectory);

    qCInfo(pluginUpdate) << "Update manager initialized";
}

void PluginUpdateManager::loadConfiguration() {
    QSettings settings;
    settings.beginGroup("PluginUpdates");

    d->config.policy = static_cast<UpdatePolicy>(settings.value("policy", static_cast<int>(UpdatePolicy::Prompt)).toInt());
    d->config.channel = static_cast<UpdateChannel>(settings.value("channel", static_cast<int>(UpdateChannel::Stable)).toInt());
    d->config.checkOnStartup = settings.value("checkOnStartup", true).toBool();
    d->config.autoDownload = settings.value("autoDownload", false).toBool();
    d->config.autoInstall = settings.value("autoInstall", false).toBool();
    d->config.notifyUpdates = settings.value("notifyUpdates", true).toBool();
    d->config.includePrerelease = settings.value("includePrerelease", false).toBool();
    d->config.backupBeforeUpdate = settings.value("backupBeforeUpdate", true).toBool();
    d->config.checkInterval = settings.value("checkInterval", 24).toInt();
    d->config.scheduledTime = settings.value("scheduledTime", QTime(2, 0)).toTime();
    d->config.excludedPlugins = settings.value("excludedPlugins").toStringList();
    d->config.priorityPlugins = settings.value("priorityPlugins").toStringList();
    d->config.backupDirectory = settings.value("backupDirectory",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/backups").toString();
    d->config.maxBackups = settings.value("maxBackups", 5).toInt();

    settings.endGroup();

    qCInfo(pluginUpdate) << "Loaded update configuration";
}

void PluginUpdateManager::saveConfiguration() {
    QSettings settings;
    settings.beginGroup("PluginUpdates");

    settings.setValue("policy", static_cast<int>(d->config.policy));
    settings.setValue("channel", static_cast<int>(d->config.channel));
    settings.setValue("checkOnStartup", d->config.checkOnStartup);
    settings.setValue("autoDownload", d->config.autoDownload);
    settings.setValue("autoInstall", d->config.autoInstall);
    settings.setValue("notifyUpdates", d->config.notifyUpdates);
    settings.setValue("includePrerelease", d->config.includePrerelease);
    settings.setValue("backupBeforeUpdate", d->config.backupBeforeUpdate);
    settings.setValue("checkInterval", d->config.checkInterval);
    settings.setValue("scheduledTime", d->config.scheduledTime);
    settings.setValue("excludedPlugins", d->config.excludedPlugins);
    settings.setValue("priorityPlugins", d->config.priorityPlugins);
    settings.setValue("backupDirectory", d->config.backupDirectory);
    settings.setValue("maxBackups", d->config.maxBackups);

    settings.endGroup();

    qCInfo(pluginUpdate) << "Saved update configuration";
}

void PluginUpdateManager::setupPeriodicChecking() {
    if (d->config.checkInterval > 0) {
        d->periodicTimer->setInterval(d->config.checkInterval * 60 * 60 * 1000); // Convert hours to milliseconds
        if (!d->periodicTimer->isActive()) {
            d->periodicTimer->start();
        }
    } else {
        d->periodicTimer->stop();
    }
}

void PluginUpdateManager::processUpdateQueue() {
    // Process any queued updates
    // This would be used for batch processing updates
    qCInfo(pluginUpdate) << "Processing update queue";
}

bool PluginUpdateManager::shouldAutoUpdate(const UpdateInfo& updateInfo) {
    // Check if plugin is excluded
    if (d->config.excludedPlugins.contains(updateInfo.pluginId)) {
        return false;
    }

    // Check update policy
    switch (d->config.policy) {
        case UpdatePolicy::Manual:
            return false;
        case UpdatePolicy::Automatic:
            return true;
        case UpdatePolicy::SecurityOnly:
            return updateInfo.isSecurityUpdate;
        case UpdatePolicy::Stable:
            return updateInfo.type != UpdateType::Beta && updateInfo.type != UpdateType::Development;
        case UpdatePolicy::Prompt:
            return false; // Will show notification instead
        case UpdatePolicy::Scheduled:
            // Schedule the update instead of installing immediately
            QDateTime scheduledTime = QDateTime::currentDateTime().addSecs(60); // Schedule for 1 minute later
            scheduleUpdate(updateInfo.pluginId, scheduledTime);
            return false;
    }

    return false;
}

void PluginUpdateManager::notifyUpdateAvailable(const UpdateInfo& updateInfo) {
    if (d->config.notifyUpdates) {
        d->notificationWidget->showUpdateNotification(updateInfo);
    }
}

QString PluginUpdateManager::createBackupId() const {
    return QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + "_" +
           QString::number(QRandomGenerator::global()->bounded(1000, 9999));
}

// PluginUpdateChecker Implementation
PluginUpdateChecker::PluginUpdateChecker(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_timeout(30000) // 30 seconds
    , m_userAgent("QtPluginSystem/1.0")
{
    // Set default update sources
    m_updateSources.append(QUrl("https://updates.example.com/api/v1"));
    m_updateSources.append(QUrl("https://plugins.example.com/updates"));

    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &PluginUpdateChecker::onNetworkReplyFinished);
}

PluginUpdateChecker::~PluginUpdateChecker() = default;

void PluginUpdateChecker::checkForUpdates(const QStringList& pluginIds, UpdateChannel channel) {
    if (pluginIds.isEmpty()) {
        emit checkCompleted(QList<UpdateInfo>());
        return;
    }

    m_foundUpdates.clear();
    emit checkStarted(pluginIds);

    int totalRequests = 0;
    for (const QUrl& source : m_updateSources) {
        for (const QString& pluginId : pluginIds) {
            QUrl updateUrl = buildUpdateUrl(source, pluginId, channel);

            QNetworkRequest request(updateUrl);
            request.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
            request.setRawHeader("Accept", "application/json");

            QNetworkReply* reply = m_networkManager->get(request);
            reply->setProperty("pluginId", pluginId);

            m_activeRequests[reply] = pluginId;
            totalRequests++;
        }
    }

    qCInfo(pluginUpdate) << "Started checking for updates:" << totalRequests << "requests";
}

void PluginUpdateChecker::checkForUpdate(const QString& pluginId, UpdateChannel channel) {
    checkForUpdates(QStringList() << pluginId, channel);
}

void PluginUpdateChecker::setUpdateSources(const QList<QUrl>& sources) {
    m_updateSources = sources;
}

QList<QUrl> PluginUpdateChecker::updateSources() const {
    return m_updateSources;
}

void PluginUpdateChecker::setTimeout(int seconds) {
    m_timeout = seconds * 1000;
}

int PluginUpdateChecker::timeout() const {
    return m_timeout / 1000;
}

void PluginUpdateChecker::setUserAgent(const QString& userAgent) {
    m_userAgent = userAgent;
}

QString PluginUpdateChecker::userAgent() const {
    return m_userAgent;
}

void PluginUpdateChecker::onNetworkReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    QString pluginId = m_activeRequests.take(reply);

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        processUpdateResponse(data, pluginId);
    } else {
        qCWarning(pluginUpdate) << "Update check failed for plugin:" << pluginId << reply->errorString();
    }

    reply->deleteLater();

    // Check if all requests completed
    if (m_activeRequests.isEmpty()) {
        emit checkCompleted(m_foundUpdates);
    }
}

void PluginUpdateChecker::onNetworkError(QNetworkReply::NetworkError error) {
    Q_UNUSED(error)
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        QString pluginId = m_activeRequests.value(reply);
        qCWarning(pluginUpdate) << "Network error checking updates for plugin:" << pluginId;
    }
}

void PluginUpdateChecker::processUpdateResponse(const QByteArray& data, const QString& pluginId) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qCWarning(pluginUpdate) << "Failed to parse update response for plugin:" << pluginId << error.errorString();
        return;
    }

    QJsonObject json = doc.object();
    UpdateInfo updateInfo = parseUpdateInfo(json, pluginId);

    if (!updateInfo.pluginId.isEmpty() && updateInfo.isNewer()) {
        m_foundUpdates.append(updateInfo);
        emit updateFound(updateInfo);
    }
}

UpdateInfo PluginUpdateChecker::parseUpdateInfo(const QJsonObject& json, const QString& pluginId) {
    UpdateInfo info;
    info.pluginId = pluginId;
    info.availableVersion = json["version"].toString();
    info.description = json["description"].toString();
    info.changelog = json["changelog"].toString();
    info.downloadUrl = QUrl(json["downloadUrl"].toString());
    info.checksum = json["checksum"].toString();
    info.size = json["size"].toVariant().toLongLong();
    info.releaseDate = QDateTime::fromString(json["releaseDate"].toString(), Qt::ISODate);
    info.isSecurityUpdate = json["isSecurityUpdate"].toBool();
    info.isBreakingChange = json["isBreakingChange"].toBool();
    info.requiresRestart = json["requiresRestart"].toBool();

    QString typeStr = json["type"].toString().toLower();
    if (typeStr == "major") info.type = UpdateType::Major;
    else if (typeStr == "minor") info.type = UpdateType::Minor;
    else if (typeStr == "patch") info.type = UpdateType::Patch;
    else if (typeStr == "hotfix") info.type = UpdateType::Hotfix;
    else if (typeStr == "beta") info.type = UpdateType::Beta;
    else if (typeStr == "development") info.type = UpdateType::Development;
    else info.type = UpdateType::Patch;

    // TODO: Get current version from plugin manager
    info.currentVersion = "1.0.0"; // Placeholder

    return info;
}

QUrl PluginUpdateChecker::buildUpdateUrl(const QUrl& baseUrl, const QString& pluginId, UpdateChannel channel) {
    QUrl url = baseUrl;
    url.setPath(url.path() + "/plugins/" + pluginId + "/updates");

    QUrlQuery query;
    query.addQueryItem("channel", QString::number(static_cast<int>(channel)));
    url.setQuery(query);

    return url;
}
