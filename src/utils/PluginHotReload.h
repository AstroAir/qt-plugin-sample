// PluginHotReload.h - Hot Reload System for Plugin Development
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>

// Forward declarations
class PluginHotReloadManager;
class FileWatcher;
class StatePreserver;
class DependencyTracker;
class HotReloadWidget;

// Reload triggers
enum class ReloadTrigger {
    FileChanged,        // Source file modified
    ConfigChanged,      // Configuration file changed
    DependencyChanged,  // Dependency updated
    Manual,             // Manual reload request
    Scheduled,          // Scheduled reload
    Error               // Error-triggered reload
};

// Reload strategies
enum class ReloadStrategy {
    Immediate,          // Reload immediately on change
    Debounced,          // Wait for changes to settle
    Batch,              // Batch multiple changes
    OnDemand,           // Reload only when requested
    Scheduled,          // Reload at scheduled times
    Smart               // Intelligent reload based on context
};

// Reload status
enum class ReloadStatus {
    Idle,               // Not reloading
    Watching,           // Watching for changes
    Detected,           // Changes detected
    Building,           // Building/compiling
    Loading,            // Loading new version
    Preserving,         // Preserving state
    Restoring,          // Restoring state
    Completed,          // Reload completed
    Failed,             // Reload failed
    Cancelled           // Reload cancelled
};

// File change information
struct FileChange {
    QString filePath;
    QString relativePath;
    QDateTime timestamp;
    qint64 size;
    QString checksum;
    QString changeType; // modified, added, removed, renamed
    QJsonObject metadata;
    
    FileChange() = default;
    FileChange(const QString& path, const QString& type)
        : filePath(path), timestamp(QDateTime::currentDateTime()), changeType(type) {
        QFileInfo info(path);
        size = info.size();
        relativePath = info.fileName();
    }
    
    bool isSourceFile() const;
    bool isConfigFile() const;
    bool isResourceFile() const;
    QString getFileExtension() const;
};

// Reload session information
struct ReloadSession {
    QString sessionId;
    QString pluginId;
    QDateTime startTime;
    QDateTime endTime;
    ReloadTrigger trigger;
    ReloadStatus status;
    QList<FileChange> changes;
    QString errorMessage;
    int buildTime; // milliseconds
    int reloadTime; // milliseconds
    bool statePreserved;
    QJsonObject preservedState;
    QStringList affectedFiles;
    QJsonObject metadata;
    
    ReloadSession() = default;
    ReloadSession(const QString& plugId, ReloadTrigger trig)
        : pluginId(plugId), startTime(QDateTime::currentDateTime()), trigger(trig),
          status(ReloadStatus::Detected), buildTime(0), reloadTime(0), statePreserved(false) {
        sessionId = generateSessionId();
    }
    
    int getTotalTime() const;
    bool wasSuccessful() const;
    QString getStatusString() const;
    
private:
    QString generateSessionId() const;
};

// Hot reload configuration
struct HotReloadConfig {
    bool enabled = true;
    ReloadStrategy strategy = ReloadStrategy::Debounced;
    int debounceDelay = 500; // milliseconds
    int batchTimeout = 2000; // milliseconds
    bool preserveState = true;
    bool autoRecompile = true;
    bool showNotifications = true;
    bool enableLogging = true;
    QStringList watchedExtensions = {"cpp", "h", "hpp", "c", "cc", "cxx", "qml", "js", "json", "xml"};
    QStringList ignoredPaths = {".git", ".svn", "build", "debug", "release", "tmp"};
    QStringList buildCommands;
    QString buildDirectory;
    QString outputDirectory;
    int maxReloadHistory = 100;
    bool enableDependencyTracking = true;
    bool enableSmartReload = true;
    
    HotReloadConfig() = default;
};

// Main hot reload manager
class PluginHotReloadManager : public QObject {
    Q_OBJECT

public:
    explicit PluginHotReloadManager(QObject* parent = nullptr);
    ~PluginHotReloadManager() override;

    // Hot reload control
    void enableHotReload(bool enable);
    bool isHotReloadEnabled() const;
    void startWatching(const QString& pluginId, const QString& sourcePath);
    void stopWatching(const QString& pluginId);
    bool isWatching(const QString& pluginId) const;
    QStringList getWatchedPlugins() const;
    
