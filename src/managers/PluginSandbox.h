// PluginSandbox.h - Advanced Plugin Sandboxing and Isolation System
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
#include <QSpinBox>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <memory>
#include "core/PluginInterface.h"
#include "managers/PluginPermissionSystem.h"
#include "managers/PluginResourceMonitor.h"

// Forward declarations
class PluginSandboxManager;
class SandboxEnvironment;
class SecurityPolicy;
class ResourceMonitor;
class IsolationContainer;
class SandboxedProcess;

// Note: SecurityLevel enum is defined in core/PluginInterface.h

// Sandbox types
enum class SandboxType {
    Process,       // Process-based isolation
    Thread,        // Thread-based isolation
    Container,     // Container-based isolation (if available)
    Virtual,       // Virtual machine isolation
    Hybrid         // Combination of multiple types
};

// Note: PermissionType enum is defined in managers/PluginPermissionSystem.h
// Note: ResourceType enum is defined in managers/PluginResourceMonitor.h

// Resource limits
struct ResourceLimits {
    qint64 maxMemoryBytes = 100 * 1024 * 1024; // 100MB default
    double maxCpuPercent = 25.0; // 25% CPU usage
    int maxThreads = 10;
    int maxFileHandles = 100;
    int maxNetworkConnections = 10;
    qint64 maxDiskSpace = 50 * 1024 * 1024; // 50MB disk space
    int maxProcesses = 5;
    int timeoutSeconds = 300; // 5 minutes
    
    ResourceLimits() = default;
};

// Security policy configuration
struct SecurityPolicy {
    QString name;
    QString description;
    SecurityLevel level = SecurityLevel::Sandbox;
    SandboxType sandboxType = SandboxType::Process;
    ResourceLimits limits;
    QMap<PermissionType, bool> permissions;
    QStringList allowedPaths;
    QStringList blockedPaths;
    QStringList allowedHosts;
    QStringList blockedHosts;
    QStringList allowedProcesses;
    QStringList blockedProcesses;
    QStringList allowedRegistryKeys;
    QStringList blockedRegistryKeys;
    bool allowNetworkAccess = false;
    bool allowFileSystemAccess = false;
    bool allowProcessCreation = false;
    bool allowRegistryAccess = false;
    bool allowHardwareAccess = false;
    bool enableLogging = true;
    bool enableMonitoring = true;
    QJsonObject customSettings;
    
    SecurityPolicy() {
        initializeDefaults();
    }
    
    void initializeDefaults();
    bool isPermissionAllowed(PermissionType type) const;
    void setPermission(PermissionType type, bool allowed);
};

// Sandbox violation information
struct SandboxViolation {
    QString id;
    QString pluginId;
    QString sandboxId;
    PermissionType violationType;
    QString description;
    QString details;
    QDateTime timestamp;
    QString severity; // Low, Medium, High, Critical
    QString action; // Blocked, Allowed, Logged
    QJsonObject metadata;
    
    SandboxViolation() = default;
    SandboxViolation(const QString& pid, PermissionType type, const QString& desc)
        : pluginId(pid), violationType(type), description(desc), timestamp(QDateTime::currentDateTime()) {}
};

// Main sandbox manager
class PluginSandboxManager : public QObject {
    Q_OBJECT

public:
    explicit PluginSandboxManager(QObject* parent = nullptr);
    ~PluginSandboxManager() override;

    // Sandbox management
    QString createSandbox(const QString& pluginId, const SecurityPolicy& policy);
    void destroySandbox(const QString& sandboxId);
    bool isSandboxActive(const QString& sandboxId) const;
    QStringList activeSandboxes() const;
    
    // Plugin execution
    bool executePlugin(const QString& pluginId, const QString& sandboxId);
    void terminatePlugin(const QString& pluginId);
    bool isPluginRunning(const QString& pluginId) const;
    QString getPluginSandbox(const QString& pluginId) const;
    
