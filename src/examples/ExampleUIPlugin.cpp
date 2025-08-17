// ExampleUIPlugin.cpp - Implementation of Example UI Plugin
#include "ExampleUIPlugin.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QProgressDialog>
#include <QSplitter>
#include <QTabWidget>
#include <QGroupBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <algorithm>
#include <numeric>

ExampleUIPlugin::ExampleUIPlugin(QObject* parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    connect(m_timer, &QTimer::timeout, this, &ExampleUIPlugin::onTimerTimeout);
}

ExampleUIPlugin::~ExampleUIPlugin()
{
    shutdown();
}

qtplugin::expected<void, qtplugin::PluginError> ExampleUIPlugin::initialize()
{
    if (m_initialized) {
        return qtplugin::make_success();
    }

    qDebug() << "Initializing Example UI Plugin...";

    // Load default configuration
    auto default_config = default_configuration();
    if (default_config) {
        m_configuration = *default_config;
    }

    // Setup timer
    m_timerInterval = m_configuration["updateInterval"].toInt(1000);
    m_timer->setInterval(m_timerInterval);

    m_initialized = true;
    m_state = qtplugin::PluginState::Running;

    qDebug() << "Example UI Plugin initialized successfully";
    return qtplugin::make_success();
}

void ExampleUIPlugin::shutdown() noexcept
{
    if (!m_initialized) {
        return;
    }

    qDebug() << "Shutting down Example UI Plugin...";

    if (m_timer) {
        m_timer->stop();
    }

    // Save current state
    saveWidgetState();

    // Clean up widgets
    if (m_mainWidget) {
        m_mainWidget->deleteLater();
        m_mainWidget = nullptr;
    }

    if (m_configWidget) {
        m_configWidget->deleteLater();
        m_configWidget = nullptr;
    }

    m_initialized = false;
    m_state = qtplugin::PluginState::Stopped;

    qDebug() << "Example UI Plugin shut down";
}

qtplugin::PluginCapabilities ExampleUIPlugin::capabilities() const noexcept
{
    return qtplugin::PluginCapability::UI | qtplugin::PluginCapability::Configuration;
}

std::optional<QJsonObject> ExampleUIPlugin::default_configuration() const
{
    QJsonObject config;
    config["theme"] = "Default";
    config["animationsEnabled"] = true;
    config["updateInterval"] = 1000;
    config["maxItems"] = 100;
    config["enableLogging"] = true;
    config["autoSave"] = true;
    config["opacity"] = 100;
    config["customPath"] = "";
    config["logLevel"] = "Info";
    return config;
}

qtplugin::expected<void, qtplugin::PluginError> ExampleUIPlugin::configure(const QJsonObject& config)
{
    m_configuration = config;

    // Apply configuration
    m_currentTheme = config["theme"].toString("Default");
    m_animationsEnabled = config["animationsEnabled"].toBool(true);
    m_timerInterval = config["updateInterval"].toInt(1000);

    if (m_timer) {
        m_timer->setInterval(m_timerInterval);
    }

    // Update UI if widgets exist
    if (m_configWidget) {
        // Update configuration widget to reflect new settings
        onConfigurationChanged();
    }

    qDebug() << "Example UI Plugin configured with theme:" << m_currentTheme;
    return qtplugin::make_success();
}

QJsonObject ExampleUIPlugin::current_configuration() const
{
    return m_configuration;
}

std::unique_ptr<QWidget> ExampleUIPlugin::create_widget(QWidget* parent)
{
    if (!m_initialized) {
        initialize();
    }

    auto widget = std::make_unique<QWidget>(parent);
    widget->setWindowTitle("Example UI Plugin - Demo");
    widget->resize(800, 600);

    setupDemoWidget(widget.get());
    m_mainWidget = widget.get();

    // Load saved state
    loadWidgetState();

    return widget;
}



