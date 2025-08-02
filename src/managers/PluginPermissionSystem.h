// PluginPermissionSystem.h - Granular Permission Management for Plugin Capabilities
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
#include <QDateTimeEdit>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

// Forward declarations
class PluginPermissionManager;
class PermissionGroup;
class PermissionRequest;
class PermissionAudit;
class PermissionPolicy;
class PermissionDialog;

// Permission categories
enum class PermissionCategory {
    FileSystem,     // File and directory access
    Network,        // Network communication
    System,         // System API access
    Hardware,       // Hardware device access
    UI,             // User interface manipulation
    Data,           // Data access and storage
    Process,        // Process management
    Registry,       // Windows registry (Windows only)
    Security,       // Security-related operations
    Custom          // Custom application-specific permissions
};

// Permission levels
enum class PermissionLevel {
    None,           // No access
    Read,           // Read-only access
    Write,          // Write access
    Execute,        // Execute/run access
    Full,           // Full access
    Admin           // Administrative access
};

// Permission scope
enum class PermissionScope {
    Global,         // System-wide permission
    User,           // User-specific permission
    Session,        // Session-specific permission
    Temporary,      // Temporary permission
    Conditional     // Conditional permission
};

// Permission status
enum class PermissionStatus {
    Granted,        // Permission granted
    Denied,         // Permission denied
    Pending,        // Permission request pending
    Revoked,        // Permission revoked
    Expired,        // Permission expired
    Conditional     // Permission granted with conditions
};

// Specific permission types
enum class PermissionType {
    // File System
    FileRead,
    FileWrite,
    FileExecute,
    DirectoryList,
    DirectoryCreate,
    DirectoryDelete,
    
    // Network
    NetworkConnect,
    NetworkListen,
    NetworkSend,
    NetworkReceive,
    
    // System
    SystemInfo,
    SystemSettings,
    SystemServices,
    SystemRegistry,
    
    // Hardware
    Camera,
    Microphone,
    GPS,
    Bluetooth,
    USB,
    Printer,
    
    // UI
    WindowCreate,
    WindowManipulate,
    Clipboard,
    Notifications,
    SystemTray,
    
    // Data
    DatabaseAccess,
    ConfigurationAccess,
    UserDataAccess,
    TempDataAccess,
    
    // Process
    ProcessCreate,
    ProcessTerminate,
    ProcessMonitor,
    ThreadCreate,
    
    // Security
    CertificateAccess,
    EncryptionKeys,
    PasswordManager,
    
    // Custom
    CustomPermission
};

// Permission definition
struct Permission {
    QString id;
    QString name;
    QString description;
    PermissionCategory category;
    PermissionType type;
    PermissionLevel defaultLevel;
    PermissionScope scope;
    bool isRequired;
    bool isDangerous;
    QStringList dependencies;
    QStringList conflicts;
    QJsonObject metadata;
    
    Permission() = default;
    Permission(const QString& i, const QString& n, PermissionCategory cat, PermissionType t)
        : id(i), name(n), category(cat), type(t), defaultLevel(PermissionLevel::None), 
          scope(PermissionScope::User), isRequired(false), isDangerous(false) {}
    
    QString getCategoryString() const;
    QString getTypeString() const;
    QString getLevelString() const;
    QString getScopeString() const;
};

// Permission grant information
struct PermissionGrant {
    QString permissionId;
    QString pluginId;
    QString userId;
    PermissionLevel level;
    PermissionScope scope;
    PermissionStatus status;
    QDateTime grantedDate;
    QDateTime expiryDate;
    QString grantedBy;
    QString reason;
    QStringList conditions;
    int usageCount;
    QDateTime lastUsed;
    QJsonObject metadata;
    
    PermissionGrant() = default;
    PermissionGrant(const QString& permId, const QString& plugId, PermissionLevel lvl)
        : permissionId(permId), pluginId(plugId), level(lvl), scope(PermissionScope::User),
          status(PermissionStatus::Granted), grantedDate(QDateTime::currentDateTime()), usageCount(0) {}
    
    bool isValid() const;
    bool isExpired() const;
    bool hasConditions() const;
    void recordUsage();
};

// Permission request
struct PermissionRequest {
    QString id;
    QString pluginId;
    QString permissionId;
    PermissionLevel requestedLevel;
    QString justification;
    QDateTime requestDate;
    PermissionStatus status;
    QString responseReason;
    QDateTime responseDate;
    QString respondedBy;
    bool isUrgent;
    QStringList alternatives;
    QJsonObject metadata;
    
    PermissionRequest() = default;
    PermissionRequest(const QString& plugId, const QString& permId, PermissionLevel level, const QString& just)
        : pluginId(plugId), permissionId(permId), requestedLevel(level), justification(just),
          requestDate(QDateTime::currentDateTime()), status(PermissionStatus::Pending), isUrgent(false) {
        id = generateRequestId();
    }
    
