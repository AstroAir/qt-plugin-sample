// PluginWizard.h - Plugin Creation Wizard
#pragma once

#include <QWizard>
#include <QWizardPage>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include <QFileDialog>
#include <QProgressBar>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>

class PluginTypeSelectionPage;
class PluginInfoPage;
class PluginCapabilitiesPage;
class PluginInterfacePage;
class PluginConfigurationPage;
class PluginGenerationPage;
class PluginTemplate;

class PluginWizard : public QWizard
{
    Q_OBJECT

public:
    enum PageId {
        TypeSelectionPage,
        InfoPage,
        CapabilitiesPage,
        InterfacePage,
        ConfigurationPage,
        GenerationPage
    };

    enum PluginType {
        UIPlugin,
        ServicePlugin,
        NetworkPlugin,
        DataProviderPlugin,
        ScriptingPlugin,
        CustomPlugin
    };

    explicit PluginWizard(QWidget* parent = nullptr);
    ~PluginWizard() override;

    // Wizard data access
    PluginType selectedPluginType() const;
    QJsonObject getPluginInfo() const;
    QStringList getSelectedCapabilities() const;
    QStringList getSelectedInterfaces() const;
    QJsonObject getPluginConfiguration() const;
    QString getOutputDirectory() const;

    // Template management
    void loadTemplates();
    QStringList availableTemplates() const;
    PluginTemplate* getTemplate(const QString& name) const;

signals:
    void pluginGenerated(const QString& pluginPath);
    void generationFailed(const QString& error);

public slots:
    void generatePlugin();

private slots:
    void onCurrentIdChanged(int id);
    void onFinishClicked();

private:
    void setupPages();
    void setupConnections();
    bool validateCurrentPage();

    QHash<QString, PluginTemplate*> m_templates;
    QJsonObject m_wizardData;
};

// Plugin Type Selection Page
class PluginTypeSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginTypeSelectionPage(QWidget* parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;

    PluginWizard::PluginType getSelectedType() const;

private slots:
    void onTypeSelectionChanged();
    void onTemplateSelectionChanged();

private:
    void setupUI();
    void updateTemplateList();
    void updateDescription();

    QComboBox* m_typeCombo;
    QListWidget* m_templateList;
    QTextEdit* m_descriptionText;
    QLabel* m_previewLabel;
};

// Plugin Information Page
class PluginInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginInfoPage(QWidget* parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;
    bool validatePage() override;

    QJsonObject getPluginInfo() const;

private slots:
    void onFieldChanged();
    void onBrowseOutputDirectory();
    void onGenerateUuid();

private:
    void setupUI();
    void updateCompleteStatus();
    bool validateFields() const;

    QLineEdit* m_nameEdit;
    QLineEdit* m_classNameEdit;
    QTextEdit* m_descriptionEdit;
    QLineEdit* m_authorEdit;
    QLineEdit* m_versionEdit;
    QLineEdit* m_homepageEdit;
    QComboBox* m_licenseCombo;
    QLineEdit* m_uuidEdit;
    QLineEdit* m_outputDirEdit;
    QPushButton* m_browseBtn;
    QPushButton* m_generateUuidBtn;
};

// Plugin Capabilities Page
class PluginCapabilitiesPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginCapabilitiesPage(QWidget* parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

    QStringList getSelectedCapabilities() const;
    QJsonObject getCapabilityConfiguration() const;

private slots:
    void onCapabilityToggled();
    void onCapabilityConfigChanged();

private:
    void setupUI();
    void updateCapabilityConfig();

    QTreeWidget* m_capabilityTree;
    QTabWidget* m_configTabs;
    QHash<QString, QWidget*> m_configWidgets;
};

// Plugin Interface Page
class PluginInterfacePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginInterfacePage(QWidget* parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

    QStringList getSelectedInterfaces() const;
    QJsonObject getInterfaceConfiguration() const;

private slots:
    void onInterfaceToggled();
    void onMethodToggled();
    void onAddCustomMethod();
    void onRemoveCustomMethod();

private:
    void setupUI();
    void updateInterfaceMethods();
    void addCustomMethodWidget();

    QListWidget* m_interfaceList;
    QTreeWidget* m_methodTree;
    QWidget* m_customMethodsWidget;
    QVBoxLayout* m_customMethodsLayout;
    QPushButton* m_addMethodBtn;
    int m_customMethodCounter;
};