std::unique_ptr<QWidget> ExampleUIPlugin::create_configuration_widget(QWidget* parent)
{
    if (m_configWidget) {
        // Return a copy since we can't return the same widget multiple times
        auto widget = std::make_unique<QWidget>(parent);
        widget->setWindowTitle("Example UI Plugin - Configuration");
        widget->resize(400, 300);
        setupConfigurationWidget(widget.get());
        return widget;
    }

    auto widget = std::make_unique<QWidget>(parent);
    widget->setWindowTitle("Example UI Plugin - Configuration");
    widget->resize(400, 300);

    setupConfigurationWidget(widget.get());
    m_configWidget = widget.get();

    return widget;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> ExampleUIPlugin::execute_command(std::string_view command, const QJsonObject& params)
{
    QString cmd = QString::fromUtf8(command.data(), command.size());

    if (cmd == "getStatus") {
        QJsonObject status;
        status["initialized"] = m_initialized;
        status["progressValue"] = m_progressValue;
        status["theme"] = m_currentTheme;
        status["timerRunning"] = m_timer && m_timer->isActive();
        return status;
    } else if (cmd == "setProgress") {
        int value = params.value("value").toInt(0);
        m_progressValue = qBound(0, value, 100);
        if (m_progressBar) {
            m_progressBar->setValue(m_progressValue);
        }
        QJsonObject result;
        result["success"] = true;
        result["value"] = m_progressValue;
        return result;
    } else if (cmd == "startTimer") {
        QJsonObject result;
        if (m_timer) {
            m_timer->start();
            result["success"] = true;
            result["message"] = "Timer started";
        } else {
            result["success"] = false;
            result["message"] = "Timer not available";
        }
        return result;
    } else if (cmd == "stopTimer") {
        QJsonObject result;
        if (m_timer) {
            m_timer->stop();
            result["success"] = true;
            result["message"] = "Timer stopped";
        } else {
            result["success"] = false;
            result["message"] = "Timer not available";
        }
        return result;
    } else if (cmd == "addListItem") {
        QString text = params.value("text").toString("New Item");
        QJsonObject result;
        if (m_listWidget) {
            m_listWidget->addItem(text);
            result["success"] = true;
            result["message"] = "Item added";
            result["text"] = text;
        } else {
            result["success"] = false;
            result["message"] = "List widget not available";
        }
        return result;
    } else if (cmd == "clearList") {
        QJsonObject result;
        if (m_listWidget) {
            m_listWidget->clear();
            result["success"] = true;
            result["message"] = "List cleared";
        } else {
            result["success"] = false;
            result["message"] = "List widget not available";
        }
        return result;
    }

    return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound, "Unknown command: " + cmd.toStdString());
}

std::vector<std::string> ExampleUIPlugin::available_commands() const
{
    return {"getStatus", "setProgress", "startTimer", "stopTimer", "addListItem", "clearList"};
}

