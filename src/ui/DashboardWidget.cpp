// DashboardWidget.cpp - Implementation of comprehensive dashboard
#include "DashboardWidget.h"
#include <qtplugin/qtplugin.hpp>
#include "../core/PluginRegistry.h"
#include <QApplication>
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
#include <QSplitter>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QJsonObject>
#include <QJsonArray>
#include <QEnterEvent>

#ifdef QT_CHARTS_AVAILABLE
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#endif

DashboardWidget::DashboardWidget(QWidget* parent)
    : QWidget(parent)
    , m_refreshTimer(new QTimer(this))
{
    setupUI();
    connectSignals();
    
    // Start refresh timer
    m_refreshTimer->setInterval(m_refreshInterval);
    connect(m_refreshTimer, &QTimer::timeout, this, &DashboardWidget::onRefreshTimer);
    m_refreshTimer->start();
    
    m_elapsedTimer.start();
}

DashboardWidget::~DashboardWidget() = default;

void DashboardWidget::setPluginManager(qtplugin::PluginManager* manager)
{
    m_pluginManager = manager;
    if (manager) {
        // Connect to plugin manager signals for real-time updates
        connect(manager, &qtplugin::PluginManager::plugin_loaded, this, &DashboardWidget::updateMetrics);
        connect(manager, &qtplugin::PluginManager::plugin_unloaded, this, &DashboardWidget::updateMetrics);
        connect(manager, &qtplugin::PluginManager::plugin_state_changed, this, &DashboardWidget::updateMetrics);
    }
    updateAllComponents();
}

void DashboardWidget::setPluginRegistry(PluginRegistry* registry)
{
    m_pluginRegistry = registry;
    if (registry) {
        // Connect to registry signals for real-time updates
        connect(registry, &PluginRegistry::countChanged, this, &DashboardWidget::updateMetrics);
        connect(registry, &PluginRegistry::pluginStateChanged, this, &DashboardWidget::updateMetrics);
    }
    updateAllComponents();
}

void DashboardWidget::refreshDashboard()
{
    updateAllComponents();
    if (m_lastUpdateLabel) {
        m_lastUpdateLabel->setText("Last updated: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
    }
}

void DashboardWidget::updateMetrics()
{
    // Update metrics cards
    if (!m_metricsCards.isEmpty()) {
        // Total plugins
        int totalPlugins = m_pluginRegistry ? m_pluginRegistry->rowCount() : 0;
        if (m_metricsCards.size() > 0) {
            m_metricsCards[0]->setValue(QString::number(totalPlugins));
        }
        
        // Enabled plugins (placeholder)
        int enabledPlugins = totalPlugins > 0 ? totalPlugins / 2 : 0;
        if (m_metricsCards.size() > 1) {
            m_metricsCards[1]->setValue(QString::number(enabledPlugins));
        }
        
        // Memory usage (placeholder)
        if (m_metricsCards.size() > 2) {
            m_metricsCards[2]->setValue("128 MB");
        }
        
        // CPU usage (placeholder)
        if (m_metricsCards.size() > 3) {
            m_metricsCards[3]->setValue("15%");
        }
    }
}

void DashboardWidget::exportReport()
{
    qDebug() << "Exporting dashboard report...";
    // TODO: Implement report export functionality
}

void DashboardWidget::resetMetrics()
{
    qDebug() << "Resetting dashboard metrics...";
    // TODO: Implement metrics reset functionality
}

void DashboardWidget::onRefreshTimer()
{
    updateMetrics();
}

void DashboardWidget::onTimeRangeChanged()
{
    if (m_timeRangeCombo) {
        m_selectedTimeRange = m_timeRangeCombo->currentText();
        updateAllComponents();
    }
}

void DashboardWidget::onMetricTypeChanged()
{
    // TODO: Implement metric type change handling
}

void DashboardWidget::onPluginSelected()
{
    // TODO: Implement plugin selection handling
}

void DashboardWidget::onExportFormatChanged()
{
    // TODO: Implement export format change handling
}

void DashboardWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(16);
    
    // Create scroll area
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    
    m_contentWidget = new QWidget;
    m_mainLayout = new QVBoxLayout(m_contentWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(16);
    
    // Setup header
    setupHeader();
    
    // Setup metrics cards
    setupMetricsCards();
    
    // Setup charts
#ifdef QT_CHARTS_AVAILABLE
    setupCharts();
#endif
    
    // Setup details section
    setupDetailsSection();
    
    m_scrollArea->setWidget(m_contentWidget);
    layout->addWidget(m_scrollArea);
}

void DashboardWidget::setupHeader()
{
    m_headerFrame = new QFrame;
    m_headerFrame->setFrameStyle(QFrame::StyledPanel);
    m_headerFrame->setStyleSheet("QFrame { background-color: white; border-radius: 8px; padding: 16px; }");
    
    auto* headerLayout = new QHBoxLayout(m_headerFrame);
    
    // Title section
    auto* titleLayout = new QVBoxLayout;
    m_titleLabel = new QLabel("Plugin Dashboard");
    m_titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    titleLayout->addWidget(m_titleLabel);
    
    m_lastUpdateLabel = new QLabel("Last updated: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
    m_lastUpdateLabel->setStyleSheet("color: #7f8c8d; font-size: 12px;");
    titleLayout->addWidget(m_lastUpdateLabel);
    
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    
    // Controls section
    auto* controlsLayout = new QHBoxLayout;
    
    m_timeRangeCombo = new QComboBox;
    m_timeRangeCombo->addItems({"Last Hour", "Last 24 Hours", "Last Week", "Last Month"});
    m_timeRangeCombo->setCurrentText("Last Hour");
    controlsLayout->addWidget(m_timeRangeCombo);
    
    m_refreshBtn = new QPushButton("Refresh");
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 8px 16px; border-radius: 4px; }");
    controlsLayout->addWidget(m_refreshBtn);
    
    m_exportBtn = new QPushButton("Export");
    m_exportBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; border: none; padding: 8px 16px; border-radius: 4px; }");
    controlsLayout->addWidget(m_exportBtn);
    
    headerLayout->addLayout(controlsLayout);
    
    m_mainLayout->addWidget(m_headerFrame);
}