    // Configuration
    void setConfiguration(const HotReloadConfig& config);
    HotReloadConfig configuration() const;
    void setReloadStrategy(ReloadStrategy strategy);
    ReloadStrategy reloadStrategy() const;
    void setDebounceDelay(int milliseconds);
    int debounceDelay() const;
    
    // Manual reload operations
    void reloadPlugin(const QString& pluginId);
    void reloadAllPlugins();
    void cancelReload(const QString& pluginId);
    void forceReload(const QString& pluginId);
    
    // State preservation
    void preservePluginState(const QString& pluginId);
    void restorePluginState(const QString& pluginId);
    QJsonObject getPreservedState(const QString& pluginId) const;
    void setPreservedState(const QString& pluginId, const QJsonObject& state);
    void clearPreservedState(const QString& pluginId);
    
    // Build integration
    void setBuildCommand(const QString& pluginId, const QStringList& command);
    QStringList getBuildCommand(const QString& pluginId) const;
    void setBuildDirectory(const QString& pluginId, const QString& directory);
    QString getBuildDirectory(const QString& pluginId) const;
    bool buildPlugin(const QString& pluginId);
    
    // Reload history and statistics
    QList<ReloadSession> getReloadHistory(const QString& pluginId = "") const;
    void clearReloadHistory(const QString& pluginId = "");
    int getReloadCount(const QString& pluginId) const;
    double getAverageReloadTime(const QString& pluginId) const;
    double getSuccessRate(const QString& pluginId) const;
    
    // File watching
    void addWatchPath(const QString& pluginId, const QString& path);
    void removeWatchPath(const QString& pluginId, const QString& path);
    QStringList getWatchPaths(const QString& pluginId) const;
    void setWatchedExtensions(const QStringList& extensions);
    QStringList watchedExtensions() const;
    void setIgnoredPaths(const QStringList& paths);
    QStringList ignoredPaths() const;

signals:
    void hotReloadEnabled(bool enabled);
    void watchingStarted(const QString& pluginId);
    void watchingStopped(const QString& pluginId);
    void fileChanged(const QString& pluginId, const FileChange& change);
    void reloadStarted(const QString& pluginId, ReloadTrigger trigger);
    void reloadProgress(const QString& pluginId, const QString& stage, int percentage);
    void reloadCompleted(const QString& pluginId, bool success, int totalTime);
    void reloadFailed(const QString& pluginId, const QString& error);
    void buildStarted(const QString& pluginId);
    void buildCompleted(const QString& pluginId, bool success, int buildTime);
    void buildFailed(const QString& pluginId, const QString& error);
    void statePreserved(const QString& pluginId);
    void stateRestored(const QString& pluginId);

public slots:
    void refreshWatchers();
    void showHotReloadWidget();
    void showReloadHistory();

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& path);
    void onDebounceTimer();
    void onBatchTimer();
    void onBuildProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onReloadTimer();

private:
    
    void initializeManager();
    void loadConfiguration();
    void saveConfiguration();
    void setupFileWatchers();
    void processFileChange(const QString& pluginId, const FileChange& change);
    void scheduleReload(const QString& pluginId, ReloadTrigger trigger);
    void performReload(const QString& pluginId, ReloadTrigger trigger);
    bool shouldReload(const QString& pluginId, const FileChange& change) const;
    void updateDependencies(const QString& pluginId);
    QString calculateFileChecksum(const QString& filePath) const;
    void logReloadSession(const ReloadSession& session);
};

// File watcher for monitoring source files
class FileWatcher : public QObject {
    Q_OBJECT

public:
    explicit FileWatcher(const QString& pluginId, QObject* parent = nullptr);
    ~FileWatcher() override;

    // Watcher control
    void startWatching();
    void stopWatching();
    bool isWatching() const;
    QString pluginId() const;
    
    // Path management
    void addPath(const QString& path);
    void removePath(const QString& path);
    QStringList watchedPaths() const;
    void setRecursive(bool recursive);
    bool isRecursive() const;
    
