// DashboardWidget.h - Comprehensive dashboard for plugin analytics
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QListWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QElapsedTimer>
#include <memory>

#ifdef QT_CHARTS_AVAILABLE
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QBarSeries>
#include <QBarSet>
#include <QPieSeries>
#include <QAreaSeries>
#include <QValueAxis>
#include <QCategoryAxis>
#include <QDateTimeAxis>
#endif

namespace qtplugin { class PluginManager; }
class PluginRegistry;
class MetricsCard;
class ActivityLog;
class SystemOverview;
class PluginAnalytics;

#ifdef QT_CHARTS_AVAILABLE
class PerformanceChart;
#endif

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget* parent = nullptr);
    ~DashboardWidget() override;

    void setPluginManager(qtplugin::PluginManager* manager);
    void setPluginRegistry(PluginRegistry* registry);

public slots:
    void refreshDashboard();
    void updateMetrics();
    void exportReport();
    void resetMetrics();

private slots:
    void onRefreshTimer();
    void onTimeRangeChanged();
    void onMetricTypeChanged();
    void onPluginSelected();
    void onExportFormatChanged();

private:
    void setupUI();
    void setupHeader();
    void setupMetricsCards();
#ifdef QT_CHARTS_AVAILABLE
    void setupCharts();
#endif
    void setupDetailsSection();
    void setupActivityLog();
    void setupSystemOverview();
    void connectSignals();
    void updateAllComponents();

    qtplugin::PluginManager* m_pluginManager = nullptr;
    PluginRegistry* m_pluginRegistry = nullptr;

    // UI Components
    QScrollArea* m_scrollArea;
    QWidget* m_contentWidget;
    QVBoxLayout* m_mainLayout;
    
    // Header
    QFrame* m_headerFrame;
    QLabel* m_titleLabel;
    QLabel* m_lastUpdateLabel;
    QPushButton* m_refreshBtn;
    QPushButton* m_exportBtn;
    QComboBox* m_timeRangeCombo;

    // Metrics Cards
    QFrame* m_metricsFrame;
    QHBoxLayout* m_metricsLayout;
    QList<MetricsCard*> m_metricsCards;

    // Charts Section
    QTabWidget* m_chartsTab;
#ifdef QT_CHARTS_AVAILABLE
    PerformanceChart* m_performanceChart;
    QChartView* m_usageChartView;
    QChartView* m_distributionChartView;
    QChartView* m_trendsChartView;
#endif

    // Details Section
    QSplitter* m_detailsSplitter;
    SystemOverview* m_systemOverview;
    ActivityLog* m_activityLog;
    PluginAnalytics* m_pluginAnalytics;

    // Update timer
    QTimer* m_refreshTimer;
    QElapsedTimer m_elapsedTimer;
    
    // Settings
    int m_refreshInterval = 5000; // 5 seconds
    QString m_selectedTimeRange = "1h";
    QString m_selectedMetricType = "all";
};

// Metrics Card Widget
class MetricsCard : public QFrame
{
    Q_OBJECT

public:
    explicit MetricsCard(const QString& title, QWidget* parent = nullptr);

    void setValue(const QString& value);
    void setSubtitle(const QString& subtitle);
    void setIcon(const QString& iconPath);
    void setColor(const QColor& color);
    void setTrend(double percentage);
    void setClickable(bool clickable);

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void setupUI();
    void updateTrendIndicator();

    QString m_title;
    QString m_value;
    QString m_subtitle;
    QString m_iconPath;
    QColor m_color;
    double m_trendPercentage = 0.0;
    bool m_clickable = false;
    bool m_hovered = false;

    QLabel* m_titleLabel;
    QLabel* m_valueLabel;
    QLabel* m_subtitleLabel;
    QLabel* m_iconLabel;
    QLabel* m_trendLabel;
};

#ifdef QT_CHARTS_AVAILABLE
// Performance Chart Widget
class PerformanceChart : public QWidget
{
    Q_OBJECT

public:
    explicit PerformanceChart(QWidget* parent = nullptr);

    void addDataPoint(const QString& plugin, double cpuUsage, double memoryUsage, const QDateTime& timestamp);
    void clearData();
    void setTimeRange(const QString& range);
    void setSelectedPlugins(const QStringList& plugins);

public slots:
    void updateChart();
    void exportChart();

private slots:
    void onSeriesHovered(const QPointF& point, bool state);
    void onSeriesClicked(const QPointF& point);

private:
    void setupChart();
    void setupSeries();
    void updateAxes();
    void addLegend();

    QChartView* m_chartView;
    QChart* m_chart;
    QLineSeries* m_cpuSeries;
    QLineSeries* m_memorySeries;
    QAreaSeries* m_cpuAreaSeries;
    QAreaSeries* m_memoryAreaSeries;
    QValueAxis* m_axisY;
    QDateTimeAxis* m_axisX;

    struct DataPoint {
        QString plugin;
        double cpuUsage;
        double memoryUsage;
        QDateTime timestamp;
    };