    // Security policies
    void addSecurityPolicy(const SecurityPolicy& policy);
    void removeSecurityPolicy(const QString& policyName);
    SecurityPolicy getSecurityPolicy(const QString& policyName) const;
    QStringList availablePolicies() const;
    void setDefaultPolicy(const QString& policyName);
    QString defaultPolicy() const;
    
    // Monitoring and violations
    QList<SandboxViolation> getViolations(const QString& pluginId = "") const;
    QList<SandboxViolation> getRecentViolations(int hours = 24) const;
    void clearViolations(const QString& pluginId = "");
    int getViolationCount(const QString& pluginId) const;
    
    // Resource monitoring
    ResourceLimits getCurrentUsage(const QString& sandboxId) const;
    bool isResourceLimitExceeded(const QString& sandboxId, ResourceType resourceType) const;
    void updateResourceLimits(const QString& sandboxId, const ResourceLimits& limits);
    
    // Configuration
    void setSandboxDirectory(const QString& directory);
    QString sandboxDirectory() const;
    void setLoggingEnabled(bool enabled);
    bool isLoggingEnabled() const;
    void setMonitoringInterval(int milliseconds);
    int monitoringInterval() const;

signals:
    void sandboxCreated(const QString& sandboxId, const QString& pluginId);
    void sandboxDestroyed(const QString& sandboxId);
    void pluginStarted(const QString& pluginId, const QString& sandboxId);
    void pluginTerminated(const QString& pluginId, const QString& reason);
    void violationDetected(const SandboxViolation& violation);
    void resourceLimitExceeded(const QString& sandboxId, ResourceType resourceType);
    void securityPolicyUpdated(const QString& policyName);

public slots:
    void refreshSandboxes();
    void cleanupInactiveSandboxes();
    void showSandboxManager();

private slots:
    void onMonitoringTimer();
    void onSandboxProcessFinished(int exitCode);
    void onViolationDetected();

private:
    struct SandboxManagerPrivate;
    std::unique_ptr<SandboxManagerPrivate> d;
    
    void initializeManager();
    void loadSecurityPolicies();
    void saveSecurityPolicies();
    void setupMonitoring();
    void createDefaultPolicies();
    QString generateSandboxId() const;
    void logViolation(const SandboxViolation& violation);
    void enforceResourceLimits(const QString& sandboxId);
};

// Sandbox environment for isolated execution
class SandboxEnvironment : public QObject {
    Q_OBJECT

public:
    explicit SandboxEnvironment(const QString& sandboxId, const SecurityPolicy& policy, QObject* parent = nullptr);
    ~SandboxEnvironment() override;

    // Environment management
    QString sandboxId() const;
    SecurityPolicy securityPolicy() const;
    void updateSecurityPolicy(const SecurityPolicy& policy);
    bool isActive() const;
    void activate();
    void deactivate();
    
    // Process management
    bool startProcess(const QString& program, const QStringList& arguments = QStringList());
    void terminateProcess();
    bool isProcessRunning() const;
    qint64 processId() const;
    
    // Resource monitoring
    ResourceLimits getCurrentUsage() const;
    void updateResourceUsage();
    bool checkResourceLimits() const;
    
    // Permission checking
    bool checkPermission(PermissionType type, const QString& resource = "") const;
    bool checkFileAccess(const QString& filePath, QIODevice::OpenMode mode) const;
    bool checkNetworkAccess(const QString& host, int port) const;
    bool checkProcessAccess(const QString& processName) const;
    
    // Communication
    void sendMessage(const QJsonObject& message);
    QJsonObject receiveMessage();
    bool hasMessages() const;

signals:
    void processStarted(qint64 processId);
    void processFinished(int exitCode);
    void permissionViolation(PermissionType type, const QString& resource);
    void resourceLimitExceeded(ResourceType resourceType);
    void messageReceived(const QJsonObject& message);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onResourceMonitorTimer();

private:
    QString m_sandboxId;
    SecurityPolicy m_policy;
    std::unique_ptr<SandboxedProcess> m_process;
    std::unique_ptr<ResourceMonitor> m_resourceMonitor;
    std::unique_ptr<IsolationContainer> m_container;
    QTimer* m_resourceTimer;
    bool m_isActive;
    QLocalServer* m_communicationServer;
    QList<QLocalSocket*> m_clientSockets;
    
