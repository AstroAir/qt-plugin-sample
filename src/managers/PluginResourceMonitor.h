// PluginResourceMonitor.h - Advanced Resource Monitoring and Management System
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
#include <QElapsedTimer>
#include <QDateTime>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QAtomicInt>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>

// Forward declarations
class PluginResourceMonitor;
class ResourceTracker;
class ResourceLimitEnforcer;
class ResourceAllocator;
class ResourceAnalyzer;
class ResourceDashboard;
class ResourceChart;

// Resource types
enum class ResourceType {
    Memory,         // RAM usage
    CPU,            // CPU utilization
    Disk,           // Disk space and I/O
    Network,        // Network bandwidth
    Handles,        // File/system handles
    Threads,        // Thread count
    Processes,      // Process count
    GPU,            // GPU usage (if available)
    Battery,        // Battery consumption
    Custom          // Custom resource types
};

// Resource units
enum class ResourceUnit {
    Bytes,          // Memory, disk space
    Percentage,     // CPU, GPU utilization
    Count,          // Handles, threads, processes
    BytesPerSecond, // Network, disk I/O
    Hertz,          // Frequency
    Watts,          // Power consumption
    Custom          // Custom units
};

// Resource limit types
enum class LimitType {
    Hard,           // Hard limit (cannot exceed)
    Soft,           // Soft limit (warning when exceeded)
    Adaptive,       // Adaptive limit (adjusts based on system load)
    Percentage,     // Percentage of system resources
    Absolute        // Absolute value
};

// Resource allocation strategy
enum class AllocationStrategy {
    FirstFit,       // First available resource
    BestFit,        // Best matching resource
    WorstFit,       // Worst matching resource
    RoundRobin,     // Round-robin allocation
    Priority,       // Priority-based allocation
    LoadBalanced    // Load-balanced allocation
};

// Resource usage sample
struct ResourceSample {
    QString pluginId;
    ResourceType type;
    QDateTime timestamp;
    double value;
    ResourceUnit unit;
    double systemTotal;
    double systemAvailable;
    QJsonObject metadata;
    
    ResourceSample() = default;
    ResourceSample(const QString& id, ResourceType t, double v, ResourceUnit u)
        : pluginId(id), type(t), timestamp(QDateTime::currentDateTime()), value(v), unit(u) {}
    
    double getPercentageOfSystem() const;
    QString getFormattedValue() const;
    QString getUnitString() const;
};

// Resource limit definition
struct ResourceLimit {
    QString pluginId;
    ResourceType type;
    LimitType limitType;
    double value;
    ResourceUnit unit;
    QString description;
    bool isEnabled;
    QDateTime createdDate;
    QDateTime lastModified;
    QString createdBy;
    QJsonObject metadata;
    
    ResourceLimit() = default;
    ResourceLimit(const QString& id, ResourceType t, double v, ResourceUnit u, LimitType lt = LimitType::Hard)
        : pluginId(id), type(t), limitType(lt), value(v), unit(u), isEnabled(true), 
          createdDate(QDateTime::currentDateTime()), lastModified(QDateTime::currentDateTime()) {}
    
    bool isExceeded(double currentValue) const;
    QString getFormattedLimit() const;
    QString getLimitTypeString() const;
};

// Resource allocation record
struct ResourceAllocation {
    QString allocationId;
    QString pluginId;
    ResourceType type;
    double allocatedAmount;
    double usedAmount;
    ResourceUnit unit;
    AllocationStrategy strategy;
    QDateTime allocationTime;
    QDateTime lastAccessed;
    bool isActive;
    QString description;
    QJsonObject metadata;
    
    ResourceAllocation() = default;
    ResourceAllocation(const QString& id, ResourceType t, double amount, ResourceUnit u)
        : pluginId(id), type(t), allocatedAmount(amount), usedAmount(0), unit(u),
          strategy(AllocationStrategy::FirstFit), allocationTime(QDateTime::currentDateTime()),
          lastAccessed(QDateTime::currentDateTime()), isActive(true) {
        allocationId = generateAllocationId();
    }
    
    double getUtilizationPercentage() const;
    bool isUnderUtilized(double threshold = 0.1) const;
    bool isOverAllocated() const;
    
private:
    QString generateAllocationId() const;
};

