// PluginDependencyManager.cpp - Implementation of Plugin Dependency Management System
#include "PluginDependencyManager.h"
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

Q_LOGGING_CATEGORY(pluginDependency, "plugin.dependency")

// DependencyConstraint Implementation
bool DependencyConstraint::isCompatible(const QString& targetVersion) const {
    if (version.isEmpty()) {
        return true; // No version constraint
    }
    
    QVersionNumber target = QVersionNumber::fromString(targetVersion);
    QVersionNumber required = QVersionNumber::fromString(version);
    
    if (versionRange.isEmpty()) {
        // Exact version match
        return target == required;
    }
    
    // Parse version range (e.g., ">=1.0.0,<2.0.0")
    QStringList ranges = versionRange.split(',');
    for (const QString& range : ranges) {
        QString trimmedRange = range.trimmed();
        if (trimmedRange.startsWith(">=")) {
            QVersionNumber minVersion = QVersionNumber::fromString(trimmedRange.mid(2));
            if (target < minVersion) return false;
        } else if (trimmedRange.startsWith("<=")) {
            QVersionNumber maxVersion = QVersionNumber::fromString(trimmedRange.mid(2));
            if (target > maxVersion) return false;
        } else if (trimmedRange.startsWith(">")) {
            QVersionNumber minVersion = QVersionNumber::fromString(trimmedRange.mid(1));
            if (target <= minVersion) return false;
        } else if (trimmedRange.startsWith("<")) {
            QVersionNumber maxVersion = QVersionNumber::fromString(trimmedRange.mid(1));
            if (target >= maxVersion) return false;
        } else if (trimmedRange.startsWith("=")) {
            QVersionNumber exactVersion = QVersionNumber::fromString(trimmedRange.mid(1));
            if (target != exactVersion) return false;
        }
    }
    
    return true;
}

bool DependencyConstraint::isPlatformCompatible() const {
    if (platforms.isEmpty()) {
        return true; // No platform constraint
    }
    
    QString currentPlatform;
#ifdef Q_OS_WIN
    currentPlatform = "windows";
#elif defined(Q_OS_MAC)
    currentPlatform = "macos";
#elif defined(Q_OS_LINUX)
    currentPlatform = "linux";
#else
    currentPlatform = "unknown";
#endif
    
    return platforms.contains(currentPlatform, Qt::CaseInsensitive);
}

QVersionNumber DependencyConstraint::getMinVersion() const {
    if (versionRange.isEmpty()) {
        return QVersionNumber::fromString(version);
    }
    
    QStringList ranges = versionRange.split(',');
    QVersionNumber minVersion;
    
    for (const QString& range : ranges) {
        QString trimmedRange = range.trimmed();
        if (trimmedRange.startsWith(">=") || trimmedRange.startsWith(">")) {
            QString versionStr = trimmedRange.startsWith(">=") ? trimmedRange.mid(2) : trimmedRange.mid(1);
            QVersionNumber rangeVersion = QVersionNumber::fromString(versionStr);
            if (minVersion.isNull() || rangeVersion > minVersion) {
                minVersion = rangeVersion;
            }
        }
    }
    
    return minVersion.isNull() ? QVersionNumber::fromString(version) : minVersion;
}

QVersionNumber DependencyConstraint::getMaxVersion() const {
    if (versionRange.isEmpty()) {
        return QVersionNumber::fromString(version);
    }
    
    QStringList ranges = versionRange.split(',');
    QVersionNumber maxVersion;
    
    for (const QString& range : ranges) {
        QString trimmedRange = range.trimmed();
        if (trimmedRange.startsWith("<=") || trimmedRange.startsWith("<")) {
            QString versionStr = trimmedRange.startsWith("<=") ? trimmedRange.mid(2) : trimmedRange.mid(1);
            QVersionNumber rangeVersion = QVersionNumber::fromString(versionStr);
            if (maxVersion.isNull() || rangeVersion < maxVersion) {
                maxVersion = rangeVersion;
            }
        }
    }
    
    return maxVersion.isNull() ? QVersionNumber::fromString(version) : maxVersion;
}

// PluginDependencyManager Private Implementation
struct PluginDependencyManager::DependencyManagerPrivate {
    QMap<QString, PluginRepository*> repositories;
    QList<PluginPackage> installedPackages;
    QList<PluginPackage> availablePackages;
    std::unique_ptr<PluginDependencyResolver> resolver;
    std::unique_ptr<PluginDownloader> downloader;
    