void ExampleUIPlugin::setupDemoWidget(QWidget* widget)
{
    auto* layout = new QVBoxLayout(widget);
    
    // Header
    auto* headerFrame = new QFrame;
    headerFrame->setFrameStyle(QFrame::StyledPanel);
    auto* headerLayout = new QHBoxLayout(headerFrame);
    
    auto* titleLabel = new QLabel("Example UI Plugin Demo");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    auto* versionLabel = new QLabel("v" + QString::fromStdString(version().to_string()));
    versionLabel->setStyleSheet("color: #7f8c8d;");
    headerLayout->addWidget(versionLabel);
    
    layout->addWidget(headerFrame);
    
    // Main content in tabs
    auto* tabWidget = new QTabWidget;
    
    // Controls Tab
    auto* controlsTab = new QWidget;
    auto* controlsLayout = new QVBoxLayout(controlsTab);
    
    // Status and Progress
    auto* statusGroup = new QGroupBox("Status & Progress");
    auto* statusLayout = new QVBoxLayout(statusGroup);
    
    m_statusLabel = new QLabel("Plugin Status: Running");
    m_statusLabel->setStyleSheet("color: #27ae60; font-weight: bold;");
    statusLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(m_progressValue);
    statusLayout->addWidget(m_progressBar);
    
    auto* progressLayout = new QHBoxLayout;
    auto* startBtn = new QPushButton("Start Progress");
    auto* stopBtn = new QPushButton("Stop Progress");
    auto* resetBtn = new QPushButton("Reset");
    
    connect(startBtn, &QPushButton::clicked, [this]() {
        m_timer->start();
        if (m_statusLabel) m_statusLabel->setText("Plugin Status: Running");
    });
    
    connect(stopBtn, &QPushButton::clicked, [this]() {
        m_timer->stop();
        if (m_statusLabel) m_statusLabel->setText("Plugin Status: Paused");
    });
    
    connect(resetBtn, &QPushButton::clicked, [this]() {
        m_progressValue = 0;
        if (m_progressBar) m_progressBar->setValue(0);
        if (m_statusLabel) m_statusLabel->setText("Plugin Status: Reset");
    });
    
    progressLayout->addWidget(startBtn);
    progressLayout->addWidget(stopBtn);
    progressLayout->addWidget(resetBtn);
    statusLayout->addLayout(progressLayout);
    
    controlsLayout->addWidget(statusGroup);
    
    // Interactive Controls
    auto* interactiveGroup = new QGroupBox("Interactive Controls");
    auto* interactiveLayout = new QFormLayout(interactiveGroup);
    
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setValue(50);
    connect(m_slider, &QSlider::valueChanged, this, &ExampleUIPlugin::onSliderValueChanged);
    
    m_spinBox = new QSpinBox;
    m_spinBox->setRange(0, 100);
    m_spinBox->setValue(50);
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            [this](int value) { if (m_slider) m_slider->setValue(value); });
    
    interactiveLayout->addRow("Value Slider:", m_slider);
    interactiveLayout->addRow("Value Spin:", m_spinBox);
    
    controlsLayout->addWidget(interactiveGroup);
    
    tabWidget->addTab(controlsTab, "Controls");
    
    // Data Tab
    auto* dataTab = new QWidget;
    auto* dataLayout = new QVBoxLayout(dataTab);
    
    auto* textGroup = new QGroupBox("Text Input");
    auto* textLayout = new QVBoxLayout(textGroup);
    
    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText("Enter some text here...");
    m_textEdit->setMaximumHeight(100);
    connect(m_textEdit, &QTextEdit::textChanged, this, &ExampleUIPlugin::onTextChanged);
    textLayout->addWidget(m_textEdit);
    
    dataLayout->addWidget(textGroup);
    
    auto* listGroup = new QGroupBox("Dynamic List");
    auto* listLayout = new QVBoxLayout(listGroup);
    
    m_listWidget = new QListWidget;
    listLayout->addWidget(m_listWidget);
    
    auto* listButtonLayout = new QHBoxLayout;
    auto* addItemBtn = new QPushButton("Add Item");
    auto* removeItemBtn = new QPushButton("Remove Selected");
    auto* clearListBtn = new QPushButton("Clear All");
    
    connect(addItemBtn, &QPushButton::clicked, [this]() {
        QString text = QString("Item %1").arg(m_listWidget->count() + 1);
        m_listWidget->addItem(text);
    });
    
    connect(removeItemBtn, &QPushButton::clicked, [this]() {
        auto* item = m_listWidget->currentItem();
        if (item) {
            delete m_listWidget->takeItem(m_listWidget->row(item));
        }
    });
    
    connect(clearListBtn, &QPushButton::clicked, [this]() {
        m_listWidget->clear();
    });
    
    listButtonLayout->addWidget(addItemBtn);
    listButtonLayout->addWidget(removeItemBtn);
    listButtonLayout->addWidget(clearListBtn);
    listLayout->addLayout(listButtonLayout);
    
    dataLayout->addWidget(listGroup);
    
    tabWidget->addTab(dataTab, "Data");
    
    // Add embedded demo widgets
    tabWidget->addTab(new InteractiveDemo, "Calculator");
    tabWidget->addTab(new DataVisualization, "Visualization");
    
    layout->addWidget(tabWidget);
    
    // Start timer if animations enabled
    if (m_animationsEnabled) {
        m_timer->start();
    }
}