void DashboardWidget::setupMetricsCards()
{
    m_metricsFrame = new QFrame;
    m_metricsLayout = new QHBoxLayout(m_metricsFrame);
    m_metricsLayout->setSpacing(16);
    
    // Create metrics cards
    auto* totalPluginsCard = new MetricsCard("Total Plugins");
    totalPluginsCard->setValue("0");
    totalPluginsCard->setSubtitle("Installed");
    totalPluginsCard->setColor(QColor("#3498db"));
    m_metricsCards.append(totalPluginsCard);
    
    auto* enabledPluginsCard = new MetricsCard("Enabled Plugins");
    enabledPluginsCard->setValue("0");
    enabledPluginsCard->setSubtitle("Active");
    enabledPluginsCard->setColor(QColor("#2ecc71"));
    m_metricsCards.append(enabledPluginsCard);
    
    auto* memoryCard = new MetricsCard("Memory Usage");
    memoryCard->setValue("0 MB");
    memoryCard->setSubtitle("Total");
    memoryCard->setColor(QColor("#f39c12"));
    m_metricsCards.append(memoryCard);
    
    auto* cpuCard = new MetricsCard("CPU Usage");
    cpuCard->setValue("0%");
    cpuCard->setSubtitle("Average");
    cpuCard->setColor(QColor("#e74c3c"));
    m_metricsCards.append(cpuCard);
    
    for (auto* card : m_metricsCards) {
        m_metricsLayout->addWidget(card);
    }
    
    m_mainLayout->addWidget(m_metricsFrame);
}

#ifdef QT_CHARTS_AVAILABLE
void DashboardWidget::setupCharts()
{
    m_chartsTab = new QTabWidget;
    m_chartsTab->setStyleSheet("QTabWidget::pane { border: 1px solid #bdc3c7; background-color: white; }");

    // Performance chart placeholder
    auto* performanceWidget = new QWidget;
    auto* performanceLayout = new QVBoxLayout(performanceWidget);
    auto* performanceLabel = new QLabel("Performance Chart");
    performanceLabel->setAlignment(Qt::AlignCenter);
    performanceLabel->setStyleSheet("font-size: 18px; color: #7f8c8d; padding: 40px;");
    performanceLayout->addWidget(performanceLabel);
    m_chartsTab->addTab(performanceWidget, "Performance");

    // Usage chart placeholder
    auto* usageWidget = new QWidget;
    auto* usageLayout = new QVBoxLayout(usageWidget);
    auto* usageLabel = new QLabel("Usage Statistics");
    usageLabel->setAlignment(Qt::AlignCenter);
    usageLabel->setStyleSheet("font-size: 18px; color: #7f8c8d; padding: 40px;");
    usageLayout->addWidget(usageLabel);
    m_chartsTab->addTab(usageWidget, "Usage");

    // Distribution chart placeholder
    auto* distributionWidget = new QWidget;
    auto* distributionLayout = new QVBoxLayout(distributionWidget);
    auto* distributionLabel = new QLabel("Plugin Distribution");
    distributionLabel->setAlignment(Qt::AlignCenter);
    distributionLabel->setStyleSheet("font-size: 18px; color: #7f8c8d; padding: 40px;");
    distributionLayout->addWidget(distributionLabel);
    m_chartsTab->addTab(distributionWidget, "Distribution");

    m_mainLayout->addWidget(m_chartsTab);
}
#endif