// Resource alert
struct ResourceAlert {
    QString alertId;
    QString pluginId;
    ResourceType type;
    QString severity; // Low, Medium, High, Critical
    QString message;
    QString description;
    double currentValue;
    double thresholdValue;
    QDateTime timestamp;
    bool isActive;
    bool isAcknowledged;
    QString acknowledgedBy;
    QDateTime acknowledgedTime;
    QJsonObject metadata;
    
    ResourceAlert() = default;
    ResourceAlert(const QString& id, ResourceType t, const QString& msg, double current, double threshold)
        : pluginId(id), type(t), message(msg), currentValue(current), thresholdValue(threshold),
          timestamp(QDateTime::currentDateTime()), isActive(true), isAcknowledged(false) {
        alertId = generateAlertId();
        severity = determineSeverity(current, threshold);
    }
    
private:
    QString generateAlertId() const;
    QString determineSeverity(double current, double threshold) const;
};

// Main resource monitor
class PluginResourceMonitor : public QObject {
    Q_OBJECT

public:
    explicit PluginResourceMonitor(QObject* parent = nullptr);
    ~PluginResourceMonitor() override;

    // Monitoring control
    void startMonitoring();
    void stopMonitoring();
    void pauseMonitoring();
    void resumeMonitoring();
    bool isMonitoring() const;
    bool isPaused() const;
    
    // Plugin management
    void addPlugin(const QString& pluginId);
    void removePlugin(const QString& pluginId);
    QStringList monitoredPlugins() const;
    void enablePluginMonitoring(const QString& pluginId, bool enable);
    bool isPluginMonitoringEnabled(const QString& pluginId) const;
    
    // Resource tracking
    void recordResourceUsage(const ResourceSample& sample);
    QList<ResourceSample> getResourceHistory(const QString& pluginId, ResourceType type, int maxSamples = -1) const;
    QList<ResourceSample> getResourceHistory(const QString& pluginId, const QDateTime& from, const QDateTime& to) const;
    ResourceSample getCurrentUsage(const QString& pluginId, ResourceType type) const;
    QMap<ResourceType, ResourceSample> getAllCurrentUsage(const QString& pluginId) const;
    
    // Resource limits
    void setResourceLimit(const ResourceLimit& limit);
    void removeResourceLimit(const QString& pluginId, ResourceType type);
    ResourceLimit getResourceLimit(const QString& pluginId, ResourceType type) const;
    QList<ResourceLimit> getAllResourceLimits(const QString& pluginId) const;
    bool isResourceLimitExceeded(const QString& pluginId, ResourceType type) const;
    
    // Resource allocation
    QString allocateResource(const QString& pluginId, ResourceType type, double amount, ResourceUnit unit, AllocationStrategy strategy = AllocationStrategy::FirstFit);
    void deallocateResource(const QString& allocationId);
    void updateResourceUsage(const QString& allocationId, double usedAmount);
    ResourceAllocation getResourceAllocation(const QString& allocationId) const;
    QList<ResourceAllocation> getPluginAllocations(const QString& pluginId) const;
    QList<ResourceAllocation> getAllAllocations() const;
    
    // System resource information
    double getSystemResourceTotal(ResourceType type) const;
    double getSystemResourceAvailable(ResourceType type) const;
    double getSystemResourceUsed(ResourceType type) const;
    double getSystemResourceUsagePercentage(ResourceType type) const;
    QMap<ResourceType, double> getSystemResourceSummary() const;
    
    // Alerts and notifications
    QList<ResourceAlert> getActiveAlerts() const;
    QList<ResourceAlert> getAlerts(const QString& pluginId) const;
    void acknowledgeAlert(const QString& alertId);
    void dismissAlert(const QString& alertId);
    void setAlertThreshold(ResourceType type, double threshold);
    double getAlertThreshold(ResourceType type) const;
    
    // Analysis and optimization
    QStringList analyzeResourceUsage(const QString& pluginId) const;
    QStringList getOptimizationRecommendations(const QString& pluginId) const;
    QMap<QString, double> getResourceEfficiencyScores() const;
    QStringList identifyResourceBottlenecks() const;
    