    void setupIsolation();
    void setupCommunication();
    void enforceFileSystemRestrictions();
    void enforceNetworkRestrictions();
    void enforceProcessRestrictions();
    void createSandboxDirectory();
    void cleanupSandboxDirectory();
};

// Sandboxed process wrapper
class SandboxedProcess : public QProcess {
    Q_OBJECT

public:
    explicit SandboxedProcess(const SecurityPolicy& policy, QObject* parent = nullptr);
    ~SandboxedProcess() override;

    // Enhanced process control
    void setSecurityPolicy(const SecurityPolicy& policy);
    SecurityPolicy securityPolicy() const;
    void setResourceLimits(const ResourceLimits& limits);
    ResourceLimits resourceLimits() const;
    
    // Resource monitoring
    qint64 memoryUsage() const;
    double cpuUsage() const;
    int threadCount() const;
    int fileHandleCount() const;
    
    // Security enforcement
    void enforceResourceLimits();
    bool checkResourceUsage() const;

signals:
    void resourceLimitExceeded(ResourceType resourceType);
    void securityViolation(const QString& violation);

protected:
    // Note: setupChildProcess() is deprecated in newer Qt versions
    // Use setChildProcessModifier() instead if needed

private slots:
    void onResourceCheckTimer();

private:
    SecurityPolicy m_policy;
    ResourceLimits m_limits;
    QTimer* m_resourceCheckTimer;
    QElapsedTimer m_cpuTimer;
    qint64 m_lastCpuTime;
    
    void applyProcessRestrictions();
    void setupResourceMonitoring();
    qint64 getProcessMemoryUsage() const;
    double getProcessCpuUsage() const;
    int getProcessThreadCount() const;
};

// Resource monitor for tracking usage
class ResourceMonitor : public QObject {
    Q_OBJECT

public:
    explicit ResourceMonitor(const QString& processId, const ResourceLimits& limits, QObject* parent = nullptr);
    ~ResourceMonitor() override;

    // Monitoring control
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;
    void setMonitoringInterval(int milliseconds);
    int monitoringInterval() const;
    
    // Resource tracking
    ResourceLimits getCurrentUsage() const;
    ResourceLimits getMaxUsage() const;
    ResourceLimits getAverageUsage() const;
    void resetStatistics();
    
    // Limits management
    void setResourceLimits(const ResourceLimits& limits);
    ResourceLimits resourceLimits() const;
    bool isLimitExceeded(ResourceType resourceType) const;

signals:
    void resourceUsageUpdated(const ResourceLimits& usage);
    void resourceLimitExceeded(ResourceType resourceType, double currentValue, double limitValue);
    void monitoringStarted();
    void monitoringStopped();

private slots:
    void onMonitoringTimer();

private:
    QString m_processId;
    ResourceLimits m_limits;
    ResourceLimits m_currentUsage;
    ResourceLimits m_maxUsage;
    ResourceLimits m_totalUsage;
    QTimer* m_monitoringTimer;
    int m_sampleCount;
    bool m_isMonitoring;
    
    void collectResourceUsage();
    void updateStatistics();
    void checkLimits();
};

// Isolation container for advanced sandboxing
class IsolationContainer : public QObject {
    Q_OBJECT

public:
    explicit IsolationContainer(const QString& containerId, const SecurityPolicy& policy, QObject* parent = nullptr);
    ~IsolationContainer() override;

    // Container management
    QString containerId() const;
    bool createContainer();
    void destroyContainer();
    bool isContainerActive() const;
    
    // File system isolation
    void mountFileSystem(const QString& hostPath, const QString& containerPath, bool readOnly = true);
    void unmountFileSystem(const QString& containerPath);
    QStringList mountedPaths() const;
    