    QList<DataPoint> m_dataPoints;
    QString m_timeRange = "1h";
    QStringList m_selectedPlugins;
};
#endif

// Activity Log Widget
class ActivityLog : public QWidget
{
    Q_OBJECT

public:
    explicit ActivityLog(QWidget* parent = nullptr);

    void addLogEntry(const QString& timestamp, const QString& plugin, const QString& action, const QString& details);
    void clearLog();
    void setMaxEntries(int maxEntries);
    void exportLog(const QString& filePath);

public slots:
    void filterByPlugin(const QString& plugin);
    void filterByAction(const QString& action);
    void clearFilters();

private slots:
    void onItemDoubleClicked();
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUI();
    void updateFilter();

    QTableWidget* m_logTable;
    QComboBox* m_pluginFilter;
    QComboBox* m_actionFilter;
    QPushButton* m_clearBtn;
    QPushButton* m_exportBtn;

    struct LogEntry {
        QDateTime timestamp;
        QString plugin;
        QString action;
        QString details;
    };

    QList<LogEntry> m_logEntries;
    int m_maxEntries = 1000;
};

// System Overview Widget
class SystemOverview : public QWidget
{
    Q_OBJECT

public:
    explicit SystemOverview(QWidget* parent = nullptr);

    void updateSystemInfo();
    void updatePluginStats();
    void updatePerformanceStats();

private slots:
    void onRefreshClicked();
    void onDetailsClicked();

private:
    void setupUI();
    void createInfoGroup(const QString& title, QWidget* parent);

    // System Info
    QGroupBox* m_systemGroup;
    QLabel* m_osLabel;
    QLabel* m_qtVersionLabel;
    QLabel* m_uptimeLabel;
    QLabel* m_memoryLabel;

    // Plugin Stats
    QGroupBox* m_pluginGroup;
    QLabel* m_totalPluginsLabel;
    QLabel* m_enabledPluginsLabel;
    QLabel* m_loadedPluginsLabel;
    QLabel* m_errorPluginsLabel;

    // Performance Stats
    QGroupBox* m_performanceGroup;
    QProgressBar* m_cpuBar;
    QProgressBar* m_memoryBar;
    QProgressBar* m_diskBar;
    QLabel* m_networkLabel;

    QPushButton* m_refreshBtn;
    QPushButton* m_detailsBtn;
};

// Plugin Analytics Widget
class PluginAnalytics : public QWidget
{
    Q_OBJECT

public:
    explicit PluginAnalytics(QWidget* parent = nullptr);

    void setSelectedPlugin(const QString& plugin);
    void updateAnalytics();

private slots:
    void onPluginChanged();
    void onMetricChanged();
    void onTimeRangeChanged();

private:
    void setupUI();
    void updatePluginChart();
    void updatePluginMetrics();
    void updatePluginDetails();

    QComboBox* m_pluginCombo;
    QComboBox* m_metricCombo;
    QComboBox* m_timeRangeCombo;

    // Charts
#ifdef QT_CHARTS_AVAILABLE
    QChartView* m_pluginChartView;
    QChart* m_pluginChart;
#endif

    // Metrics
    QGroupBox* m_metricsGroup;
    QLabel* m_loadTimeLabel;
    QLabel* m_avgCpuLabel;
    QLabel* m_avgMemoryLabel;
    QLabel* m_errorCountLabel;
    QLabel* m_lastUsedLabel;

    // Details
    QGroupBox* m_detailsGroup;
    QTextEdit* m_detailsText;

    QString m_selectedPlugin;
    QString m_selectedMetric = "cpu";
    QString m_selectedTimeRange = "1h";
};

#ifdef QT_CHARTS_AVAILABLE
// Chart Utilities
class ChartUtils
{
public:
    static QChart* createLineChart(const QString& title);
    static QChart* createBarChart(const QString& title);
    static QChart* createPieChart(const QString& title);
    static QChart* createAreaChart(const QString& title);

    static void applyTheme(QChart* chart, bool darkTheme = false);
    static void exportChart(QChart* chart, const QString& filePath, const QSize& size = QSize(800, 600));
    static QColor getSeriesColor(int index);
    static void animateChart(QChart* chart);
};
#endif

// Metrics Collector
class MetricsCollector : public QObject
{
    Q_OBJECT

public:
    explicit MetricsCollector(QObject* parent = nullptr);

    void startCollection();
    void stopCollection();
    void setCollectionInterval(int intervalMs);

    QJsonObject getCurrentMetrics() const;
    QJsonArray getHistoricalMetrics(const QString& timeRange) const;

signals:
    void metricsUpdated(const QJsonObject& metrics);

private slots:
    void collectMetrics();

private:
    void collectSystemMetrics();
    void collectPluginMetrics();
    void collectPerformanceMetrics();
    void storeMetrics(const QJsonObject& metrics);

    QTimer* m_collectionTimer;
    QList<QJsonObject> m_metricsHistory;
    int m_maxHistorySize = 1000;
    int m_collectionInterval = 5000;
};