void DashboardWidget::setupDetailsSection()
{
    m_detailsSplitter = new QSplitter(Qt::Horizontal);
    
    // System overview placeholder
    auto* systemWidget = new QWidget;
    auto* systemLayout = new QVBoxLayout(systemWidget);
    auto* systemLabel = new QLabel("System Overview");
    systemLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 16px;");
    systemLayout->addWidget(systemLabel);
    
    auto* systemInfo = new QLabel("System information will be displayed here");
    systemInfo->setStyleSheet("color: #7f8c8d; padding: 16px;");
    systemLayout->addWidget(systemInfo);
    systemLayout->addStretch();
    
    // Activity log placeholder
    auto* activityWidget = new QWidget;
    auto* activityLayout = new QVBoxLayout(activityWidget);
    auto* activityLabel = new QLabel("Activity Log");
    activityLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 16px;");
    activityLayout->addWidget(activityLabel);
    
    auto* activityInfo = new QLabel("Plugin activity will be logged here");
    activityInfo->setStyleSheet("color: #7f8c8d; padding: 16px;");
    activityLayout->addWidget(activityInfo);
    activityLayout->addStretch();
    
    m_detailsSplitter->addWidget(systemWidget);
    m_detailsSplitter->addWidget(activityWidget);
    m_detailsSplitter->setSizes({400, 400});
    
    m_mainLayout->addWidget(m_detailsSplitter);
}

void DashboardWidget::connectSignals()
{
    if (m_refreshBtn) {
        connect(m_refreshBtn, &QPushButton::clicked, this, &DashboardWidget::refreshDashboard);
    }
    if (m_exportBtn) {
        connect(m_exportBtn, &QPushButton::clicked, this, &DashboardWidget::exportReport);
    }
    if (m_timeRangeCombo) {
        connect(m_timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &DashboardWidget::onTimeRangeChanged);
    }
}

void DashboardWidget::updateAllComponents()
{
    updateMetrics();
    // TODO: Update charts and other components
}

// MetricsCard Implementation
MetricsCard::MetricsCard(const QString& title, QWidget* parent)
    : QFrame(parent)
    , m_title(title)
    , m_color(QColor("#3498db"))
{
    setupUI();
    setFrameStyle(QFrame::StyledPanel);
    setStyleSheet("QFrame { background-color: white; border-radius: 8px; padding: 16px; }");
    setMinimumSize(200, 120);
}

void MetricsCard::setValue(const QString& value)
{
    m_value = value;
    if (m_valueLabel) {
        m_valueLabel->setText(value);
    }
}

void MetricsCard::setSubtitle(const QString& subtitle)
{
    m_subtitle = subtitle;
    if (m_subtitleLabel) {
        m_subtitleLabel->setText(subtitle);
    }
}

void MetricsCard::setColor(const QColor& color)
{
    m_color = color;
    update();
}

void MetricsCard::setTrend(double percentage)
{
    m_trendPercentage = percentage;
    updateTrendIndicator();
}

void MetricsCard::setClickable(bool clickable)
{
    m_clickable = clickable;
    setCursor(clickable ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void MetricsCard::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);
    
    m_titleLabel = new QLabel(m_title);
    m_titleLabel->setStyleSheet("font-size: 14px; color: #7f8c8d; font-weight: 500;");
    layout->addWidget(m_titleLabel);
    
    m_valueLabel = new QLabel(m_value);
    m_valueLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #2c3e50;");
    layout->addWidget(m_valueLabel);
    
    m_subtitleLabel = new QLabel(m_subtitle);
    m_subtitleLabel->setStyleSheet("font-size: 12px; color: #95a5a6;");
    layout->addWidget(m_subtitleLabel);
    
    layout->addStretch();
}