    bool isPending() const { return status == PermissionStatus::Pending; }
    bool isApproved() const { return status == PermissionStatus::Granted; }
    bool isDenied() const { return status == PermissionStatus::Denied; }
    
private:
    QString generateRequestId() const;
};

// Permission audit entry
struct PermissionAuditEntry {
    QString id;
    QString pluginId;
    QString permissionId;
    QString action; // granted, denied, revoked, used, etc.
    PermissionLevel level;
    QDateTime timestamp;
    QString userId;
    QString details;
    QString ipAddress;
    QString sessionId;
    QJsonObject metadata;
    
    PermissionAuditEntry() = default;
    PermissionAuditEntry(const QString& plugId, const QString& permId, const QString& act)
        : pluginId(plugId), permissionId(permId), action(act), timestamp(QDateTime::currentDateTime()) {
        id = generateAuditId();
    }
    
private:
    QString generateAuditId() const;
};

// Main permission manager
class PluginPermissionManager : public QObject {
    Q_OBJECT

public:
    explicit PluginPermissionManager(QObject* parent = nullptr);
    ~PluginPermissionManager() override;

    // Permission registration
    void registerPermission(const Permission& permission);
    void unregisterPermission(const QString& permissionId);
    Permission getPermission(const QString& permissionId) const;
    QList<Permission> getAllPermissions() const;
    QList<Permission> getPermissionsByCategory(PermissionCategory category) const;
    
    // Permission requests
    QString requestPermission(const QString& pluginId, const QString& permissionId, PermissionLevel level, const QString& justification = "");
    void approveRequest(const QString& requestId, const QString& reason = "");
    void denyRequest(const QString& requestId, const QString& reason = "");
    QList<PermissionRequest> getPendingRequests() const;
    QList<PermissionRequest> getRequestHistory(const QString& pluginId = "") const;
    
    // Permission grants
    void grantPermission(const QString& pluginId, const QString& permissionId, PermissionLevel level, PermissionScope scope = PermissionScope::User);
    void revokePermission(const QString& pluginId, const QString& permissionId);
    void revokeAllPermissions(const QString& pluginId);
    QList<PermissionGrant> getGrantedPermissions(const QString& pluginId) const;
    QList<PermissionGrant> getAllGrants() const;
    
    // Permission checking
    bool hasPermission(const QString& pluginId, const QString& permissionId, PermissionLevel requiredLevel = PermissionLevel::Read) const;
    PermissionLevel getPermissionLevel(const QString& pluginId, const QString& permissionId) const;
    PermissionStatus checkPermission(const QString& pluginId, const QString& permissionId, PermissionLevel requiredLevel = PermissionLevel::Read) const;
    bool canPerformAction(const QString& pluginId, const QString& action, const QJsonObject& context = QJsonObject()) const;
    
    // Permission policies
    void setPermissionPolicy(const QString& policyName, const QJsonObject& policy);
    QJsonObject getPermissionPolicy(const QString& policyName) const;
    void removePermissionPolicy(const QString& policyName);
    QStringList getAvailablePolicies() const;
    void applyPolicy(const QString& pluginId, const QString& policyName);
    
    // Bulk operations
    void grantPermissionSet(const QString& pluginId, const QStringList& permissionIds, PermissionLevel level);
    void revokePermissionSet(const QString& pluginId, const QStringList& permissionIds);
    void copyPermissions(const QString& fromPluginId, const QString& toPluginId);
    void resetPermissions(const QString& pluginId);
    
    // Audit and logging
    QList<PermissionAuditEntry> getAuditLog(const QString& pluginId = "", int maxEntries = 1000) const;
    void clearAuditLog(const QString& pluginId = "");
    void exportAuditLog(const QString& filePath, const QString& format = "json") const;
    
    // Configuration
    void setDefaultPermissionLevel(PermissionCategory category, PermissionLevel level);
    PermissionLevel getDefaultPermissionLevel(PermissionCategory category) const;
    void setRequireExplicitGrant(bool require);
    bool requireExplicitGrant() const;
    void setAuditingEnabled(bool enabled);
    bool isAuditingEnabled() const;
    void setAutoApproveLevel(PermissionLevel level);
    PermissionLevel autoApproveLevel() const;

signals:
    void permissionRequested(const PermissionRequest& request);
    void permissionGranted(const QString& pluginId, const QString& permissionId, PermissionLevel level);
    void permissionRevoked(const QString& pluginId, const QString& permissionId);
    void permissionDenied(const QString& pluginId, const QString& permissionId, const QString& reason);
    void permissionUsed(const QString& pluginId, const QString& permissionId);
    void requestApproved(const QString& requestId);
    void requestDenied(const QString& requestId, const QString& reason);
    void auditEntryAdded(const PermissionAuditEntry& entry);
    void policyApplied(const QString& pluginId, const QString& policyName);

public slots:
    void refreshPermissions();
    void cleanupExpiredGrants();
    void showPermissionManager();
    void showPermissionDialog(const QString& pluginId);

private slots:
    void onCleanupTimer();
    void onPermissionUsed(const QString& pluginId, const QString& permissionId);

private:
    