    // Configuration
    void setSamplingInterval(int milliseconds);
    int samplingInterval() const;
    void setMaxHistorySize(int maxSamples);
    int maxHistorySize() const;
    void setAlertingEnabled(bool enabled);
    bool isAlertingEnabled() const;
    void setAutoOptimizationEnabled(bool enabled);
    bool isAutoOptimizationEnabled() const;

signals:
    void monitoringStarted();
    void monitoringStopped();
    void monitoringPaused();
    void monitoringResumed();
    void resourceSampleRecorded(const ResourceSample& sample);
    void resourceLimitExceeded(const QString& pluginId, ResourceType type, double currentValue, double limitValue);
    void resourceAllocated(const QString& allocationId, const QString& pluginId, ResourceType type);
    void resourceDeallocated(const QString& allocationId);
    void alertTriggered(const ResourceAlert& alert);
    void alertResolved(const QString& alertId);
    void optimizationRecommendationAvailable(const QString& pluginId, const QString& recommendation);

public slots:
    void clearHistory();
    void clearHistory(const QString& pluginId);
    void optimizeResourceUsage();
    void optimizeResourceUsage(const QString& pluginId);
    void showResourceDashboard();

private slots:
    void onSamplingTimer();
    void onAlertCheckTimer();
    void onOptimizationTimer();
    void onCleanupTimer();

private:
    
    void initializeMonitor();
    void loadConfiguration();
    void saveConfiguration();
    void setupTimers();
    void collectResourceSamples();
    void collectSystemResourceSample(ResourceType type);
    void collectPluginResourceSample(const QString& pluginId, ResourceType type);
    void checkResourceLimits();
    void checkResourceAlerts();
    void performOptimization();
    void cleanupOldData();
    void updateResourceStatistics();
    QString generateSampleId() const;
};

// Resource tracker for individual plugins
class ResourceTracker : public QObject {
    Q_OBJECT

public:
    explicit ResourceTracker(const QString& pluginId, QObject* parent = nullptr);
    ~ResourceTracker() override;

    // Tracking control
    void startTracking();
    void stopTracking();
    bool isTracking() const;
    QString pluginId() const;
    
    // Resource measurement
    double getCurrentMemoryUsage() const;
    double getCurrentCPUUsage() const;
    double getCurrentDiskUsage() const;
    double getCurrentNetworkUsage() const;
    int getCurrentHandleCount() const;
    int getCurrentThreadCount() const;
    
    // Custom resource tracking
    void addCustomResource(const QString& resourceName, ResourceType type, ResourceUnit unit);
    void removeCustomResource(const QString& resourceName);
    void updateCustomResource(const QString& resourceName, double value);
    double getCustomResourceValue(const QString& resourceName) const;
    QStringList getCustomResourceNames() const;
    
    // Statistics
    double getAverageUsage(ResourceType type) const;
    double getPeakUsage(ResourceType type) const;
    double getMinimumUsage(ResourceType type) const;
    QDateTime getPeakUsageTime(ResourceType type) const;
    void resetStatistics();

signals:
    void trackingStarted();
    void trackingStopped();
    void resourceUsageChanged(ResourceType type, double value);
    void customResourceUpdated(const QString& resourceName, double value);

private slots:
    void onTrackingTimer();

private:
    QString m_pluginId;
    QTimer* m_trackingTimer;
    bool m_isTracking;
    QMap<ResourceType, QQueue<double>> m_usageHistory;
    QMap<ResourceType, double> m_peakUsage;
    QMap<ResourceType, QDateTime> m_peakUsageTime;
    QMap<QString, double> m_customResources;
    QMap<QString, ResourceType> m_customResourceTypes;
    QMap<QString, ResourceUnit> m_customResourceUnits;
    
    void collectResourceData();
    double measureMemoryUsage() const;
    double measureCPUUsage() const;
    double measureDiskUsage() const;
    double measureNetworkUsage() const;
    int measureHandleCount() const;
    int measureThreadCount() const;
    void updateStatistics(ResourceType type, double value);
};

// Resource limit enforcer
class ResourceLimitEnforcer : public QObject {
    Q_OBJECT

public:
    explicit ResourceLimitEnforcer(QObject* parent = nullptr);
    ~ResourceLimitEnforcer() override;

