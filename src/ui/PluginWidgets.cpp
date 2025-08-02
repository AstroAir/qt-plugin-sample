// PluginWidgets.cpp - UI widgets implementation
#include "PluginWidgets.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QTabWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTextStream>
#include <QFile>
#include <QApplication>
#include <QDateTime>

// PluginListWidget Implementation
PluginListWidget::PluginListWidget(QWidget* parent)
    : QListWidget(parent)
{
    connect(this, &QListWidget::itemSelectionChanged, this, &PluginListWidget::onItemSelectionChanged);
    connect(this, &QListWidget::itemDoubleClicked, this, &PluginListWidget::onItemDoubleClicked);
}

void PluginListWidget::refreshPlugins() {
    clear();
    // Stub implementation - would normally load from plugin manager
    addItem("Sample Plugin 1");
    addItem("Sample Plugin 2");
    addItem("Sample Plugin 3");
}

void PluginListWidget::filterPlugins(const QString& filter) {
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem* item = this->item(i);
        bool visible = filter.isEmpty() || item->text().contains(filter, Qt::CaseInsensitive);
        item->setHidden(!visible);
    }
}

void PluginListWidget::onItemSelectionChanged() {
    QListWidgetItem* current = currentItem();
    if (current) {
        emit pluginSelected(current->text());
    }
}

void PluginListWidget::onItemDoubleClicked() {
    QListWidgetItem* current = currentItem();
    if (current) {
        emit pluginDoubleClicked(current->text());
    }
}



// LogViewer Implementation
LogViewer::LogViewer(QWidget* parent)
    : QTextEdit(parent)
    , m_maxLines(1000)
{
    setReadOnly(true);
    setFont(QFont("Consolas", 9));
}

void LogViewer::addLogEntry(const QString& level, const QString& message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString entry = QString("[%1] %2: %3").arg(timestamp, level, message);
    append(entry);
    limitLines();
}

void LogViewer::onLogMessage(const QString& message) {
    addLogEntry("INFO", message);
}

void LogViewer::clearLogs() {
    clear();
}

void LogViewer::exportLogs(const QString& fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << toPlainText();
    }
}

void LogViewer::limitLines() {
    if (document()->lineCount() > m_maxLines) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor, 
                          document()->lineCount() - m_maxLines);
        cursor.removeSelectedText();
    }
}

// PerformanceMonitor Implementation
PerformanceMonitorWidget::PerformanceMonitorWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void PerformanceMonitorWidget::setupUI() {
    auto layout = new QGridLayout(this);
    
    layout->addWidget(new QLabel("CPU Usage:"), 0, 0);
    m_cpuBar = new QProgressBar();
    m_cpuLabel = new QLabel("0%");
    layout->addWidget(m_cpuBar, 0, 1);
    layout->addWidget(m_cpuLabel, 0, 2);
    
    layout->addWidget(new QLabel("Memory Usage:"), 1, 0);
    m_memoryBar = new QProgressBar();
    m_memoryLabel = new QLabel("0 MB");
    layout->addWidget(m_memoryBar, 1, 1);
    layout->addWidget(m_memoryLabel, 1, 2);
}

void PerformanceMonitorWidget::updateMetrics() {
    // Stub implementation - would normally get real metrics
    m_cpuBar->setValue(25);
    m_cpuLabel->setText("25%");
    m_memoryBar->setValue(60);
    m_memoryLabel->setText("120 MB");
}

void PerformanceMonitorWidget::startMonitoring() {
    updateMetrics();
}

void PerformanceMonitorWidget::stopMonitoring() {
    m_cpuBar->setValue(0);
    m_memoryBar->setValue(0);
}

// ConsoleWidget Implementation
ConsoleWidget::ConsoleWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ConsoleWidget::setupUI() {
    auto layout = new QVBoxLayout(this);
    
    m_output = new QTextEdit();
    m_output->setReadOnly(true);
    m_output->setFont(QFont("Consolas", 9));
    
    m_input = new QLineEdit();
    m_input->setPlaceholderText("Enter command...");
    
    layout->addWidget(m_output);
    layout->addWidget(m_input);
    
    connect(m_input, &QLineEdit::returnPressed, this, &ConsoleWidget::onReturnPressed);
}

void ConsoleWidget::executeCommand(const QString& command) {
    addOutput(QString("> %1").arg(command));
    addOutput("Command executed (stub implementation)");
    emit commandExecuted(command);
}

void ConsoleWidget::addOutput(const QString& output) {
    m_output->append(output);
}

void ConsoleWidget::clearConsole() {
    m_output->clear();
}

void ConsoleWidget::onReturnPressed() {
    QString command = m_input->text();
    if (!command.isEmpty()) {
        executeCommand(command);
        m_input->clear();
    }
}

// SecurityManager Implementation
SecurityManager::SecurityManager(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void SecurityManager::setupUI() {
    auto layout = new QVBoxLayout(this);
    
    m_statusLabel = new QLabel("Security Status: OK");
    m_settingsButton = new QPushButton("Security Settings");
    
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_settingsButton);
    layout->addStretch();
}

void SecurityManager::updateSecurityStatus() {
    m_statusLabel->setText("Security Status: OK");
}

void SecurityManager::showSecuritySettings() {
    // Stub implementation
}

// PreferencesDialog Implementation
PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Preferences");
    setModal(true);
    resize(400, 300);
    setupUI();
}

void PreferencesDialog::setupUI() {
    auto layout = new QVBoxLayout(this);
    
    auto tabWidget = new QTabWidget();
    
    // General tab
    auto generalWidget = new QWidget();
    auto generalLayout = new QVBoxLayout(generalWidget);
    generalLayout->addWidget(new QCheckBox("Enable auto-save"));
    generalLayout->addWidget(new QCheckBox("Show splash screen"));
    generalLayout->addStretch();
    tabWidget->addTab(generalWidget, "General");
    
    // Plugins tab
    auto pluginsWidget = new QWidget();
    auto pluginsLayout = new QVBoxLayout(pluginsWidget);
    pluginsLayout->addWidget(new QCheckBox("Auto-load plugins"));
    pluginsLayout->addWidget(new QCheckBox("Enable hot reload"));
    pluginsLayout->addStretch();
    tabWidget->addTab(pluginsWidget, "Plugins");
    
    layout->addWidget(tabWidget);
    
    // Buttons
    auto buttonLayout = new QHBoxLayout();
    auto okButton = new QPushButton("OK");
    auto cancelButton = new QPushButton("Cancel");
    
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    
    layout->addLayout(buttonLayout);
}

void PreferencesDialog::loadSettings() {
    // Stub implementation
}

void PreferencesDialog::saveSettings() {
    // Stub implementation
}

// AboutDialog Implementation
AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("About");
    setModal(true);
    setupUI();
}

void AboutDialog::setupUI() {
    auto layout = new QVBoxLayout(this);
    
    auto titleLabel = new QLabel("Advanced Plugin Manager");
    titleLabel->setAlignment(Qt::AlignCenter);
    auto font = titleLabel->font();
    font.setPointSize(16);
    font.setBold(true);
    titleLabel->setFont(font);
    
    auto versionLabel = new QLabel("Version 2.1.0");
    versionLabel->setAlignment(Qt::AlignCenter);
    
    auto descLabel = new QLabel("A comprehensive Qt6 plugin management system");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    
    auto copyrightLabel = new QLabel("© 2024 Example Corporation");
    copyrightLabel->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(titleLabel);
    layout->addWidget(versionLabel);
    layout->addWidget(descLabel);
    layout->addWidget(copyrightLabel);
    
    auto closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
}