    // Filter configuration
    void setWatchedExtensions(const QStringList& extensions);
    QStringList watchedExtensions() const;
    void setIgnoredPaths(const QStringList& paths);
    QStringList ignoredPaths() const;
    void setIgnoreHiddenFiles(bool ignore);
    bool ignoreHiddenFiles() const;
    
    // Change detection
    void setChangeDetectionMode(const QString& mode); // timestamp, checksum, size
    QString changeDetectionMode() const;
    void forceCheck();
    QDateTime lastChangeTime() const;

signals:
    void fileChanged(const FileChange& change);
    void directoryChanged(const QString& path);
    void watchingStarted();
    void watchingStopped();
    void errorOccurred(const QString& error);

private slots:
    void onFileSystemWatcherFileChanged(const QString& path);
    void onFileSystemWatcherDirectoryChanged(const QString& path);
    void onCheckTimer();

private:
    QString m_pluginId;
    QFileSystemWatcher* m_fileSystemWatcher;
    QTimer* m_checkTimer;
    QStringList m_watchedPaths;
    QStringList m_watchedExtensions;
    QStringList m_ignoredPaths;
    QMap<QString, QDateTime> m_lastModified;
    QMap<QString, QString> m_lastChecksum;
    QMap<QString, qint64> m_lastSize;
    bool m_isWatching;
    bool m_recursive;
    bool m_ignoreHiddenFiles;
    QString m_changeDetectionMode;
    
    void setupWatcher();
    void addPathRecursively(const QString& path);
    bool shouldWatchFile(const QString& filePath) const;
    bool shouldIgnorePath(const QString& path) const;
    FileChange detectChange(const QString& filePath) const;
    void updateFileInfo(const QString& filePath);
};

// State preserver for maintaining plugin state during reloads
class StatePreserver : public QObject {
    Q_OBJECT

public:
    explicit StatePreserver(QObject* parent = nullptr);
    ~StatePreserver() override;

    // State preservation
    void preserveState(const QString& pluginId, const QJsonObject& state);
    QJsonObject getPreservedState(const QString& pluginId) const;
    void restoreState(const QString& pluginId, QJsonObject& state);
    void clearState(const QString& pluginId);
    bool hasPreservedState(const QString& pluginId) const;
    
    // Automatic state extraction
    QJsonObject extractPluginState(const QString& pluginId) const;
    void injectPluginState(const QString& pluginId, const QJsonObject& state);
    
    // State serialization
    void saveStateToDisk(const QString& pluginId, const QString& filePath = "");
    QJsonObject loadStateFromDisk(const QString& pluginId, const QString& filePath = "");
    void setStateDirectory(const QString& directory);
    QString stateDirectory() const;
    
    // State validation
    bool validateState(const QJsonObject& state) const;
    QStringList getStateValidationErrors(const QJsonObject& state) const;
    void setStateSchema(const QJsonObject& schema);
    QJsonObject stateSchema() const;

signals:
    void statePreserved(const QString& pluginId);
    void stateRestored(const QString& pluginId);
    void stateCleared(const QString& pluginId);
    void stateValidationFailed(const QString& pluginId, const QStringList& errors);

private:
    QMap<QString, QJsonObject> m_preservedStates;
    QString m_stateDirectory;
    QJsonObject m_stateSchema;
    
    QString getStateFilePath(const QString& pluginId) const;
    QJsonObject sanitizeState(const QJsonObject& state) const;
    void ensureStateDirectory();
};

// Dependency tracker for smart reloading
class DependencyTracker : public QObject {
    Q_OBJECT

public:
    explicit DependencyTracker(QObject* parent = nullptr);
    ~DependencyTracker() override;

    // Dependency management
    void addDependency(const QString& pluginId, const QString& dependencyPath);
    void removeDependency(const QString& pluginId, const QString& dependencyPath);
    QStringList getDependencies(const QString& pluginId) const;
    QStringList getDependents(const QString& dependencyPath) const;
    void clearDependencies(const QString& pluginId);
    
    // Dependency analysis
    void analyzeDependencies(const QString& pluginId, const QString& sourcePath);
    void refreshDependencies(const QString& pluginId);
    QStringList findCircularDependencies() const;
    QStringList getReloadOrder(const QStringList& pluginIds) const;
    
