// PluginPerformanceProfiler.h - Real-time Plugin Performance Monitoring and Profiling
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QProgressBar>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QFrame>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QAtomicInt>
#include <QAtomicPointer>
#include <QSharedMemory>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <QStack>

// Forward declarations
class PluginPerformanceProfiler;
class PerformanceMetrics;
class PerformanceMonitor;
class PerformanceAnalyzer;
class PerformanceReporter;
class PerformanceOptimizer;
class ProfilerWidget;
class MetricsChart;

// Performance metric types
enum class MetricType {
    CPUUsage,           // CPU utilization percentage
    MemoryUsage,        // Memory consumption in bytes
    IOOperations,       // File I/O operations per second
    NetworkTraffic,     // Network bytes sent/received
    FunctionCalls,      // Function call frequency
    ExecutionTime,      // Function execution time
    ThreadCount,        // Number of active threads
    HandleCount,        // System handle count
    GCCollections,      // Garbage collection events
    Exceptions,         // Exception occurrences
    DatabaseQueries,    // Database query count/time
    CacheHitRate,       // Cache hit/miss ratio
    EventProcessing,    // Event processing rate
    UIResponsiveness,   // UI thread responsiveness
    Custom              // Custom user-defined metrics
};

// Performance alert levels
enum class AlertLevel {
    Info,
    Warning,
    Critical,
    Emergency
};

// Profiling modes
enum class ProfilingMode {
    Disabled,           // No profiling
    Basic,              // Basic metrics only
    Detailed,           // Detailed function-level profiling
    Comprehensive,      // Full system profiling
    Custom              // Custom profiling configuration
};

// Performance sample data
struct PerformanceSample {
    QString pluginId;
    MetricType type;
    QDateTime timestamp;
    double value;
    QString unit;
    QJsonObject metadata;
    
    PerformanceSample() = default;
    PerformanceSample(const QString& id, MetricType t, double v, const QString& u = "")
        : pluginId(id), type(t), timestamp(QDateTime::currentDateTime()), value(v), unit(u) {}
};

// Performance statistics
struct PerformanceStats {
    QString pluginId;
    MetricType type;
    double minimum = 0.0;
    double maximum = 0.0;
    double average = 0.0;
    double median = 0.0;
    double standardDeviation = 0.0;
    int sampleCount = 0;
    QDateTime firstSample;
    QDateTime lastSample;
    
    PerformanceStats() = default;
    PerformanceStats(const QString& id, MetricType t) : pluginId(id), type(t) {}
    
    void updateWith(double value);
    void reset();
};

// Performance alert
struct PerformanceAlert {
    QString id;
    QString pluginId;
    MetricType metricType;
    AlertLevel level;
    QString title;
    QString description;
    QString recommendation;
    QDateTime timestamp;
    double threshold;
    double actualValue;
    bool isActive = true;
    bool isAcknowledged = false;
    
    PerformanceAlert() = default;
    PerformanceAlert(const QString& pid, MetricType mt, AlertLevel al, const QString& desc)
        : pluginId(pid), metricType(mt), level(al), description(desc), timestamp(QDateTime::currentDateTime()) {}
};

// Performance configuration
struct PerformanceConfig {
    ProfilingMode mode = ProfilingMode::Basic;
    int samplingInterval = 1000; // milliseconds
    int maxSamples = 10000;
    bool enableCPUProfiling = true;
    bool enableMemoryProfiling = true;
    bool enableIOProfiling = false;
    bool enableNetworkProfiling = false;
    bool enableFunctionProfiling = false;
    bool enableAlerts = true;
    bool enableAutoOptimization = false;
    QString logDirectory;
    int logRetentionDays = 30;
    QMap<MetricType, double> alertThresholds;
    QStringList excludedPlugins;
    QStringList monitoredFunctions;
    
    PerformanceConfig() {
        // Set default alert thresholds
        alertThresholds[MetricType::CPUUsage] = 80.0; // 80%
        alertThresholds[MetricType::MemoryUsage] = 100 * 1024 * 1024; // 100MB
        alertThresholds[MetricType::ExecutionTime] = 1000.0; // 1 second
        alertThresholds[MetricType::ThreadCount] = 10.0; // 10 threads
    }
};

// Main performance profiler
class PluginPerformanceProfiler : public QObject {
    Q_OBJECT

public:
    explicit PluginPerformanceProfiler(QObject* parent = nullptr);
    ~PluginPerformanceProfiler() override;

    // Configuration
    void setConfiguration(const PerformanceConfig& config);
    PerformanceConfig configuration() const;
    void setProfilingMode(ProfilingMode mode);
    ProfilingMode profilingMode() const;
    void setSamplingInterval(int milliseconds);
    int samplingInterval() const;
    
    // Profiling control
    void startProfiling();
    void stopProfiling();
    void pauseProfiling();
    void resumeProfiling();
    bool isProfiling() const;
    bool isPaused() const;
    