void ExampleUIPlugin::setupConfigurationWidget(QWidget* widget)
{
    auto* layout = new QVBoxLayout(widget);

    auto* settingsGroup = new QGroupBox("Plugin Settings");
    auto* settingsLayout = new QFormLayout(settingsGroup);

    // Theme selection
    m_themeCombo = new QComboBox;
    m_themeCombo->addItems({"Default", "Dark", "Light", "Blue", "Green"});
    m_themeCombo->setCurrentText(m_currentTheme);
    connect(m_themeCombo, &QComboBox::currentTextChanged, this, &ExampleUIPlugin::onConfigurationChanged);
    settingsLayout->addRow("Theme:", m_themeCombo);

    // Animations
    m_enableAnimations = new QCheckBox;
    m_enableAnimations->setChecked(m_animationsEnabled);
    connect(m_enableAnimations, &QCheckBox::toggled, this, &ExampleUIPlugin::onConfigurationChanged);
    settingsLayout->addRow("Enable Animations:", m_enableAnimations);

    // Update interval
    m_updateInterval = new QSpinBox;
    m_updateInterval->setRange(100, 5000);
    m_updateInterval->setSuffix(" ms");
    m_updateInterval->setValue(m_timerInterval);
    connect(m_updateInterval, QOverload<int>::of(&QSpinBox::valueChanged), this, &ExampleUIPlugin::onConfigurationChanged);
    settingsLayout->addRow("Update Interval:", m_updateInterval);

    layout->addWidget(settingsGroup);

    // Buttons
    auto* buttonLayout = new QHBoxLayout;
    auto* applyBtn = new QPushButton("Apply");
    auto* resetBtn = new QPushButton("Reset to Defaults");

    connect(applyBtn, &QPushButton::clicked, [this]() {
        // Apply current configuration
        QJsonObject config = current_configuration();
        configure(config);
        QMessageBox::information(m_configWidget, "Settings", "Configuration applied successfully!");
    });

    connect(resetBtn, &QPushButton::clicked, [this]() {
        auto defaults_opt = default_configuration();
        QJsonObject defaults = defaults_opt ? *defaults_opt : QJsonObject();
        configure(defaults);

        // Update UI
        m_themeCombo->setCurrentText(defaults["theme"].toString());
        m_enableAnimations->setChecked(defaults["animationsEnabled"].toBool());
        m_updateInterval->setValue(defaults["updateInterval"].toInt());

        QMessageBox::information(m_configWidget, "Settings", "Configuration reset to defaults!");
    });

    buttonLayout->addWidget(applyBtn);
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);
    layout->addStretch();
}

void ExampleUIPlugin::onButtonClicked()
{
    qDebug() << "Button clicked in Example UI Plugin";
}

void ExampleUIPlugin::onSliderValueChanged(int value)
{
    if (m_spinBox) {
        m_spinBox->setValue(value);
    }
    qDebug() << "Slider value changed to:" << value;
}

void ExampleUIPlugin::onTimerTimeout()
{
    updateProgress();
}

void ExampleUIPlugin::onTextChanged()
{
    if (m_textEdit) {
        qDebug() << "Text changed, length:" << m_textEdit->toPlainText().length();
    }
}

void ExampleUIPlugin::onConfigurationChanged()
{
    if (!m_configWidget) return;

    // Update configuration object
    m_configuration["theme"] = m_themeCombo->currentText();
    m_configuration["animationsEnabled"] = m_enableAnimations->isChecked();
    m_configuration["updateInterval"] = m_updateInterval->value();

    // Apply changes immediately
    m_currentTheme = m_themeCombo->currentText();
    m_animationsEnabled = m_enableAnimations->isChecked();
    m_timerInterval = m_updateInterval->value();

    if (m_timer) {
        m_timer->setInterval(m_timerInterval);
    }
}

void ExampleUIPlugin::updateProgress()
{
    m_progressValue = (m_progressValue + 1) % 101;
    if (m_progressBar) {
        m_progressBar->setValue(m_progressValue);
    }

    if (m_statusLabel) {
        m_statusLabel->setText(QString("Plugin Status: Running (%1%)").arg(m_progressValue));
    }
}