    QString installDirectory;
    QString cacheDirectory;
    ResolutionStrategy strategy = ResolutionStrategy::Stable;
    int maxConcurrentDownloads = 3;
    
    QTimer* refreshTimer = nullptr;
    QMutex packageMutex;
    
    explicit DependencyManagerPrivate(PluginDependencyManager* parent) {
        resolver = std::make_unique<PluginDependencyResolver>(parent);
        downloader = std::make_unique<PluginDownloader>(parent);
        refreshTimer = new QTimer(parent);
        refreshTimer->setSingleShot(true);
        refreshTimer->setInterval(300000); // 5 minutes
        
        // Set default directories
        installDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins";
        cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/plugin-cache";
    }
};

// PluginDependencyManager Implementation
PluginDependencyManager::PluginDependencyManager(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<DependencyManagerPrivate>(this))
{
    initializeManager();
    
    // Connect resolver signals
    connect(d->resolver.get(), &PluginDependencyResolver::resolutionCompleted,
            this, [this](const ResolutionResult& result) {
                emit dependencyResolved("", result);
                if (result.hasConflicts()) {
                    emit conflictDetected(result.conflicts);
                }
            });
    
    // Connect downloader signals
    connect(d->downloader.get(), &PluginDownloader::downloadProgress,
            this, [this](const QString& packageId, qint64 received, qint64 total) {
                int percentage = total > 0 ? (received * 100) / total : 0;
                emit installationProgress(packageId, percentage);
            });
    
    connect(d->downloader.get(), &PluginDownloader::downloadFinished,
            this, &PluginDependencyManager::onPackageDownloadFinished);
    
    connect(d->downloader.get(), &PluginDownloader::downloadFailed,
            this, [this](const QString& packageId, const QString& error) {
                emit installationFailed(packageId, error);
            });
    
    // Setup refresh timer
    connect(d->refreshTimer, &QTimer::timeout, this, &PluginDependencyManager::refreshRepositories);
    
    qCInfo(pluginDependency) << "PluginDependencyManager initialized";
}

PluginDependencyManager::~PluginDependencyManager() {
    saveInstalledPackages();
    qCInfo(pluginDependency) << "PluginDependencyManager destroyed";
}

void PluginDependencyManager::addRepository(const QString& name, const QUrl& url) {
    if (d->repositories.contains(name)) {
        qCWarning(pluginDependency) << "Repository already exists:" << name;
        return;
    }
    
    auto repository = new PluginRepository(name, url, this);
    d->repositories[name] = repository;
    
    // Connect repository signals
    connect(repository, &PluginRepository::refreshFinished,
            this, &PluginDependencyManager::onRepositoryRefreshFinished);
    connect(repository, &PluginRepository::packageListUpdated,
            this, [this, name]() {
                emit repositoryRefreshed(name);
                updateAvailablePackages();
            });
    
    // Refresh the new repository
    repository->refresh();
    
    qCInfo(pluginDependency) << "Added repository:" << name << url;
}

void PluginDependencyManager::removeRepository(const QString& name) {
    if (!d->repositories.contains(name)) {
        return;
    }
    
    PluginRepository* repository = d->repositories.take(name);
    repository->deleteLater();
    
    updateAvailablePackages();
    qCInfo(pluginDependency) << "Removed repository:" << name;
}

QStringList PluginDependencyManager::repositories() const {
    return d->repositories.keys();
}

void PluginDependencyManager::refreshRepositories() {
    for (PluginRepository* repository : d->repositories.values()) {
        repository->refresh();
    }
    
    qCInfo(pluginDependency) << "Refreshing all repositories";
}

QList<PluginPackage> PluginDependencyManager::availablePackages() const {
    QMutexLocker locker(&d->packageMutex);
    return d->availablePackages;
}

QList<PluginPackage> PluginDependencyManager::installedPackages() const {
    QMutexLocker locker(&d->packageMutex);
    return d->installedPackages;
}

PluginPackage PluginDependencyManager::findPackage(const QString& id) const {
    QMutexLocker locker(&d->packageMutex);
    
    // First check installed packages
    for (const PluginPackage& package : d->installedPackages) {
        if (package.id == id) {
            return package;
        }
    }
    
    // Then check available packages
    for (const PluginPackage& package : d->availablePackages) {
        if (package.id == id) {
            return package;
        }
    }
    
    return PluginPackage(); // Not found
}

