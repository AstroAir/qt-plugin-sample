// ExampleUIPlugin.h - Example UI Plugin demonstrating widget creation
#pragma once

#include "../core/PluginInterface.h"
#include "../core/AdvancedInterfaces.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QTimer>
#include <QGroupBox>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QJsonObject>
#include <QUuid>
#include <QVersionNumber>

class ExampleUIPlugin : public QObject, public IUIPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.IPlugin/2.0" FILE "ExampleUIPlugin.json")
    Q_INTERFACES(IPlugin IUIPlugin)

public:
    ExampleUIPlugin(QObject* parent = nullptr);
    ~ExampleUIPlugin() override;

    // IPlugin interface
    QString name() const override { return "Example UI Plugin"; }
    QString description() const override { return "Demonstrates various UI components and interactions"; }
    QVersionNumber version() const override { return QVersionNumber(1, 2, 0); }
    QString author() const override { return "Plugin Framework Team"; }
    QUuid uuid() const override { return QUuid("{12345678-1234-5678-9abc-123456789abc}"); }
    QString category() const override { return "UI"; }
    QString homepage() const override { return "https://example.com/ui-plugin"; }
    QString license() const override { return "MIT"; }

    PluginCapabilities capabilities() const override {
        return PluginCapability::UI | PluginCapability::Configuration;
    }

    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override { return m_initialized; }
    PluginStatus status() const override { return m_status; }

    // Configuration
    QJsonObject defaultConfiguration() const override;
    bool configure(const QJsonObject& config) override;
    QJsonObject currentConfiguration() const override;

    // IUIPlugin interface
    std::unique_ptr<QWidget> createWidget(QWidget* parent = nullptr) override;
    QWidget* createConfigurationWidget(QWidget* parent = nullptr) override;

    // Commands
    QVariant executeCommand(const QString& command, const QVariantMap& params = {}) override;
    QStringList availableCommands() const override;

private slots:
    void onButtonClicked();
    void onSliderValueChanged(int value);
    void onTimerTimeout();
    void onTextChanged();
    void onConfigurationChanged();

private:
    void setupDemoWidget(QWidget* widget);
    void setupConfigurationWidget(QWidget* widget);
    void updateProgress();
    void saveWidgetState();
    void loadWidgetState();

    bool m_initialized = false;
    PluginStatus m_status = PluginStatus::Unknown;
    QJsonObject m_configuration;
    
    // Demo widget components
    QWidget* m_mainWidget = nullptr;
    QLabel* m_statusLabel = nullptr;
    QProgressBar* m_progressBar = nullptr;
    QSlider* m_slider = nullptr;
    QSpinBox* m_spinBox = nullptr;
    QTextEdit* m_textEdit = nullptr;
    QListWidget* m_listWidget = nullptr;
    QTimer* m_timer = nullptr;
    
    // Configuration widget components
    QWidget* m_configWidget = nullptr;
    QCheckBox* m_enableAnimations = nullptr;
    QComboBox* m_themeCombo = nullptr;
    QSpinBox* m_updateInterval = nullptr;
    
    int m_progressValue = 0;
    QString m_currentTheme = "Default";
    bool m_animationsEnabled = true;
    int m_timerInterval = 1000;
};

// Demo Widget Classes
class InteractiveDemo : public QWidget
{
    Q_OBJECT

public:
    explicit InteractiveDemo(QWidget* parent = nullptr);

private slots:
    void onCalculateClicked();
    void onResetClicked();
    void onModeChanged();

private:
    void setupUI();
    void updateResult();

    QLineEdit* m_input1;
    QLineEdit* m_input2;
    QComboBox* m_operation;
    QLabel* m_result;
    QPushButton* m_calculateBtn;
    QPushButton* m_resetBtn;
};

class DataVisualization : public QWidget
{
    Q_OBJECT

public:
    explicit DataVisualization(QWidget* parent = nullptr);

public slots:
    void addDataPoint(double value);
    void clearData();
    void setDataRange(double min, double max);

private:
    void setupUI();
    void updateChart();

    QListWidget* m_dataList;
    QProgressBar* m_minBar;
    QProgressBar* m_maxBar;
    QProgressBar* m_avgBar;
    QLabel* m_statsLabel;
    QPushButton* m_addBtn;
    QPushButton* m_clearBtn;
    
    QList<double> m_data;
    double m_minValue = 0.0;
    double m_maxValue = 100.0;
};

class PluginSettings : public QWidget
{
    Q_OBJECT

public:
    explicit PluginSettings(QWidget* parent = nullptr);
    
    QJsonObject getSettings() const;
    void setSettings(const QJsonObject& settings);

signals:
    void settingsChanged(const QJsonObject& settings);

private slots:
    void onSettingChanged();

private:
    void setupUI();

    QCheckBox* m_enableLogging;
    QCheckBox* m_autoSave;
    QSpinBox* m_maxItems;
    QComboBox* m_logLevel;
    QLineEdit* m_customPath;
    QSlider* m_opacity;
    QLabel* m_opacityLabel;
};