    void initializeManager();
    void loadConfiguration();
    void saveConfiguration();
    void loadPermissions();
    void savePermissions();
    void loadGrants();
    void saveGrants();
    void setupCleanupTimer();
    void createDefaultPermissions();
    void createDefaultPolicies();
    void logAuditEntry(const PermissionAuditEntry& entry);
    bool evaluateConditions(const QStringList& conditions, const QString& pluginId, const QJsonObject& context) const;
    QString generateGrantId() const;
};

// Permission group for organizing related permissions
class PermissionGroup : public QObject {
    Q_OBJECT

public:
    explicit PermissionGroup(const QString& name, const QString& description = "", QObject* parent = nullptr);
    ~PermissionGroup() override;

    // Group management
    QString name() const;
    void setName(const QString& name);
    QString description() const;
    void setDescription(const QString& description);
    
    // Permission management
    void addPermission(const Permission& permission);
    void removePermission(const QString& permissionId);
    Permission getPermission(const QString& permissionId) const;
    QList<Permission> getPermissions() const;
    bool hasPermission(const QString& permissionId) const;
    int permissionCount() const;
    
    // Group operations
    void grantAllPermissions(const QString& pluginId, PermissionLevel level);
    void revokeAllPermissions(const QString& pluginId);
    QStringList getPermissionIds() const;
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

signals:
    void permissionAdded(const QString& permissionId);
    void permissionRemoved(const QString& permissionId);
    void groupChanged();

private:
    QString m_name;
    QString m_description;
    QMap<QString, Permission> m_permissions;
};

// Permission policy for defining permission sets
class PermissionPolicy : public QObject {
    Q_OBJECT

public:
    explicit PermissionPolicy(const QString& name, QObject* parent = nullptr);
    ~PermissionPolicy() override;

    // Policy management
    QString name() const;
    void setName(const QString& name);
    QString description() const;
    void setDescription(const QString& description);
    
    // Permission rules
    void addPermissionRule(const QString& permissionId, PermissionLevel level, PermissionScope scope = PermissionScope::User);
    void removePermissionRule(const QString& permissionId);
    void setPermissionLevel(const QString& permissionId, PermissionLevel level);
    PermissionLevel getPermissionLevel(const QString& permissionId) const;
    QMap<QString, PermissionLevel> getPermissionRules() const;
    
    // Policy application
    void applyToPlugin(const QString& pluginId, PluginPermissionManager* manager);
    bool isApplicable(const QString& pluginId) const;
    void addApplicabilityCondition(const QString& condition);
    void removeApplicabilityCondition(const QString& condition);
    QStringList getApplicabilityConditions() const;
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

signals:
    void policyChanged();
    void ruleAdded(const QString& permissionId, PermissionLevel level);
    void ruleRemoved(const QString& permissionId);

private:
    QString m_name;
    QString m_description;
    QMap<QString, PermissionLevel> m_permissionRules;
    QMap<QString, PermissionScope> m_permissionScopes;
    QStringList m_applicabilityConditions;
};

// Permission manager widget
class PermissionManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit PermissionManagerWidget(PluginPermissionManager* manager, QWidget* parent = nullptr);
    ~PermissionManagerWidget() override;

    // Display management
    void refreshPermissionList();
    void refreshGrantsList();
    void refreshRequestsList();
    void refreshAuditLog();
    void showPluginPermissions(const QString& pluginId);

signals:
    void permissionSelected(const QString& permissionId);
    void grantSelected(const PermissionGrant& grant);
    void requestSelected(const PermissionRequest& request);
    void grantPermissionRequested(const QString& pluginId, const QString& permissionId);
    void revokePermissionRequested(const QString& pluginId, const QString& permissionId);
    void approveRequestRequested(const QString& requestId);
    void denyRequestRequested(const QString& requestId);

private slots:
    void onPermissionItemClicked();
    void onGrantItemClicked();
    void onRequestItemClicked();
    void onGrantButtonClicked();
    void onRevokeButtonClicked();
    void onApproveButtonClicked();
    void onDenyButtonClicked();
    void onRefreshClicked();