    // Plugin monitoring
    void addPlugin(const QString& pluginId);
    void removePlugin(const QString& pluginId);
    QStringList monitoredPlugins() const;
    void enablePluginProfiling(const QString& pluginId, bool enable);
    bool isPluginProfilingEnabled(const QString& pluginId) const;
    
    // Metrics collection
    void recordSample(const PerformanceSample& sample);
    void recordSample(const QString& pluginId, MetricType type, double value, const QString& unit = "");
    QList<PerformanceSample> getSamples(const QString& pluginId, MetricType type, int maxSamples = -1) const;
    QList<PerformanceSample> getSamples(const QString& pluginId, const QDateTime& from, const QDateTime& to) const;
    
    // Statistics
    PerformanceStats getStatistics(const QString& pluginId, MetricType type) const;
    QMap<MetricType, PerformanceStats> getAllStatistics(const QString& pluginId) const;
    QStringList getTopPerformers(MetricType type, int count = 10) const;
    QStringList getBottomPerformers(MetricType type, int count = 10) const;
    
    // Alerts
    QList<PerformanceAlert> getActiveAlerts() const;
    QList<PerformanceAlert> getAlerts(const QString& pluginId) const;
    void acknowledgeAlert(const QString& alertId);
    void dismissAlert(const QString& alertId);
    void setAlertThreshold(MetricType type, double threshold);
    double getAlertThreshold(MetricType type) const;
    
    // Analysis and optimization
    QStringList analyzePerformance(const QString& pluginId) const;
    QStringList getOptimizationSuggestions(const QString& pluginId) const;
    void generatePerformanceReport(const QString& pluginId, const QString& filePath) const;
    void exportMetrics(const QString& filePath, const QString& format = "json") const;
    
    // System monitoring
    double getSystemCPUUsage() const;
    qint64 getSystemMemoryUsage() const;
    qint64 getAvailableMemory() const;
    int getSystemThreadCount() const;
    double getSystemLoadAverage() const;

signals:
    void profilingStarted();
    void profilingStopped();
    void profilingPaused();
    void profilingResumed();
    void sampleRecorded(const PerformanceSample& sample);
    void alertTriggered(const PerformanceAlert& alert);
    void alertResolved(const QString& alertId);
    void statisticsUpdated(const QString& pluginId, MetricType type);
    void optimizationSuggestionAvailable(const QString& pluginId, const QString& suggestion);

public slots:
    void clearMetrics();
    void clearMetrics(const QString& pluginId);
    void resetStatistics();
    void resetStatistics(const QString& pluginId);
    void showProfilerWidget();

private slots:
    void onSamplingTimer();
    void onAlertCheckTimer();
    void onCleanupTimer();

private:
    
    void initializeProfiler();
    void loadConfiguration();
    void saveConfiguration();
    void setupTimers();
    void collectSystemMetrics();
    void collectPluginMetrics(const QString& pluginId);
    void checkAlerts();
    void cleanupOldData();
    void updateStatistics(const PerformanceSample& sample);
    QString generateAlertId() const;
};

// Performance monitor for real-time data collection
class PerformanceMonitor : public QThread {
    Q_OBJECT

public:
    explicit PerformanceMonitor(QObject* parent = nullptr);
    ~PerformanceMonitor() override;

    // Monitoring control
    void startMonitoring(const QStringList& pluginIds);
    void stopMonitoring();
    void addPlugin(const QString& pluginId);
    void removePlugin(const QString& pluginId);
    
    // Configuration
    void setSamplingInterval(int milliseconds);
    int samplingInterval() const;
    void setMetricTypes(const QList<MetricType>& types);
    QList<MetricType> metricTypes() const;

signals:
    void sampleCollected(const PerformanceSample& sample);
    void monitoringStarted();
    void monitoringStopped();
    void errorOccurred(const QString& error);

protected:
    void run() override;

private:
    QStringList m_pluginIds;
    QList<MetricType> m_metricTypes;
    int m_samplingInterval;
    QAtomicInt m_stopRequested;
    QMutex m_configMutex;
    
    void collectMetrics();
    double getCPUUsage(const QString& pluginId) const;
    qint64 getMemoryUsage(const QString& pluginId) const;
    int getThreadCount(const QString& pluginId) const;
    double getIOOperations(const QString& pluginId) const;
};

// Performance analyzer for data analysis and insights
class PerformanceAnalyzer : public QObject {
    Q_OBJECT

public:
    explicit PerformanceAnalyzer(QObject* parent = nullptr);
    ~PerformanceAnalyzer() override;

    // Analysis methods
    QStringList analyzePlugin(const QString& pluginId, const QList<PerformanceSample>& samples) const;
    QStringList detectAnomalies(const QString& pluginId, const QList<PerformanceSample>& samples) const;
    QStringList identifyBottlenecks(const QString& pluginId, const QList<PerformanceSample>& samples) const;
    QStringList generateOptimizationSuggestions(const QString& pluginId, const PerformanceStats& stats) const;
    