    // Change impact analysis
    QStringList getAffectedPlugins(const QString& changedFile) const;
    bool shouldReload(const QString& pluginId, const QString& changedFile) const;
    int getReloadPriority(const QString& pluginId, const QString& changedFile) const;
    
    // Dependency graph
    QJsonObject exportDependencyGraph() const;
    void importDependencyGraph(const QJsonObject& graph);
    void visualizeDependencyGraph(const QString& outputPath) const;

signals:
    void dependencyAdded(const QString& pluginId, const QString& dependencyPath);
    void dependencyRemoved(const QString& pluginId, const QString& dependencyPath);
    void dependenciesAnalyzed(const QString& pluginId);
    void circularDependencyDetected(const QStringList& cycle);

private:
    QMap<QString, QSet<QString>> m_dependencies; // plugin -> dependencies
    QMap<QString, QSet<QString>> m_dependents;   // dependency -> dependents
    
    void updateDependents();
    QStringList parseDependenciesFromFile(const QString& filePath) const;
    QStringList parseIncludeDirectives(const QString& content) const;
    QStringList parseImportStatements(const QString& content) const;
    bool hasCyclicDependency(const QString& pluginId, QSet<QString>& visited, QSet<QString>& recursionStack) const;
};

// Hot reload widget for monitoring and control
class HotReloadWidget : public QWidget {
    Q_OBJECT

public:
    explicit HotReloadWidget(PluginHotReloadManager* manager, QWidget* parent = nullptr);
    ~HotReloadWidget() override;

    // Display management
    void refreshPluginList();
    void refreshReloadHistory();
    void refreshFileWatchers();
    void showPluginDetails(const QString& pluginId);

signals:
    void pluginSelected(const QString& pluginId);
    void reloadRequested(const QString& pluginId);
    void watchingToggled(const QString& pluginId, bool enabled);
    void configurationRequested();

private slots:
    void onPluginItemClicked();
    void onReloadButtonClicked();
    void onWatchToggled();
    void onConfigureClicked();
    void onRefreshClicked();
    void onClearHistoryClicked();

private:
    PluginHotReloadManager* m_manager;
    QSplitter* m_splitter;
    QTreeWidget* m_pluginTree;
    QTableWidget* m_historyTable;
    QTextEdit* m_logView;
    QTabWidget* m_tabWidget;
    
    void setupUI();
    void setupPluginTab();
    void setupHistoryTab();
    void setupLogTab();
    void populatePluginTree();
    void populateHistoryTable();
    void updateLogView();
    QTreeWidgetItem* createPluginItem(const QString& pluginId);
    void addHistoryRow(const ReloadSession& session);
    QString formatReloadTime(int milliseconds) const;
};

// Hot reload configuration dialog
class HotReloadConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit HotReloadConfigDialog(const HotReloadConfig& config, QWidget* parent = nullptr);
    ~HotReloadConfigDialog() override;

    // Configuration management
    HotReloadConfig getConfiguration() const;
    void setConfiguration(const HotReloadConfig& config);

public slots:
    void accept() override;
    void reject() override;

signals:
    void configurationChanged(const HotReloadConfig& config);

private slots:
    void onStrategyChanged();
    void onExtensionChanged();
    void onPathChanged();
    void onBuildCommandChanged();
    void onResetClicked();

private:
    HotReloadConfig m_config;
    QCheckBox* m_enabledCheck;
    QComboBox* m_strategyCombo;
    QSpinBox* m_debounceSpinBox;
    QSpinBox* m_batchTimeoutSpinBox;
    QCheckBox* m_preserveStateCheck;
    QCheckBox* m_autoRecompileCheck;
    QCheckBox* m_showNotificationsCheck;
    QListWidget* m_extensionsList;
    QListWidget* m_ignoredPathsList;
    QTextEdit* m_buildCommandsEdit;
    QLineEdit* m_buildDirectoryEdit;
    QLineEdit* m_outputDirectoryEdit;
    
    void setupUI();
    void setupGeneralTab();
    void setupWatchingTab();
    void setupBuildTab();
    void updateUIFromConfig();
    void updateConfigFromUI();
    void addExtension();
    void removeExtension();
    void addIgnoredPath();
    void removeIgnoredPath();
};
