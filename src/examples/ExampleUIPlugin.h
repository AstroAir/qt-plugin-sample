// ExampleUIPlugin.h - Example UI Plugin demonstrating widget creation
#pragma once

#include <qtplugin/qtplugin.hpp>
#include <qtplugin/ui/ui_plugin_interface.hpp>
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

class ExampleUIPlugin : public QObject, public qtplugin::IUIPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IUIPlugin/3.0" FILE "ExampleUIPlugin.json")
    Q_INTERFACES(qtplugin::IPlugin qtplugin::IUIPlugin)

public:
    ExampleUIPlugin(QObject* parent = nullptr);
    ~ExampleUIPlugin() override;

    // IPlugin interface
    std::string id() const noexcept override { return "example-ui-plugin"; }
    std::string_view name() const noexcept override { return "Example UI Plugin"; }
    std::string_view description() const noexcept override { return "Demonstrates various UI components and interactions"; }
    qtplugin::Version version() const noexcept override { return qtplugin::Version{1, 2, 0}; }
    std::string_view author() const noexcept override { return "Plugin Framework Team"; }
    QUuid uuid() const noexcept override { return QUuid("{12345678-1234-5678-9abc-123456789abc}"); }
    std::string_view category() const noexcept override { return "UI"; }
    std::string_view homepage() const noexcept override { return "https://example.com/ui-plugin"; }
    std::string_view license() const noexcept override { return "MIT"; }
    qtplugin::PluginCapabilities capabilities() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override { return m_state; }

    // Configuration
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    // IUIPlugin interface
    std::unique_ptr<QWidget> create_widget(QWidget* parent = nullptr) override;
    std::unique_ptr<QWidget> create_configuration_widget(QWidget* parent = nullptr) override;

    // Commands
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;

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
    qtplugin::PluginState m_state = qtplugin::PluginState::Unloaded;
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