void MetricsCard::updateTrendIndicator()
{
    // TODO: Implement trend indicator
}

void MetricsCard::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    // TODO: Add custom painting for visual enhancements
}

void MetricsCard::mousePressEvent(QMouseEvent* event)
{
    if (m_clickable) {
        emit clicked();
    }
    QFrame::mousePressEvent(event);
}

void MetricsCard::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    if (m_clickable) {
        setStyleSheet("QFrame { background-color: #f8f9fa; border-radius: 8px; padding: 16px; }");
    }
    QFrame::enterEvent(event);
}

void MetricsCard::leaveEvent(QEvent* event)
{
    m_hovered = false;
    setStyleSheet("QFrame { background-color: white; border-radius: 8px; padding: 16px; }");
    QFrame::leaveEvent(event);
}

void MetricsCard::setIcon(const QString& iconPath)
{
    m_iconPath = iconPath;
    // TODO: Implement icon loading
}

#ifdef QT_CHARTS_AVAILABLE
// PerformanceChart Implementation
PerformanceChart::PerformanceChart(QWidget* parent)
    : QWidget(parent)
    , m_chartView(nullptr)
    , m_chart(nullptr)
    , m_cpuSeries(nullptr)
    , m_memorySeries(nullptr)
    , m_cpuAreaSeries(nullptr)
    , m_memoryAreaSeries(nullptr)
    , m_axisY(nullptr)
    , m_axisX(nullptr)
{
    setupChart();
}

void PerformanceChart::setupChart()
{
    auto* layout = new QVBoxLayout(this);

    // Create placeholder chart
    m_chartView = new QChartView;
    m_chart = new QChart;
    m_chart->setTitle("Performance Chart");
    m_chartView->setChart(m_chart);

    layout->addWidget(m_chartView);
}

void PerformanceChart::addDataPoint(const QString& plugin, double cpuUsage, double memoryUsage, const QDateTime& timestamp)
{
    DataPoint point;
    point.plugin = plugin;
    point.cpuUsage = cpuUsage;
    point.memoryUsage = memoryUsage;
    point.timestamp = timestamp;
    m_dataPoints.append(point);
}

void PerformanceChart::clearData()
{
    m_dataPoints.clear();
}

void PerformanceChart::setTimeRange(const QString& range)
{
    m_timeRange = range;
}

void PerformanceChart::setSelectedPlugins(const QStringList& plugins)
{
    m_selectedPlugins = plugins;
}

void PerformanceChart::updateChart()
{
    // TODO: Implement chart update logic
}

void PerformanceChart::exportChart()
{
    // TODO: Implement chart export
}

void PerformanceChart::onSeriesHovered(const QPointF& point, bool state)
{
    Q_UNUSED(point)
    Q_UNUSED(state)
    // TODO: Implement hover handling
}

void PerformanceChart::onSeriesClicked(const QPointF& point)
{
    Q_UNUSED(point)
    // TODO: Implement click handling
}

void PerformanceChart::setupSeries()
{
    // TODO: Implement series setup
}

void PerformanceChart::updateAxes()
{
    // TODO: Implement axes update
}

void PerformanceChart::addLegend()
{
    // TODO: Implement legend
}
#endif

// ActivityLog Implementation
ActivityLog::ActivityLog(QWidget* parent)
    : QWidget(parent)
    , m_logTable(nullptr)
    , m_pluginFilter(nullptr)
    , m_actionFilter(nullptr)
    , m_clearBtn(nullptr)
    , m_exportBtn(nullptr)
{
    setupUI();
}

void ActivityLog::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Filter controls
    auto* filterLayout = new QHBoxLayout;

    m_pluginFilter = new QComboBox;
    m_pluginFilter->addItem("All Plugins");
    filterLayout->addWidget(new QLabel("Plugin:"));
    filterLayout->addWidget(m_pluginFilter);

    m_actionFilter = new QComboBox;
    m_actionFilter->addItem("All Actions");
    filterLayout->addWidget(new QLabel("Action:"));
    filterLayout->addWidget(m_actionFilter);

    filterLayout->addStretch();

    m_clearBtn = new QPushButton("Clear");
    m_exportBtn = new QPushButton("Export");
    filterLayout->addWidget(m_clearBtn);
    filterLayout->addWidget(m_exportBtn);

    layout->addLayout(filterLayout);

    // Log table
    m_logTable = new QTableWidget;
    m_logTable->setColumnCount(4);
    m_logTable->setHorizontalHeaderLabels({"Timestamp", "Plugin", "Action", "Details"});
    layout->addWidget(m_logTable);
}

