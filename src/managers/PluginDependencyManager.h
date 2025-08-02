// PluginDependencyManager.h - Advanced Plugin Dependency Resolution and Management
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
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QMutex>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <memory>

// Forward declarations
class PluginDependencyResolver;
class PluginRepository;
class PluginDownloader;
class DependencyGraph;
class PluginPackage;
class DependencyInstaller;

// Dependency types
enum class DependencyType {
    Required,       // Must be present for plugin to function
    Optional,       // Enhances functionality but not required
    Development,    // Only needed during development
    Runtime,        // Required at runtime
    Build,          // Required for building
    Peer           // Should be provided by parent application
};

// Dependency resolution strategies
enum class ResolutionStrategy {
    Latest,         // Always use latest compatible version
    Stable,         // Prefer stable releases
    Exact,          // Use exact version specified
    Range,          // Use version within specified range
    Conservative    // Use oldest compatible version
};

// Installation states
enum class InstallationState {
    NotInstalled,
    Downloading,
    Installing,
    Installed,
    UpdateAvailable,
    Failed,
    Corrupted,
    Incompatible
};

// Dependency constraint
struct DependencyConstraint {
    QString name;
    QString version;
    QString versionRange;
    DependencyType type = DependencyType::Required;
    QStringList platforms;
    QStringList architectures;
    QJsonObject metadata;
    
    DependencyConstraint() = default;
    DependencyConstraint(const QString& n, const QString& v, DependencyType t = DependencyType::Required)
        : name(n), version(v), type(t) {}
    
    bool isCompatible(const QString& targetVersion) const;
    bool isPlatformCompatible() const;
    QVersionNumber getMinVersion() const;
    QVersionNumber getMaxVersion() const;
};

// Plugin package information
struct PluginPackage {
    QString id;
    QString name;
    QString version;
    QString description;
    QString author;
    QString license;
    QUrl downloadUrl;
    QUrl repositoryUrl;
    QString checksum;
    qint64 size;
    QDateTime publishDate;
    QDateTime lastUpdate;
    QStringList tags;
    QStringList categories;
    QList<DependencyConstraint> dependencies;
    QJsonObject metadata;
    InstallationState state = InstallationState::NotInstalled;
    QString installPath;
    
    PluginPackage() = default;
    PluginPackage(const QString& i, const QString& n, const QString& v)
        : id(i), name(n), version(v) {}
    
    bool isInstalled() const { return state == InstallationState::Installed; }
    bool hasUpdate() const { return state == InstallationState::UpdateAvailable; }
    QVersionNumber getVersionNumber() const { return QVersionNumber::fromString(version); }

    // Equality operator for QList operations
    bool operator==(const PluginPackage& other) const {
        return id == other.id && version == other.version;
    }
};

// Dependency resolution result
struct ResolutionResult {
    bool success = false;
    QString errorMessage;
    QList<PluginPackage> toInstall;
    QList<PluginPackage> toUpdate;
    QList<PluginPackage> toRemove;
    QList<QString> conflicts;
    QMap<QString, QString> resolutionLog;
    
    bool hasConflicts() const { return !conflicts.isEmpty(); }
    int totalOperations() const { return toInstall.size() + toUpdate.size() + toRemove.size(); }
};

// Main dependency manager
class PluginDependencyManager : public QObject {
    Q_OBJECT

public:
    explicit PluginDependencyManager(QObject* parent = nullptr);
    ~PluginDependencyManager() override;

    // Repository management
    void addRepository(const QString& name, const QUrl& url);
    void removeRepository(const QString& name);
    QStringList repositories() const;
    void refreshRepositories();
    
    // Package management
    QList<PluginPackage> availablePackages() const;
    QList<PluginPackage> installedPackages() const;
    PluginPackage findPackage(const QString& id) const;
    QList<PluginPackage> searchPackages(const QString& query) const;
    
    // Dependency resolution
    ResolutionResult resolveDependencies(const QString& pluginId);
    ResolutionResult resolveDependencies(const QList<DependencyConstraint>& constraints);
    void setResolutionStrategy(ResolutionStrategy strategy);
    ResolutionStrategy resolutionStrategy() const;
    
    // Installation operations
    void installPackage(const QString& packageId);
    void updatePackage(const QString& packageId);
    void removePackage(const QString& packageId);
    void installDependencies(const ResolutionResult& result);
    
    // Validation and verification
    bool validatePackage(const PluginPackage& package);
    bool verifyChecksum(const QString& filePath, const QString& expectedChecksum);
    QStringList validateDependencies(const QString& pluginId);
    