void ExampleUIPlugin::saveWidgetState()
{
    // Save current widget state to configuration
    if (m_slider) {
        m_configuration["sliderValue"] = m_slider->value();
    }
    if (m_textEdit) {
        m_configuration["textContent"] = m_textEdit->toPlainText();
    }
    if (m_listWidget) {
        QStringList items;
        for (int i = 0; i < m_listWidget->count(); ++i) {
            items << m_listWidget->item(i)->text();
        }
        m_configuration["listItems"] = QJsonArray::fromStringList(items);
    }
}

void ExampleUIPlugin::loadWidgetState()
{
    // Load widget state from configuration
    if (m_slider && m_configuration.contains("sliderValue")) {
        m_slider->setValue(m_configuration["sliderValue"].toInt());
    }
    if (m_textEdit && m_configuration.contains("textContent")) {
        m_textEdit->setPlainText(m_configuration["textContent"].toString());
    }
    if (m_listWidget && m_configuration.contains("listItems")) {
        QJsonArray items = m_configuration["listItems"].toArray();
        for (const auto& item : items) {
            m_listWidget->addItem(item.toString());
        }
    }
}

// InteractiveDemo Implementation
InteractiveDemo::InteractiveDemo(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void InteractiveDemo::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Interactive Calculator Demo");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    auto* formLayout = new QFormLayout;

    m_input1 = new QLineEdit;
    m_input1->setPlaceholderText("Enter first number");
    formLayout->addRow("Number 1:", m_input1);

    m_operation = new QComboBox;
    m_operation->addItems({"+", "-", "*", "/"});
    formLayout->addRow("Operation:", m_operation);

    m_input2 = new QLineEdit;
    m_input2->setPlaceholderText("Enter second number");
    formLayout->addRow("Number 2:", m_input2);

    layout->addLayout(formLayout);

    auto* buttonLayout = new QHBoxLayout;
    m_calculateBtn = new QPushButton("Calculate");
    m_resetBtn = new QPushButton("Reset");

    buttonLayout->addWidget(m_calculateBtn);
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout);

    m_result = new QLabel("Result: ");
    m_result->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; padding: 10px; border: 1px solid #bdc3c7; border-radius: 4px;");
    layout->addWidget(m_result);

    layout->addStretch();

    connect(m_calculateBtn, &QPushButton::clicked, this, &InteractiveDemo::onCalculateClicked);
    connect(m_resetBtn, &QPushButton::clicked, this, &InteractiveDemo::onResetClicked);
    connect(m_operation, &QComboBox::currentTextChanged, this, &InteractiveDemo::onModeChanged);
}

void InteractiveDemo::onCalculateClicked()
{
    bool ok1, ok2;
    double num1 = m_input1->text().toDouble(&ok1);
    double num2 = m_input2->text().toDouble(&ok2);

    if (!ok1 || !ok2) {
        m_result->setText("Result: Invalid input");
        m_result->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c; padding: 10px; border: 1px solid #e74c3c; border-radius: 4px;");
        return;
    }

    double result = 0;
    QString op = m_operation->currentText();

    if (op == "+") {
        result = num1 + num2;
    } else if (op == "-") {
        result = num1 - num2;
    } else if (op == "*") {
        result = num1 * num2;
    } else if (op == "/") {
        if (num2 == 0) {
            m_result->setText("Result: Division by zero");
            m_result->setStyleSheet("font-size: 14px; font-weight: bold; color: #e74c3c; padding: 10px; border: 1px solid #e74c3c; border-radius: 4px;");
            return;
        }
        result = num1 / num2;
    }

    m_result->setText(QString("Result: %1").arg(result));
    m_result->setStyleSheet("font-size: 14px; font-weight: bold; color: #27ae60; padding: 10px; border: 1px solid #27ae60; border-radius: 4px;");
}

void InteractiveDemo::onResetClicked()
{
    m_input1->clear();
    m_input2->clear();
    m_operation->setCurrentIndex(0);
    m_result->setText("Result: ");
    m_result->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; padding: 10px; border: 1px solid #bdc3c7; border-radius: 4px;");
}