void ActivityLog::addLogEntry(const QString& timestamp, const QString& plugin, const QString& action, const QString& details)
{
    LogEntry entry;
    entry.timestamp = QDateTime::fromString(timestamp);
    entry.plugin = plugin;
    entry.action = action;
    entry.details = details;
    m_logEntries.append(entry);

    // Add to table
    if (m_logTable) {
        int row = m_logTable->rowCount();
        m_logTable->insertRow(row);
        m_logTable->setItem(row, 0, new QTableWidgetItem(timestamp));
        m_logTable->setItem(row, 1, new QTableWidgetItem(plugin));
        m_logTable->setItem(row, 2, new QTableWidgetItem(action));
        m_logTable->setItem(row, 3, new QTableWidgetItem(details));
    }
}

void ActivityLog::clearLog()
{
    m_logEntries.clear();
    if (m_logTable) {
        m_logTable->setRowCount(0);
    }
}

void ActivityLog::setMaxEntries(int maxEntries)
{
    m_maxEntries = maxEntries;
}

void ActivityLog::exportLog(const QString& filePath)
{
    Q_UNUSED(filePath)
    // TODO: Implement log export
}

void ActivityLog::filterByPlugin(const QString& plugin)
{
    Q_UNUSED(plugin)
    updateFilter();
}

void ActivityLog::filterByAction(const QString& action)
{
    Q_UNUSED(action)
    updateFilter();
}

void ActivityLog::clearFilters()
{
    updateFilter();
}

void ActivityLog::onItemDoubleClicked()
{
    // TODO: Implement item double click handling
}

void ActivityLog::onContextMenuRequested(const QPoint& pos)
{
    Q_UNUSED(pos)
    // TODO: Implement context menu
}

void ActivityLog::updateFilter()
{
    // TODO: Implement filter update logic
}

// SystemOverview Implementation
SystemOverview::SystemOverview(QWidget* parent)
    : QWidget(parent)
    , m_systemGroup(nullptr)
    , m_osLabel(nullptr)
    , m_qtVersionLabel(nullptr)
    , m_uptimeLabel(nullptr)
    , m_memoryLabel(nullptr)
    , m_pluginGroup(nullptr)
    , m_totalPluginsLabel(nullptr)
    , m_enabledPluginsLabel(nullptr)
    , m_loadedPluginsLabel(nullptr)
    , m_errorPluginsLabel(nullptr)
    , m_performanceGroup(nullptr)
    , m_cpuBar(nullptr)
    , m_memoryBar(nullptr)
    , m_diskBar(nullptr)
    , m_networkLabel(nullptr)
    , m_refreshBtn(nullptr)
    , m_detailsBtn(nullptr)
{
    setupUI();
}