// Plugin Configuration Page
class PluginConfigurationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginConfigurationPage(QWidget* parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

    QJsonObject getConfiguration() const;

private slots:
    void onAddConfigItem();
    void onRemoveConfigItem();
    void onConfigItemChanged();
    void onImportConfig();
    void onExportConfig();

private:
    void setupUI();
    void addConfigItemWidget(const QString& key = "", const QString& type = "string", const QVariant& defaultValue = QVariant());
    void updateConfigPreview();

    QWidget* m_configItemsWidget;
    QVBoxLayout* m_configItemsLayout;
    QTextEdit* m_configPreview;
    QPushButton* m_addItemBtn;
    QPushButton* m_importBtn;
    QPushButton* m_exportBtn;
    QList<QWidget*> m_configItemWidgets;
};

// Plugin Generation Page
class PluginGenerationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PluginGenerationPage(QWidget* parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

public slots:
    void generatePlugin();

private slots:
    void onGenerationProgress(int percentage);
    void onGenerationFinished();
    void onGenerationError(const QString& error);
    void onOpenOutputDirectory();
    void onOpenInIDE();

private:
    void setupUI();
    void updateSummary();

    QTextEdit* m_summaryText;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QPushButton* m_openDirBtn;
    QPushButton* m_openIDEBtn;
    bool m_generationComplete;
};

// Plugin Template Class
class PluginTemplate : public QObject
{
    Q_OBJECT

public:
    explicit PluginTemplate(const QString& name, QObject* parent = nullptr);

    QString name() const { return m_name; }
    QString description() const { return m_description; }
    PluginWizard::PluginType type() const { return m_type; }
    QStringList requiredCapabilities() const { return m_requiredCapabilities; }
    QStringList defaultInterfaces() const { return m_defaultInterfaces; }
    QJsonObject defaultConfiguration() const { return m_defaultConfiguration; }

    void setDescription(const QString& description) { m_description = description; }
    void setType(PluginWizard::PluginType type) { m_type = type; }
    void setRequiredCapabilities(const QStringList& capabilities) { m_requiredCapabilities = capabilities; }
    void setDefaultInterfaces(const QStringList& interfaces) { m_defaultInterfaces = interfaces; }
    void setDefaultConfiguration(const QJsonObject& config) { m_defaultConfiguration = config; }

    // Template files
    void addTemplateFile(const QString& relativePath, const QString& content);
    QStringList templateFiles() const;
    QString getTemplateFileContent(const QString& relativePath) const;

    // Code generation
    bool generatePlugin(const QJsonObject& wizardData, const QString& outputDir);

signals:
    void generationProgress(int percentage);
    void generationFinished();
    void generationError(const QString& error);

private:
    QString processTemplate(const QString& templateContent, const QJsonObject& data);
    QString generateClassName(const QString& pluginName);
    QString generateFileName(const QString& className, const QString& extension);
    bool createDirectoryStructure(const QString& outputDir);
    bool writeFile(const QString& filePath, const QString& content);

    QString m_name;
    QString m_description;
    PluginWizard::PluginType m_type;
    QStringList m_requiredCapabilities;
    QStringList m_defaultInterfaces;
    QJsonObject m_defaultConfiguration;
    QHash<QString, QString> m_templateFiles;
};

// Plugin Generator Class
class PluginGenerator : public QObject
{
    Q_OBJECT

public:
    explicit PluginGenerator(QObject* parent = nullptr);

    bool generatePlugin(const QJsonObject& wizardData, PluginTemplate* pluginTemplate, const QString& outputDir);

signals:
    void progress(int percentage);
    void finished();
    void error(const QString& message);

private slots:
    void performGeneration();

private:
    bool validateInput();
    bool createProjectStructure();
    bool generateSourceFiles();
    bool generateConfigFiles();
    bool generateDocumentation();
    bool generateBuildFiles();
    bool writeFile(const QString& filePath, const QString& content);

    QJsonObject m_wizardData;
    PluginTemplate* m_template;
    QString m_outputDir;
    QString m_pluginDir;
    QStringList m_generatedFiles;
};