    // Enforcement control
    void enableEnforcement(bool enable);
    bool isEnforcementEnabled() const;
    void setEnforcementMode(const QString& mode); // strict, lenient, adaptive
    QString enforcementMode() const;
    
    // Limit management
    void addLimit(const ResourceLimit& limit);
    void removeLimit(const QString& pluginId, ResourceType type);
    void updateLimit(const ResourceLimit& limit);
    ResourceLimit getLimit(const QString& pluginId, ResourceType type) const;
    QList<ResourceLimit> getAllLimits() const;
    
    // Enforcement actions
    void checkLimits(const QString& pluginId, const QMap<ResourceType, double>& currentUsage);
    void enforceLimit(const QString& pluginId, ResourceType type, double currentValue, double limitValue);
    QStringList getAvailableActions(ResourceType type) const;
    void setDefaultAction(ResourceType type, const QString& action);
    QString getDefaultAction(ResourceType type) const;
    
    // Violation tracking
    struct LimitViolation {
        QString pluginId;
        ResourceType type;
        double currentValue;
        double limitValue;
        QDateTime timestamp;
        QString action;
        QString result;
    };
    
    QList<LimitViolation> getViolationHistory(const QString& pluginId = "") const;
    void clearViolationHistory(const QString& pluginId = "");
    int getViolationCount(const QString& pluginId, ResourceType type) const;

signals:
    void limitExceeded(const QString& pluginId, ResourceType type, double currentValue, double limitValue);
    void limitEnforced(const QString& pluginId, ResourceType type, const QString& action);
    void violationRecorded(const LimitViolation& violation);

private slots:
    void onEnforcementTimer();

private:
    bool m_enforcementEnabled;
    QString m_enforcementMode;
    QMap<QString, QMap<ResourceType, ResourceLimit>> m_limits;
    QMap<ResourceType, QString> m_defaultActions;
    QList<LimitViolation> m_violationHistory;
    QTimer* m_enforcementTimer;
    
    void performEnforcementAction(const QString& pluginId, ResourceType type, const QString& action);
    void throttlePlugin(const QString& pluginId);
    void suspendPlugin(const QString& pluginId);
    void terminatePlugin(const QString& pluginId);
    void logViolation(const LimitViolation& violation);
};

// Resource allocator for managing resource allocation
class ResourceAllocator : public QObject {
    Q_OBJECT

public:
    explicit ResourceAllocator(QObject* parent = nullptr);
    ~ResourceAllocator() override;

    // Allocation strategies
    void setAllocationStrategy(ResourceType type, AllocationStrategy strategy);
    AllocationStrategy getAllocationStrategy(ResourceType type) const;
    void setDefaultStrategy(AllocationStrategy strategy);
    AllocationStrategy defaultStrategy() const;
    
    // Resource pools
    void createResourcePool(const QString& poolName, ResourceType type, double totalAmount, ResourceUnit unit);
    void removeResourcePool(const QString& poolName);
    void resizeResourcePool(const QString& poolName, double newSize);
    QStringList getResourcePools() const;
    double getPoolUtilization(const QString& poolName) const;
    
    // Allocation operations
    QString allocateFromPool(const QString& poolName, const QString& pluginId, double amount);
    void deallocateFromPool(const QString& allocationId);
    bool canAllocate(const QString& poolName, double amount) const;
    double getAvailableInPool(const QString& poolName) const;
    
    // Allocation optimization
    void optimizeAllocations();
    void defragmentPool(const QString& poolName);
    QStringList getOptimizationSuggestions() const;
    void setAutoOptimization(bool enabled);
    bool isAutoOptimizationEnabled() const;

signals:
    void poolCreated(const QString& poolName);
    void poolRemoved(const QString& poolName);
    void poolResized(const QString& poolName, double oldSize, double newSize);
    void allocationSucceeded(const QString& allocationId);
    void allocationFailed(const QString& pluginId, const QString& reason);
    void optimizationCompleted();

private slots:
    void onOptimizationTimer();

private:
    struct ResourcePool {
        QString name;
        ResourceType type;
        double totalAmount;
        double allocatedAmount;
        ResourceUnit unit;
        AllocationStrategy strategy;
        QMap<QString, ResourceAllocation> allocations;
    };
    