void SystemOverview::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // System info group
    m_systemGroup = new QGroupBox("System Information");
    auto* systemLayout = new QVBoxLayout(m_systemGroup);

    m_osLabel = new QLabel("OS: Unknown");
    m_qtVersionLabel = new QLabel("Qt Version: " + QString(QT_VERSION_STR));
    m_uptimeLabel = new QLabel("Uptime: Unknown");
    m_memoryLabel = new QLabel("Memory: Unknown");

    systemLayout->addWidget(m_osLabel);
    systemLayout->addWidget(m_qtVersionLabel);
    systemLayout->addWidget(m_uptimeLabel);
    systemLayout->addWidget(m_memoryLabel);

    layout->addWidget(m_systemGroup);

    // Plugin stats group
    m_pluginGroup = new QGroupBox("Plugin Statistics");
    auto* pluginLayout = new QVBoxLayout(m_pluginGroup);

    m_totalPluginsLabel = new QLabel("Total: 0");
    m_enabledPluginsLabel = new QLabel("Enabled: 0");
    m_loadedPluginsLabel = new QLabel("Loaded: 0");
    m_errorPluginsLabel = new QLabel("Errors: 0");

    pluginLayout->addWidget(m_totalPluginsLabel);
    pluginLayout->addWidget(m_enabledPluginsLabel);
    pluginLayout->addWidget(m_loadedPluginsLabel);
    pluginLayout->addWidget(m_errorPluginsLabel);

    layout->addWidget(m_pluginGroup);

    // Performance group
    m_performanceGroup = new QGroupBox("Performance");
    auto* performanceLayout = new QVBoxLayout(m_performanceGroup);

    m_cpuBar = new QProgressBar;
    m_cpuBar->setRange(0, 100);
    m_cpuBar->setValue(0);
    performanceLayout->addWidget(new QLabel("CPU Usage:"));
    performanceLayout->addWidget(m_cpuBar);

    m_memoryBar = new QProgressBar;
    m_memoryBar->setRange(0, 100);
    m_memoryBar->setValue(0);
    performanceLayout->addWidget(new QLabel("Memory Usage:"));
    performanceLayout->addWidget(m_memoryBar);

    m_diskBar = new QProgressBar;
    m_diskBar->setRange(0, 100);
    m_diskBar->setValue(0);
    performanceLayout->addWidget(new QLabel("Disk Usage:"));
    performanceLayout->addWidget(m_diskBar);

    m_networkLabel = new QLabel("Network: Unknown");
    performanceLayout->addWidget(m_networkLabel);

    layout->addWidget(m_performanceGroup);

    // Control buttons
    auto* buttonLayout = new QHBoxLayout;
    m_refreshBtn = new QPushButton("Refresh");
    m_detailsBtn = new QPushButton("Details");
    buttonLayout->addWidget(m_refreshBtn);
    buttonLayout->addWidget(m_detailsBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
    layout->addStretch();
}

void SystemOverview::updateSystemInfo()
{
    // TODO: Implement system info update
}

void SystemOverview::updatePluginStats()
{
    // TODO: Implement plugin stats update
}

void SystemOverview::updatePerformanceStats()
{
    // TODO: Implement performance stats update
}

void SystemOverview::onRefreshClicked()
{
    updateSystemInfo();
    updatePluginStats();
    updatePerformanceStats();
}

void SystemOverview::onDetailsClicked()
{
    // TODO: Implement details dialog
}

void SystemOverview::createInfoGroup(const QString& title, QWidget* parent)
{
    Q_UNUSED(title)
    Q_UNUSED(parent)
    // TODO: Implement info group creation
}

// PluginAnalytics Implementation
PluginAnalytics::PluginAnalytics(QWidget* parent)
    : QWidget(parent)
    , m_pluginCombo(nullptr)
    , m_metricCombo(nullptr)
    , m_timeRangeCombo(nullptr)
#ifdef QT_CHARTS_AVAILABLE
    , m_pluginChartView(nullptr)
    , m_pluginChart(nullptr)
#endif
    , m_metricsGroup(nullptr)
    , m_loadTimeLabel(nullptr)
    , m_avgCpuLabel(nullptr)
    , m_avgMemoryLabel(nullptr)
    , m_errorCountLabel(nullptr)
    , m_lastUsedLabel(nullptr)
    , m_detailsGroup(nullptr)
    , m_detailsText(nullptr)
{
    setupUI();
}

void PluginAnalytics::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Controls
    auto* controlsLayout = new QHBoxLayout;

    m_pluginCombo = new QComboBox;
    m_pluginCombo->addItem("Select Plugin");
    controlsLayout->addWidget(new QLabel("Plugin:"));
    controlsLayout->addWidget(m_pluginCombo);

    m_metricCombo = new QComboBox;
    m_metricCombo->addItems({"CPU", "Memory", "Load Time", "Errors"});
    controlsLayout->addWidget(new QLabel("Metric:"));
    controlsLayout->addWidget(m_metricCombo);

    m_timeRangeCombo = new QComboBox;
    m_timeRangeCombo->addItems({"1 Hour", "24 Hours", "1 Week", "1 Month"});
    controlsLayout->addWidget(new QLabel("Time Range:"));
    controlsLayout->addWidget(m_timeRangeCombo);

    controlsLayout->addStretch();
    layout->addLayout(controlsLayout);

#ifdef QT_CHARTS_AVAILABLE
    // Chart
    m_pluginChartView = new QChartView;
    m_pluginChart = new QChart;
    m_pluginChart->setTitle("Plugin Analytics");
    m_pluginChartView->setChart(m_pluginChart);
    layout->addWidget(m_pluginChartView);