void InteractiveDemo::onModeChanged()
{
    // Auto-calculate if both inputs are valid
    if (!m_input1->text().isEmpty() && !m_input2->text().isEmpty()) {
        onCalculateClicked();
    }
}

// DataVisualization Implementation
DataVisualization::DataVisualization(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void DataVisualization::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Data Visualization Demo");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    auto* splitter = new QSplitter(Qt::Horizontal);

    // Left side - Data input
    auto* leftWidget = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftWidget);

    auto* inputGroup = new QGroupBox("Data Input");
    auto* inputLayout = new QVBoxLayout(inputGroup);

    auto* inputRow = new QHBoxLayout;
    auto* valueInput = new QLineEdit;
    valueInput->setPlaceholderText("Enter value (0-100)");
    m_addBtn = new QPushButton("Add");

    inputRow->addWidget(valueInput);
    inputRow->addWidget(m_addBtn);
    inputLayout->addLayout(inputRow);

    m_clearBtn = new QPushButton("Clear All Data");
    inputLayout->addWidget(m_clearBtn);

    leftLayout->addWidget(inputGroup);

    // Data list
    auto* listGroup = new QGroupBox("Data Points");
    auto* listLayout = new QVBoxLayout(listGroup);

    m_dataList = new QListWidget;
    listLayout->addWidget(m_dataList);

    leftLayout->addWidget(listGroup);

    splitter->addWidget(leftWidget);

    // Right side - Visualization
    auto* rightWidget = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightWidget);

    auto* vizGroup = new QGroupBox("Statistics");
    auto* vizLayout = new QFormLayout(vizGroup);

    m_minBar = new QProgressBar;
    m_minBar->setRange(0, 100);
    vizLayout->addRow("Minimum:", m_minBar);

    m_maxBar = new QProgressBar;
    m_maxBar->setRange(0, 100);
    vizLayout->addRow("Maximum:", m_maxBar);

    m_avgBar = new QProgressBar;
    m_avgBar->setRange(0, 100);
    vizLayout->addRow("Average:", m_avgBar);

    m_statsLabel = new QLabel("No data");
    m_statsLabel->setStyleSheet("padding: 10px; border: 1px solid #bdc3c7; border-radius: 4px;");
    vizLayout->addRow("Details:", m_statsLabel);

    rightLayout->addWidget(vizGroup);
    rightLayout->addStretch();

    splitter->addWidget(rightWidget);
    splitter->setSizes({300, 400});

    layout->addWidget(splitter);

    // Connect signals
    connect(m_addBtn, &QPushButton::clicked, [this, valueInput]() {
        bool ok;
        double value = valueInput->text().toDouble(&ok);
        if (ok && value >= 0 && value <= 100) {
            addDataPoint(value);
            valueInput->clear();
        } else {
            QMessageBox::warning(this, "Invalid Input", "Please enter a number between 0 and 100");
        }
    });

    connect(m_clearBtn, &QPushButton::clicked, this, &DataVisualization::clearData);
    connect(valueInput, &QLineEdit::returnPressed, [this]() {
        m_addBtn->click();
    });
}

void DataVisualization::addDataPoint(double value)
{
    m_data.append(value);
    m_dataList->addItem(QString("Point %1: %2").arg(m_data.size()).arg(value));
    updateChart();
}

void DataVisualization::clearData()
{
    m_data.clear();
    m_dataList->clear();
    updateChart();
}

void DataVisualization::setDataRange(double min, double max)
{
    m_minValue = min;
    m_maxValue = max;
    updateChart();
}

void DataVisualization::updateChart()
{
    if (m_data.isEmpty()) {
        m_minBar->setValue(0);
        m_maxBar->setValue(0);
        m_avgBar->setValue(0);
        m_statsLabel->setText("No data");
        return;
    }

    double min = *std::min_element(m_data.begin(), m_data.end());
    double max = *std::max_element(m_data.begin(), m_data.end());
    double sum = std::accumulate(m_data.begin(), m_data.end(), 0.0);
    double avg = sum / m_data.size();

    m_minBar->setValue(static_cast<int>(min));
    m_maxBar->setValue(static_cast<int>(max));
    m_avgBar->setValue(static_cast<int>(avg));

    QString stats = QString("Count: %1\nMin: %2\nMax: %3\nAvg: %4\nSum: %5")
                    .arg(m_data.size())
                    .arg(min, 0, 'f', 2)
                    .arg(max, 0, 'f', 2)
                    .arg(avg, 0, 'f', 2)
                    .arg(sum, 0, 'f', 2);

    m_statsLabel->setText(stats);
}