    // Configuration
    void setInstallDirectory(const QString& directory);
    QString installDirectory() const;
    void setCacheDirectory(const QString& directory);
    QString cacheDirectory() const;
    void setMaxConcurrentDownloads(int count);
    int maxConcurrentDownloads() const;

signals:
    void repositoryRefreshed(const QString& repository);
    void packageInstalled(const QString& packageId);
    void packageUpdated(const QString& packageId);
    void packageRemoved(const QString& packageId);
    void installationProgress(const QString& packageId, int percentage);
    void installationFailed(const QString& packageId, const QString& error);
    void dependencyResolved(const QString& pluginId, const ResolutionResult& result);
    void conflictDetected(const QStringList& conflicts);

private slots:
    void onRepositoryRefreshFinished();
    void onPackageDownloadFinished();
    void onInstallationFinished();

private:
    struct DependencyManagerPrivate;
    std::unique_ptr<DependencyManagerPrivate> d;
    
    void initializeManager();
    void loadInstalledPackages();
    void saveInstalledPackages();
    void cleanupCache();
    ResolutionResult performResolution(const QList<DependencyConstraint>& constraints);
    bool installPackageInternal(const PluginPackage& package);
    bool removePackageInternal(const PluginPackage& package);
    void updateAvailablePackages();
};

// Dependency resolver with graph algorithms
class PluginDependencyResolver : public QObject {
    Q_OBJECT

public:
    explicit PluginDependencyResolver(QObject* parent = nullptr);
    ~PluginDependencyResolver() override;

    // Resolution methods
    ResolutionResult resolve(const QList<DependencyConstraint>& constraints,
                           const QList<PluginPackage>& available,
                           const QList<PluginPackage>& installed);
    
    void setStrategy(ResolutionStrategy strategy);
    ResolutionStrategy strategy() const;
    
    // Conflict resolution
    QStringList detectConflicts(const QList<PluginPackage>& packages);
    QList<PluginPackage> resolveConflicts(const QList<PluginPackage>& conflicting);
    
    // Graph operations
    DependencyGraph buildDependencyGraph(const QList<PluginPackage>& packages);
    QStringList getInstallationOrder(const QList<PluginPackage>& packages);
    bool hasCircularDependencies(const QList<PluginPackage>& packages);

signals:
    void resolutionProgress(int percentage);
    void resolutionCompleted(const ResolutionResult& result);

private:
    ResolutionStrategy m_strategy;
    
    PluginPackage selectBestVersion(const QString& packageId, 
                                  const DependencyConstraint& constraint,
                                  const QList<PluginPackage>& available);
    bool isVersionCompatible(const QString& version, const DependencyConstraint& constraint);
    QList<PluginPackage> topologicalSort(const QList<PluginPackage>& packages);
};

// Plugin repository interface
class PluginRepository : public QObject {
    Q_OBJECT

public:
    explicit PluginRepository(const QString& name, const QUrl& url, QObject* parent = nullptr);
    ~PluginRepository() override;

    // Repository information
    QString name() const;
    QUrl url() const;
    bool isOnline() const;
    QDateTime lastRefresh() const;
    
    // Package operations
    void refresh();
    QList<PluginPackage> packages() const;
    PluginPackage findPackage(const QString& id) const;
    QList<PluginPackage> searchPackages(const QString& query) const;
    
    // Metadata
    QString description() const;
    QString version() const;
    QStringList supportedPlatforms() const;

signals:
    void refreshStarted();
    void refreshFinished(bool success);
    void packageListUpdated();
    void errorOccurred(const QString& error);

private slots:
    void onNetworkReplyFinished();

private:
    QString m_name;
    QUrl m_url;
    QList<PluginPackage> m_packages;
    QNetworkAccessManager* m_networkManager;
    QDateTime m_lastRefresh;
    bool m_isOnline;
    
    void parseRepositoryData(const QByteArray& data);
    PluginPackage parsePackageData(const QJsonObject& json);
};

// Package downloader with progress tracking
class PluginDownloader : public QObject {
    Q_OBJECT

public:
    explicit PluginDownloader(QObject* parent = nullptr);
    ~PluginDownloader() override;

    // Download operations
    void downloadPackage(const PluginPackage& package, const QString& destination);
    void cancelDownload(const QString& packageId);
    void cancelAllDownloads();
    