#endif

    // Metrics group
    m_metricsGroup = new QGroupBox("Metrics");
    auto* metricsLayout = new QVBoxLayout(m_metricsGroup);

    m_loadTimeLabel = new QLabel("Load Time: Unknown");
    m_avgCpuLabel = new QLabel("Avg CPU: Unknown");
    m_avgMemoryLabel = new QLabel("Avg Memory: Unknown");
    m_errorCountLabel = new QLabel("Errors: 0");
    m_lastUsedLabel = new QLabel("Last Used: Unknown");

    metricsLayout->addWidget(m_loadTimeLabel);
    metricsLayout->addWidget(m_avgCpuLabel);
    metricsLayout->addWidget(m_avgMemoryLabel);
    metricsLayout->addWidget(m_errorCountLabel);
    metricsLayout->addWidget(m_lastUsedLabel);

    layout->addWidget(m_metricsGroup);

    // Details group
    m_detailsGroup = new QGroupBox("Details");
    auto* detailsLayout = new QVBoxLayout(m_detailsGroup);

    m_detailsText = new QTextEdit;
    m_detailsText->setReadOnly(true);
    m_detailsText->setPlainText("Select a plugin to view detailed analytics.");
    detailsLayout->addWidget(m_detailsText);

    layout->addWidget(m_detailsGroup);
}

void PluginAnalytics::setSelectedPlugin(const QString& plugin)
{
    m_selectedPlugin = plugin;
    updateAnalytics();
}

void PluginAnalytics::updateAnalytics()
{
    updatePluginChart();
    updatePluginMetrics();
    updatePluginDetails();
}

void PluginAnalytics::onPluginChanged()
{
    if (m_pluginCombo) {
        setSelectedPlugin(m_pluginCombo->currentText());
    }
}

void PluginAnalytics::onMetricChanged()
{
    if (m_metricCombo) {
        m_selectedMetric = m_metricCombo->currentText().toLower();
        updatePluginChart();
    }
}

void PluginAnalytics::onTimeRangeChanged()
{
    if (m_timeRangeCombo) {
        m_selectedTimeRange = m_timeRangeCombo->currentText();
        updateAnalytics();
    }
}

void PluginAnalytics::updatePluginChart()
{
    // TODO: Implement chart update
}

void PluginAnalytics::updatePluginMetrics()
{
    // TODO: Implement metrics update
}

void PluginAnalytics::updatePluginDetails()
{
    // TODO: Implement details update
}

// MetricsCollector Implementation
MetricsCollector::MetricsCollector(QObject* parent)
    : QObject(parent)
    , m_collectionTimer(new QTimer(this))
{
    m_collectionTimer->setInterval(m_collectionInterval);
    connect(m_collectionTimer, &QTimer::timeout, this, &MetricsCollector::collectMetrics);
}

void MetricsCollector::startCollection()
{
    m_collectionTimer->start();
}

void MetricsCollector::stopCollection()
{
    m_collectionTimer->stop();
}

void MetricsCollector::setCollectionInterval(int intervalMs)
{
    m_collectionInterval = intervalMs;
    m_collectionTimer->setInterval(intervalMs);
}

QJsonObject MetricsCollector::getCurrentMetrics() const
{
    QJsonObject metrics;
    metrics["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metrics["cpu_usage"] = 15.0; // Placeholder
    metrics["memory_usage"] = 128.0; // Placeholder
    metrics["plugin_count"] = 5; // Placeholder
    return metrics;
}

QJsonArray MetricsCollector::getHistoricalMetrics(const QString& timeRange) const
{
    Q_UNUSED(timeRange)
    QJsonArray history;
    for (const auto& metrics : m_metricsHistory) {
        history.append(metrics);
    }
    return history;
}

void MetricsCollector::collectMetrics()
{
    QJsonObject metrics;

    collectSystemMetrics();
    collectPluginMetrics();
    collectPerformanceMetrics();

    metrics = getCurrentMetrics();
    storeMetrics(metrics);

    emit metricsUpdated(metrics);
}

void MetricsCollector::collectSystemMetrics()
{
    // TODO: Implement system metrics collection
}

void MetricsCollector::collectPluginMetrics()
{
    // TODO: Implement plugin metrics collection
}

void MetricsCollector::collectPerformanceMetrics()
{
    // TODO: Implement performance metrics collection
}

void MetricsCollector::storeMetrics(const QJsonObject& metrics)
{
    m_metricsHistory.append(metrics);

    // Keep history size under limit
    while (m_metricsHistory.size() > m_maxHistorySize) {
        m_metricsHistory.removeFirst();
    }
}