// PluginSettings Implementation
PluginSettings::PluginSettings(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void PluginSettings::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel("Plugin Settings");
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px;");
    layout->addWidget(titleLabel);

    auto* settingsGroup = new QGroupBox("General Settings");
    auto* settingsLayout = new QFormLayout(settingsGroup);

    m_enableLogging = new QCheckBox;
    m_enableLogging->setChecked(true);
    settingsLayout->addRow("Enable Logging:", m_enableLogging);

    m_autoSave = new QCheckBox;
    m_autoSave->setChecked(true);
    settingsLayout->addRow("Auto Save:", m_autoSave);

    m_maxItems = new QSpinBox;
    m_maxItems->setRange(10, 1000);
    m_maxItems->setValue(100);
    settingsLayout->addRow("Max Items:", m_maxItems);

    m_logLevel = new QComboBox;
    m_logLevel->addItems({"Debug", "Info", "Warning", "Error"});
    m_logLevel->setCurrentText("Info");
    settingsLayout->addRow("Log Level:", m_logLevel);

    m_customPath = new QLineEdit;
    m_customPath->setPlaceholderText("Enter custom path...");
    settingsLayout->addRow("Custom Path:", m_customPath);

    auto* opacityLayout = new QHBoxLayout;
    m_opacity = new QSlider(Qt::Horizontal);
    m_opacity->setRange(0, 100);
    m_opacity->setValue(100);
    m_opacityLabel = new QLabel("100%");
    opacityLayout->addWidget(m_opacity);
    opacityLayout->addWidget(m_opacityLabel);
    settingsLayout->addRow("Opacity:", opacityLayout);

    layout->addWidget(settingsGroup);
    layout->addStretch();

    // Connect signals
    connect(m_enableLogging, &QCheckBox::toggled, this, &PluginSettings::onSettingChanged);
    connect(m_autoSave, &QCheckBox::toggled, this, &PluginSettings::onSettingChanged);
    connect(m_maxItems, QOverload<int>::of(&QSpinBox::valueChanged), this, &PluginSettings::onSettingChanged);
    connect(m_logLevel, &QComboBox::currentTextChanged, this, &PluginSettings::onSettingChanged);
    connect(m_customPath, &QLineEdit::textChanged, this, &PluginSettings::onSettingChanged);
    connect(m_opacity, &QSlider::valueChanged, [this](int value) {
        m_opacityLabel->setText(QString("%1%").arg(value));
        onSettingChanged();
    });
}

QJsonObject PluginSettings::getSettings() const
{
    QJsonObject settings;
    settings["enableLogging"] = m_enableLogging->isChecked();
    settings["autoSave"] = m_autoSave->isChecked();
    settings["maxItems"] = m_maxItems->value();
    settings["logLevel"] = m_logLevel->currentText();
    settings["customPath"] = m_customPath->text();
    settings["opacity"] = m_opacity->value();
    return settings;
}

void PluginSettings::setSettings(const QJsonObject& settings)
{
    m_enableLogging->setChecked(settings["enableLogging"].toBool(true));
    m_autoSave->setChecked(settings["autoSave"].toBool(true));
    m_maxItems->setValue(settings["maxItems"].toInt(100));
    m_logLevel->setCurrentText(settings["logLevel"].toString("Info"));
    m_customPath->setText(settings["customPath"].toString());
    m_opacity->setValue(settings["opacity"].toInt(100));
    m_opacityLabel->setText(QString("%1%").arg(m_opacity->value()));
}

void PluginSettings::onSettingChanged()
{
    emit settingsChanged(getSettings());
}