    // Configuration
    void setMaxConcurrentDownloads(int count);
    int maxConcurrentDownloads() const;
    void setTimeout(int seconds);
    int timeout() const;

signals:
    void downloadStarted(const QString& packageId);
    void downloadProgress(const QString& packageId, qint64 received, qint64 total);
    void downloadFinished(const QString& packageId, const QString& filePath);
    void downloadFailed(const QString& packageId, const QString& error);

private slots:
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError error);

private:
    struct DownloadInfo {
        QString packageId;
        QNetworkReply* reply;
        QString destination;
        QDateTime startTime;
    };
    
    QNetworkAccessManager* m_networkManager;
    QMap<QString, DownloadInfo> m_activeDownloads;
    int m_maxConcurrentDownloads;
    int m_timeout;
    QQueue<PluginPackage> m_downloadQueue;
    
    void processDownloadQueue();
    void startDownload(const PluginPackage& package, const QString& destination);
};

// Dependency graph for resolution algorithms
class DependencyGraph {
public:
    struct Node {
        QString packageId;
        PluginPackage package;
        QSet<QString> dependencies;
        QSet<QString> dependents;
        int depth = 0;
        bool visited = false;
    };

    DependencyGraph() = default;
    ~DependencyGraph() = default;

    // Graph construction
    void addNode(const QString& packageId, const PluginPackage& package);
    void addEdge(const QString& from, const QString& to);
    void removeNode(const QString& packageId);
    void removeEdge(const QString& from, const QString& to);
    
    // Graph queries
    bool hasNode(const QString& packageId) const;
    Node getNode(const QString& packageId) const;
    QStringList getNodes() const;
    QStringList getDependencies(const QString& packageId) const;
    QStringList getDependents(const QString& packageId) const;
    
    // Graph algorithms
    bool hasCircularDependencies() const;
    QStringList topologicalSort() const;
    QStringList getInstallationOrder() const;
    int getDepth(const QString& packageId) const;
    
    // Graph analysis
    QStringList findRoots() const;
    QStringList findLeaves() const;
    QStringList findCycles() const;
    int nodeCount() const;
    int edgeCount() const;

private:
    QMap<QString, Node> m_nodes;
    
    bool hasCycleUtil(const QString& nodeId, QSet<QString>& visited, QSet<QString>& recursionStack) const;
    void topologicalSortUtil(const QString& nodeId, QSet<QString>& visited, QStringList& result) const;
    void calculateDepths();
};

// Installation progress dialog
class DependencyInstallationDialog : public QDialog {
    Q_OBJECT

public:
    explicit DependencyInstallationDialog(const ResolutionResult& result, QWidget* parent = nullptr);
    ~DependencyInstallationDialog() override;

    // Installation control
    void startInstallation();
    void cancelInstallation();
    bool isInstalling() const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void installationCompleted(bool success);
    void installationCancelled();

private slots:
    void onInstallationProgress(const QString& packageId, int percentage);
    void onPackageInstalled(const QString& packageId);
    void onInstallationFailed(const QString& packageId, const QString& error);

private:
    ResolutionResult m_result;
    QTreeWidget* m_packageTree;
    QProgressBar* m_overallProgress;
    QLabel* m_statusLabel;
    QPushButton* m_cancelButton;
    bool m_installing;
    int m_completedPackages;
    
    void setupUI();
    void populatePackageTree();
    void updateProgress();
};

// Dependency viewer widget
class DependencyViewerWidget : public QWidget {
    Q_OBJECT

public:
    explicit DependencyViewerWidget(QWidget* parent = nullptr);
    ~DependencyViewerWidget() override;

    // Display management
    void setPlugin(const QString& pluginId);
    void setDependencyGraph(const DependencyGraph& graph);
    void refreshView();
    
    // View options
    void setShowOptionalDependencies(bool show);
    bool showOptionalDependencies() const;
    void setShowDevelopmentDependencies(bool show);
    bool showDevelopmentDependencies() const;

signals:
    void packageSelected(const QString& packageId);
    void packageDoubleClicked(const QString& packageId);
    void dependencyActionRequested(const QString& action, const QString& packageId);

private slots:
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onContextMenuRequested(const QPoint& pos);

private:
    QString m_currentPlugin;
    DependencyGraph m_graph;
    QTreeWidget* m_dependencyTree;
    QTextEdit* m_detailsView;
    QSplitter* m_splitter;
    bool m_showOptional;
    bool m_showDevelopment;
    
    void setupUI();
    void populateDependencyTree();
    void updateDetailsView(const QString& packageId);
    QTreeWidgetItem* createPackageItem(const PluginPackage& package);
    void addDependenciesToItem(QTreeWidgetItem* parent, const QStringList& dependencies);
};