QList<PluginPackage> PluginDependencyManager::searchPackages(const QString& query) const {
    QList<PluginPackage> results;
    QMutexLocker locker(&d->packageMutex);
    
    QString lowerQuery = query.toLower();
    
    for (const PluginPackage& package : d->availablePackages) {
        if (package.name.toLower().contains(lowerQuery) ||
            package.description.toLower().contains(lowerQuery) ||
            package.tags.join(" ").toLower().contains(lowerQuery)) {
            results.append(package);
        }
    }
    
    return results;
}

ResolutionResult PluginDependencyManager::resolveDependencies(const QString& pluginId) {
    PluginPackage package = findPackage(pluginId);
    if (package.id.isEmpty()) {
        ResolutionResult result;
        result.success = false;
        result.errorMessage = "Package not found: " + pluginId;
        return result;
    }
    
    return resolveDependencies(package.dependencies);
}

ResolutionResult PluginDependencyManager::resolveDependencies(const QList<DependencyConstraint>& constraints) {
    d->resolver->setStrategy(d->strategy);
    return d->resolver->resolve(constraints, d->availablePackages, d->installedPackages);
}

void PluginDependencyManager::setResolutionStrategy(ResolutionStrategy strategy) {
    d->strategy = strategy;
    qCInfo(pluginDependency) << "Resolution strategy changed to:" << static_cast<int>(strategy);
}

ResolutionStrategy PluginDependencyManager::resolutionStrategy() const {
    return d->strategy;
}

void PluginDependencyManager::installPackage(const QString& packageId) {
    PluginPackage package = findPackage(packageId);
    if (package.id.isEmpty()) {
        emit installationFailed(packageId, "Package not found");
        return;
    }
    
    if (package.isInstalled()) {
        emit installationFailed(packageId, "Package already installed");
        return;
    }
    
    // Resolve dependencies first
    ResolutionResult result = resolveDependencies(packageId);
    if (!result.success) {
        emit installationFailed(packageId, result.errorMessage);
        return;
    }
    
    // Install dependencies if needed
    if (!result.toInstall.isEmpty()) {
        installDependencies(result);
    } else {
        // Install the package directly
        installPackageInternal(package);
    }
}