    // Network isolation
    void enableNetworkIsolation(bool enable);
    bool isNetworkIsolationEnabled() const;
    void addAllowedHost(const QString& host);
    void removeAllowedHost(const QString& host);
    QStringList allowedHosts() const;
    
    // Process isolation
    void setProcessLimits(int maxProcesses);
    int processLimits() const;
    int currentProcessCount() const;
    
    // Environment variables
    void setEnvironmentVariable(const QString& name, const QString& value);
    void removeEnvironmentVariable(const QString& name);
    QStringList environmentVariables() const;

signals:
    void containerCreated();
    void containerDestroyed();
    void isolationViolation(const QString& violation);

private:
    QString m_containerId;
    SecurityPolicy m_policy;
    bool m_isActive;
    QMap<QString, QString> m_mountPoints;
    QStringList m_allowedHosts;
    QMap<QString, QString> m_environmentVars;
    int m_maxProcesses;
    bool m_networkIsolationEnabled;
    
    void setupFileSystemIsolation();
    void setupNetworkIsolation();
    void setupProcessIsolation();
    void cleanupContainer();
};

// Sandbox manager widget
class SandboxManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit SandboxManagerWidget(PluginSandboxManager* manager, QWidget* parent = nullptr);
    ~SandboxManagerWidget() override;

    // Display management
    void refreshSandboxList();
    void refreshViolationList();
    void refreshPolicyList();
    void showSandboxDetails(const QString& sandboxId);
    void showViolationDetails(const SandboxViolation& violation);

signals:
    void sandboxSelected(const QString& sandboxId);
    void policySelected(const QString& policyName);
    void violationSelected(const SandboxViolation& violation);
    void createSandboxRequested();
    void destroySandboxRequested(const QString& sandboxId);

private slots:
    void onSandboxItemClicked();
    void onViolationItemClicked();
    void onPolicyItemClicked();
    void onCreateSandboxClicked();
    void onDestroySandboxClicked();
    void onRefreshClicked();

private:
    PluginSandboxManager* m_manager;
    QTreeWidget* m_sandboxTree;
    QTableWidget* m_violationTable;
    QListWidget* m_policyList;
    QTextEdit* m_detailsView;
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    void setupUI();
    void populateSandboxTree();
    void populateViolationTable();
    void populatePolicyList();
    void updateDetailsView(const QString& content);
    QTreeWidgetItem* createSandboxItem(const QString& sandboxId);
    void addViolationRow(const SandboxViolation& violation);
};

// Security policy editor dialog
class SecurityPolicyDialog : public QDialog {
    Q_OBJECT

public:
    explicit SecurityPolicyDialog(const SecurityPolicy& policy = SecurityPolicy(), QWidget* parent = nullptr);
    ~SecurityPolicyDialog() override;

    // Policy management
    SecurityPolicy getSecurityPolicy() const;
    void setSecurityPolicy(const SecurityPolicy& policy);

public slots:
    void accept() override;
    void reject() override;

signals:
    void policyChanged(const SecurityPolicy& policy);

private slots:
    void onSecurityLevelChanged();
    void onSandboxTypeChanged();
    void onPermissionChanged();
    void onResourceLimitChanged();
    void onPathListChanged();
    void onPresetSelected();

private:
    SecurityPolicy m_policy;
    QComboBox* m_securityLevelCombo;
    QComboBox* m_sandboxTypeCombo;
    QMap<PermissionType, QCheckBox*> m_permissionChecks;
    QMap<QString, QSpinBox*> m_resourceSpins;
    QListWidget* m_allowedPathsList;
    QListWidget* m_blockedPathsList;
    QListWidget* m_allowedHostsList;
    QListWidget* m_blockedHostsList;
    
    void setupUI();
    void setupPermissions();
    void setupResourceLimits();
    void setupPathLists();
    void updateUIFromPolicy();
    void updatePolicyFromUI();
    void loadPresets();
};