    // Trend analysis
    QString analyzeTrend(const QList<PerformanceSample>& samples) const;
    double calculateTrendSlope(const QList<PerformanceSample>& samples) const;
    bool isIncreasingTrend(const QList<PerformanceSample>& samples) const;
    bool isDecreasingTrend(const QList<PerformanceSample>& samples) const;
    
    // Correlation analysis
    double calculateCorrelation(const QList<PerformanceSample>& samples1, const QList<PerformanceSample>& samples2) const;
    QMap<QString, double> findCorrelations(const QString& pluginId, const QMap<MetricType, QList<PerformanceSample>>& allSamples) const;
    
    // Statistical analysis
    PerformanceStats calculateStatistics(const QList<PerformanceSample>& samples) const;
    QList<double> detectOutliers(const QList<PerformanceSample>& samples, double threshold = 2.0) const;
    double calculatePercentile(const QList<PerformanceSample>& samples, double percentile) const;

signals:
    void analysisCompleted(const QString& pluginId, const QStringList& insights);
    void anomalyDetected(const QString& pluginId, const QString& anomaly);
    void bottleneckIdentified(const QString& pluginId, const QString& bottleneck);

private:
    double calculateMean(const QList<double>& values) const;
    double calculateStandardDeviation(const QList<double>& values, double mean) const;
    double calculateMedian(QList<double> values) const;
    QList<double> extractValues(const QList<PerformanceSample>& samples) const;
    QString formatSuggestion(const QString& category, const QString& description, const QString& action) const;
};

// Performance reporter for generating reports
class PerformanceReporter : public QObject {
    Q_OBJECT

public:
    explicit PerformanceReporter(QObject* parent = nullptr);
    ~PerformanceReporter() override;

    // Report generation
    void generateReport(const QString& pluginId, const QString& filePath, const QString& format = "html") const;
    void generateSummaryReport(const QStringList& pluginIds, const QString& filePath, const QString& format = "html") const;
    void generateComparisonReport(const QStringList& pluginIds, const QString& filePath, const QString& format = "html") const;
    
    // Export formats
    void exportToJSON(const QString& pluginId, const QString& filePath) const;
    void exportToCSV(const QString& pluginId, const QString& filePath) const;
    void exportToXML(const QString& pluginId, const QString& filePath) const;
    void exportToHTML(const QString& pluginId, const QString& filePath) const;
    void exportToPDF(const QString& pluginId, const QString& filePath) const;
    
    // Report customization
    void setReportTemplate(const QString& templatePath);
    QString reportTemplate() const;
    void setReportTitle(const QString& title);
    QString reportTitle() const;
    void setIncludeCharts(bool include);
    bool includeCharts() const;

signals:
    void reportGenerated(const QString& filePath);
    void exportCompleted(const QString& filePath, const QString& format);
    void reportError(const QString& error);

private:
    QString m_templatePath;
    QString m_reportTitle;
    bool m_includeCharts;
    
    QString generateHTMLReport(const QString& pluginId) const;
    QString generateJSONReport(const QString& pluginId) const;
    QString generateCSVReport(const QString& pluginId) const;
    QString formatMetricValue(double value, const QString& unit) const;
    QString formatDuration(qint64 milliseconds) const;
    QString formatBytes(qint64 bytes) const;
};

// Performance optimizer for automatic optimization
class PerformanceOptimizer : public QObject {
    Q_OBJECT

public:
    explicit PerformanceOptimizer(QObject* parent = nullptr);
    ~PerformanceOptimizer() override;

    // Optimization control
    void enableAutoOptimization(bool enable);
    bool isAutoOptimizationEnabled() const;
    void optimizePlugin(const QString& pluginId);
    void optimizeAllPlugins();
    
    // Optimization strategies
    void enableMemoryOptimization(bool enable);
    void enableCPUOptimization(bool enable);
    void enableIOOptimization(bool enable);
    void enableCacheOptimization(bool enable);
    
    // Configuration
    void setOptimizationThreshold(MetricType type, double threshold);
    double getOptimizationThreshold(MetricType type) const;
    void setOptimizationInterval(int minutes);
    int optimizationInterval() const;

signals:
    void optimizationStarted(const QString& pluginId);
    void optimizationCompleted(const QString& pluginId, const QStringList& actions);
    void optimizationFailed(const QString& pluginId, const QString& error);

public slots:
    void performScheduledOptimization();

private slots:
    void onOptimizationTimer();

private:
    bool m_autoOptimizationEnabled;
    bool m_memoryOptimizationEnabled;
    bool m_cpuOptimizationEnabled;
    bool m_ioOptimizationEnabled;
    bool m_cacheOptimizationEnabled;
    QMap<MetricType, double> m_optimizationThresholds;
    QTimer* m_optimizationTimer;
    int m_optimizationInterval;
    
    QStringList optimizeMemoryUsage(const QString& pluginId);
    QStringList optimizeCPUUsage(const QString& pluginId);
    QStringList optimizeIOOperations(const QString& pluginId);
    QStringList optimizeCacheUsage(const QString& pluginId);
    bool shouldOptimize(const QString& pluginId, MetricType type) const;
};