    QMap<QString, ResourcePool> m_pools;
    QMap<ResourceType, AllocationStrategy> m_strategies;
    AllocationStrategy m_defaultStrategy;
    QTimer* m_optimizationTimer;
    bool m_autoOptimizationEnabled;
    
    QString findBestPool(ResourceType type, double amount) const;
    QString allocateUsingStrategy(ResourcePool& pool, const QString& pluginId, double amount, AllocationStrategy strategy);
    void compactPool(ResourcePool& pool);
    double calculateFragmentation(const ResourcePool& pool) const;
};

// Resource dashboard widget
class ResourceDashboard : public QWidget {
    Q_OBJECT

public:
    explicit ResourceDashboard(PluginResourceMonitor* monitor, QWidget* parent = nullptr);
    ~ResourceDashboard() override;

    // Display management
    void refreshDashboard();
    void setUpdateInterval(int milliseconds);
    int updateInterval() const;
    void setSelectedPlugin(const QString& pluginId);
    QString selectedPlugin() const;
    
    // View options
    void setShowSystemResources(bool show);
    bool showSystemResources() const;
    void setShowAlerts(bool show);
    bool showAlerts() const;
    void setShowHistory(bool show);
    bool showHistory() const;

signals:
    void pluginSelected(const QString& pluginId);
    void resourceTypeSelected(ResourceType type);
    void alertSelected(const QString& alertId);
    void optimizationRequested(const QString& pluginId);

private slots:
    void onUpdateTimer();
    void onPluginSelectionChanged();
    void onResourceTypeChanged();
    void onOptimizeClicked();
    void onRefreshClicked();

private:
    PluginResourceMonitor* m_monitor;
    QTimer* m_updateTimer;
    QString m_selectedPlugin;
    bool m_showSystemResources;
    bool m_showAlerts;
    bool m_showHistory;
    
    // UI components
    QComboBox* m_pluginCombo;
    QTabWidget* m_tabWidget;
    QWidget* m_overviewTab;
    QWidget* m_detailsTab;
    QWidget* m_alertsTab;
    QWidget* m_historyTab;
    
    // Charts and displays
    QMap<ResourceType, ResourceChart*> m_resourceCharts;
    QTableWidget* m_alertsTable;
    QTableWidget* m_historyTable;
    QLabel* m_summaryLabel;
    
    void setupUI();
    void setupOverviewTab();
    void setupDetailsTab();
    void setupAlertsTab();
    void setupHistoryTab();
    void updateOverview();
    void updateDetails();
    void updateAlerts();
    void updateHistory();
    void updateResourceChart(ResourceType type);
};

// Resource chart widget for visualizing usage
class ResourceChart : public QWidget {
    Q_OBJECT

public:
    explicit ResourceChart(ResourceType type, QWidget* parent = nullptr);
    ~ResourceChart() override;

    // Chart configuration
    ResourceType resourceType() const;
    void setResourceType(ResourceType type);
    void setTimeRange(int minutes);
    int timeRange() const;
    void setMaxDataPoints(int maxPoints);
    int maxDataPoints() const;
    
    // Data management
    void addDataPoint(double value, const QDateTime& timestamp = QDateTime::currentDateTime());
    void setData(const QList<ResourceSample>& samples);
    void clearData();
    QList<ResourceSample> getData() const;
    
    // Display options
    void setShowGrid(bool show);
    bool showGrid() const;
    void setShowLegend(bool show);
    bool showLegend() const;
    void setShowThreshold(bool show, double threshold = 0.0);
    bool showThreshold() const;
    double thresholdValue() const;

signals:
    void dataPointClicked(const ResourceSample& sample);
    void thresholdExceeded(double value);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    ResourceType m_resourceType;
    QList<ResourceSample> m_data;
    int m_timeRange;
    int m_maxDataPoints;
    bool m_showGrid;
    bool m_showLegend;
    bool m_showThreshold;
    double m_thresholdValue;
    
    void drawChart(QPainter* painter);
    void drawGrid(QPainter* painter);
    void drawData(QPainter* painter);
    void drawThreshold(QPainter* painter);
    void drawLegend(QPainter* painter);
    QPoint valueToPoint(double value, const QDateTime& timestamp) const;
    ResourceSample pointToValue(const QPoint& point) const;
    QString formatValue(double value) const;
};