void PluginDependencyManager::updatePackage(const QString& packageId) {
    PluginPackage installedPackage;
    bool found = false;
    
    {
        QMutexLocker locker(&d->packageMutex);
        for (const PluginPackage& package : d->installedPackages) {
            if (package.id == packageId) {
                installedPackage = package;
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        emit installationFailed(packageId, "Package not installed");
        return;
    }
    
    // Find latest version
    PluginPackage latestPackage = findPackage(packageId);
    if (latestPackage.id.isEmpty()) {
        emit installationFailed(packageId, "Package not found in repositories");
        return;
    }
    
    if (latestPackage.getVersionNumber() <= installedPackage.getVersionNumber()) {
        emit installationFailed(packageId, "No update available");
        return;
    }
    
    // Remove old version and install new version
    removePackageInternal(installedPackage);
    installPackageInternal(latestPackage);
}

void PluginDependencyManager::removePackage(const QString& packageId) {
    PluginPackage package;
    bool found = false;
    
    {
        QMutexLocker locker(&d->packageMutex);
        for (const PluginPackage& pkg : d->installedPackages) {
            if (pkg.id == packageId) {
                package = pkg;
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        emit installationFailed(packageId, "Package not installed");
        return;
    }
    
    // Check if other packages depend on this one
    QStringList dependents;
    for (const PluginPackage& pkg : d->installedPackages) {
        for (const DependencyConstraint& dep : pkg.dependencies) {
            if (dep.name == packageId) {
                dependents.append(pkg.id);
                break;
            }
        }
    }
    
    if (!dependents.isEmpty()) {
        QString error = QString("Cannot remove package: required by %1").arg(dependents.join(", "));
        emit installationFailed(packageId, error);
        return;
    }
    
    removePackageInternal(package);
}

void PluginDependencyManager::installDependencies(const ResolutionResult& result) {
    if (!result.success) {
        return;
    }
    
    // Show installation dialog
    auto dialog = new DependencyInstallationDialog(result, qobject_cast<QWidget*>(parent()));
    connect(dialog, &DependencyInstallationDialog::installationCompleted,
            this, [this](bool success) {
                if (success) {
                    qCInfo(pluginDependency) << "Dependencies installed successfully";
                } else {
                    qCWarning(pluginDependency) << "Dependency installation failed";
                }
            });
    
    dialog->show();
    dialog->startInstallation();
}

bool PluginDependencyManager::validatePackage(const PluginPackage& package) {
    // Check if package file exists
    QFileInfo fileInfo(package.installPath);
    if (!fileInfo.exists()) {
        return false;
    }
    
    // Verify checksum if available
    if (!package.checksum.isEmpty()) {
        return verifyChecksum(package.installPath, package.checksum);
    }
    
    return true;
}

bool PluginDependencyManager::verifyChecksum(const QString& filePath, const QString& expectedChecksum) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return false;
    }
    
    QString actualChecksum = hash.result().toHex();
    return actualChecksum.compare(expectedChecksum, Qt::CaseInsensitive) == 0;
}

QStringList PluginDependencyManager::validateDependencies(const QString& pluginId) {
    QStringList errors;
    PluginPackage package = findPackage(pluginId);
    
    if (package.id.isEmpty()) {
        errors.append("Package not found: " + pluginId);
        return errors;
    }
    
    for (const DependencyConstraint& constraint : package.dependencies) {
        PluginPackage dependency = findPackage(constraint.name);
        if (dependency.id.isEmpty()) {
            if (constraint.type == DependencyType::Required) {
                errors.append("Required dependency not found: " + constraint.name);
            }
            continue;
        }
        
        if (!dependency.isInstalled()) {
            if (constraint.type == DependencyType::Required) {
                errors.append("Required dependency not installed: " + constraint.name);
            }
            continue;
        }
        
        if (!constraint.isCompatible(dependency.version)) {
            errors.append(QString("Dependency version mismatch: %1 (required: %2, installed: %3)")
                         .arg(constraint.name, constraint.version, dependency.version));
        }
        
        if (!constraint.isPlatformCompatible()) {
            errors.append("Dependency platform incompatible: " + constraint.name);
        }
    }
    
    return errors;
}

void PluginDependencyManager::setInstallDirectory(const QString& directory) {
    d->installDirectory = directory;
    QDir().mkpath(directory);
}

QString PluginDependencyManager::installDirectory() const {
    return d->installDirectory;
}

void PluginDependencyManager::setCacheDirectory(const QString& directory) {
    d->cacheDirectory = directory;
    QDir().mkpath(directory);
}

QString PluginDependencyManager::cacheDirectory() const {
    return d->cacheDirectory;
}

void PluginDependencyManager::setMaxConcurrentDownloads(int count) {
    d->maxConcurrentDownloads = count;
    d->downloader->setMaxConcurrentDownloads(count);
}

int PluginDependencyManager::maxConcurrentDownloads() const {
    return d->maxConcurrentDownloads;
}

void PluginDependencyManager::onRepositoryRefreshFinished() {
    updateAvailablePackages();
    d->refreshTimer->start(); // Schedule next refresh
}

void PluginDependencyManager::onPackageDownloadFinished() {
    // Handle package download completion
    qCInfo(pluginDependency) << "Package download finished";
}

void PluginDependencyManager::onInstallationFinished() {
    // Handle installation completion
    qCInfo(pluginDependency) << "Package installation finished";
}

void PluginDependencyManager::initializeManager() {
    // Create directories
    QDir().mkpath(d->installDirectory);
    QDir().mkpath(d->cacheDirectory);

    // Load installed packages
    loadInstalledPackages();

    // Add default repositories
    addRepository("Official", QUrl("https://plugins.example.com/api/v1/packages"));
    addRepository("Community", QUrl("https://community-plugins.example.com/api/v1/packages"));

    qCInfo(pluginDependency) << "Dependency manager initialized";
}

void PluginDependencyManager::loadInstalledPackages() {
    QString packagesFile = d->installDirectory + "/installed.json";
    QFile file(packagesFile);

    if (!file.open(QIODevice::ReadOnly)) {
        qCInfo(pluginDependency) << "No installed packages file found";
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError) {
        qCWarning(pluginDependency) << "Failed to parse installed packages:" << error.errorString();
        return;
    }

    QJsonArray packagesArray = doc.array();
    QMutexLocker locker(&d->packageMutex);
    d->installedPackages.clear();

    for (const QJsonValue& value : packagesArray) {
        QJsonObject packageObj = value.toObject();
        PluginPackage package;

        package.id = packageObj["id"].toString();
        package.name = packageObj["name"].toString();
        package.version = packageObj["version"].toString();
        package.description = packageObj["description"].toString();
        package.author = packageObj["author"].toString();
        package.license = packageObj["license"].toString();
        package.installPath = packageObj["installPath"].toString();
        package.state = InstallationState::Installed;

        // Load dependencies
        QJsonArray depsArray = packageObj["dependencies"].toArray();
        for (const QJsonValue& depValue : depsArray) {
            QJsonObject depObj = depValue.toObject();
            DependencyConstraint constraint;
            constraint.name = depObj["name"].toString();
            constraint.version = depObj["version"].toString();
            constraint.versionRange = depObj["versionRange"].toString();
            constraint.type = static_cast<DependencyType>(depObj["type"].toInt());
            package.dependencies.append(constraint);
        }

        d->installedPackages.append(package);
    }

    qCInfo(pluginDependency) << "Loaded" << d->installedPackages.size() << "installed packages";
}

void PluginDependencyManager::saveInstalledPackages() {
    QString packagesFile = d->installDirectory + "/installed.json";
    QFile file(packagesFile);

    if (!file.open(QIODevice::WriteOnly)) {
        qCWarning(pluginDependency) << "Failed to save installed packages";
        return;
    }

    QJsonArray packagesArray;
    QMutexLocker locker(&d->packageMutex);

    for (const PluginPackage& package : d->installedPackages) {
        QJsonObject packageObj;
        packageObj["id"] = package.id;
        packageObj["name"] = package.name;
        packageObj["version"] = package.version;
        packageObj["description"] = package.description;
        packageObj["author"] = package.author;
        packageObj["license"] = package.license;
        packageObj["installPath"] = package.installPath;

        QJsonArray depsArray;
        for (const DependencyConstraint& constraint : package.dependencies) {
            QJsonObject depObj;
            depObj["name"] = constraint.name;
            depObj["version"] = constraint.version;
            depObj["versionRange"] = constraint.versionRange;
            depObj["type"] = static_cast<int>(constraint.type);
            depsArray.append(depObj);
        }
        packageObj["dependencies"] = depsArray;

        packagesArray.append(packageObj);
    }

    QJsonDocument doc(packagesArray);
    file.write(doc.toJson());

    qCInfo(pluginDependency) << "Saved" << d->installedPackages.size() << "installed packages";
}

void PluginDependencyManager::cleanupCache() {
    QDir cacheDir(d->cacheDirectory);
    if (!cacheDir.exists()) {
        return;
    }

    // Remove files older than 7 days
    QDateTime cutoff = QDateTime::currentDateTime().addDays(-7);
    QFileInfoList files = cacheDir.entryInfoList(QDir::Files);

    for (const QFileInfo& fileInfo : files) {
        if (fileInfo.lastModified() < cutoff) {
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }
}

ResolutionResult PluginDependencyManager::performResolution(const QList<DependencyConstraint>& constraints) {
    return d->resolver->resolve(constraints, d->availablePackages, d->installedPackages);
}

bool PluginDependencyManager::installPackageInternal(const PluginPackage& package) {
    QString destinationPath = d->installDirectory + "/" + package.id;

    // Download package
    d->downloader->downloadPackage(package, destinationPath);

    // For now, assume installation is successful
    // In a real implementation, this would extract and install the package

    QMutexLocker locker(&d->packageMutex);
    PluginPackage installedPackage = package;
    installedPackage.state = InstallationState::Installed;
    installedPackage.installPath = destinationPath;
    d->installedPackages.append(installedPackage);

    emit packageInstalled(package.id);
    return true;
}

bool PluginDependencyManager::removePackageInternal(const PluginPackage& package) {
    // Remove package files
    QDir packageDir(package.installPath);
    if (packageDir.exists()) {
        packageDir.removeRecursively();
    }

    // Remove from installed packages list
    QMutexLocker locker(&d->packageMutex);
    d->installedPackages.removeAll(package);

    emit packageRemoved(package.id);
    return true;
}

void PluginDependencyManager::updateAvailablePackages() {
    QMutexLocker locker(&d->packageMutex);
    d->availablePackages.clear();

    for (PluginRepository* repository : d->repositories.values()) {
        d->availablePackages.append(repository->packages());
    }

    qCInfo(pluginDependency) << "Updated available packages:" << d->availablePackages.size();
}