private:
    PluginPermissionManager* m_manager;
    QTabWidget* m_tabWidget;
    QTreeWidget* m_permissionTree;
    QTableWidget* m_grantsTable;
    QTableWidget* m_requestsTable;
    QTableWidget* m_auditTable;
    QTextEdit* m_detailsView;
    QSplitter* m_splitter;
    
    void setupUI();
    void setupPermissionTab();
    void setupGrantsTab();
    void setupRequestsTab();
    void setupAuditTab();
    void populatePermissionTree();
    void populateGrantsTable();
    void populateRequestsTable();
    void populateAuditTable();
    void updateDetailsView(const QString& content);
    QTreeWidgetItem* createPermissionItem(const Permission& permission);
    void addGrantRow(const PermissionGrant& grant);
    void addRequestRow(const PermissionRequest& request);
    void addAuditRow(const PermissionAuditEntry& entry);
};

// Permission request dialog
class PermissionDialog : public QDialog {
    Q_OBJECT

public:
    explicit PermissionDialog(const PermissionRequest& request, QWidget* parent = nullptr);
    ~PermissionDialog() override;

    // Request information
    PermissionRequest getRequest() const;
    QString getApprovalReason() const;
    QString getDenialReason() const;
    bool isApproved() const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void requestApproved(const QString& requestId, const QString& reason);
    void requestDenied(const QString& requestId, const QString& reason);

private slots:
    void onApproveClicked();
    void onDenyClicked();
    void onDetailsToggled();

private:
    PermissionRequest m_request;
    QLabel* m_pluginLabel;
    QLabel* m_permissionLabel;
    QLabel* m_levelLabel;
    QTextEdit* m_justificationEdit;
    QTextEdit* m_reasonEdit;
    QPushButton* m_approveButton;
    QPushButton* m_denyButton;
    QPushButton* m_detailsButton;
    QWidget* m_detailsWidget;
    bool m_approved;
    bool m_detailsVisible;
    
    void setupUI();
    void updateRequestInfo();
    void showDetails();
    void hideDetails();
};

// Permission grant dialog
class PermissionGrantDialog : public QDialog {
    Q_OBJECT

public:
    explicit PermissionGrantDialog(const QString& pluginId, const QList<Permission>& availablePermissions, QWidget* parent = nullptr);
    ~PermissionGrantDialog() override;

    // Grant configuration
    QStringList getSelectedPermissions() const;
    PermissionLevel getPermissionLevel() const;
    PermissionScope getPermissionScope() const;
    QDateTime getExpiryDate() const;
    bool hasExpiry() const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void permissionsGranted(const QString& pluginId, const QStringList& permissionIds, PermissionLevel level, PermissionScope scope);

private slots:
    void onPermissionToggled();
    void onLevelChanged();
    void onScopeChanged();
    void onExpiryToggled();
    void onSelectAllClicked();
    void onSelectNoneClicked();

private:
    QString m_pluginId;
    QList<Permission> m_availablePermissions;
    QMap<QString, QCheckBox*> m_permissionChecks;
    QComboBox* m_levelCombo;
    QComboBox* m_scopeCombo;
    QCheckBox* m_expiryCheck;
    QDateTimeEdit* m_expiryEdit;
    QPushButton* m_selectAllButton;
    QPushButton* m_selectNoneButton;
    
    void setupUI();
    void setupPermissionList();
    void updateGrantButton();
};

// Utility functions for permission management
namespace PermissionUtils {
    // Permission validation
    bool isValidPermissionId(const QString& permissionId);
    bool isValidPermissionLevel(PermissionLevel level, PermissionCategory category);
    bool arePermissionsCompatible(const QString& permission1, const QString& permission2);
    
    // Permission conversion
    QString permissionCategoryToString(PermissionCategory category);
    PermissionCategory permissionCategoryFromString(const QString& categoryStr);
    QString permissionLevelToString(PermissionLevel level);
    PermissionLevel permissionLevelFromString(const QString& levelStr);
    QString permissionScopeToString(PermissionScope scope);
    PermissionScope permissionScopeFromString(const QString& scopeStr);
    QString permissionStatusToString(PermissionStatus status);
    PermissionStatus permissionStatusFromString(const QString& statusStr);
    
    // Permission analysis
    QStringList getDependentPermissions(const QString& permissionId, const QList<Permission>& allPermissions);
    QStringList getConflictingPermissions(const QString& permissionId, const QList<Permission>& allPermissions);
    int calculatePermissionRisk(const Permission& permission);
    QString getPermissionDescription(const Permission& permission);
    
    // Policy helpers
    QJsonObject createDefaultPolicy(const QString& policyName, PermissionLevel defaultLevel);
    bool validatePolicy(const QJsonObject& policy);
    QStringList getPolicyValidationErrors(const QJsonObject& policy);
    
    // Audit helpers
    QString formatAuditEntry(const PermissionAuditEntry& entry);
    QStringList generateAuditReport(const QList<PermissionAuditEntry>& entries);
    void exportAuditToCSV(const QList<PermissionAuditEntry>& entries, const QString& filePath);
    void exportAuditToJSON(const QList<PermissionAuditEntry>& entries, const QString& filePath);
}
