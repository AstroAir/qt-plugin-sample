#pragma once

#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QMap>
#include <memory>

// Forward declarations
class PluginTemplateEngine;

// Enums for template configuration
enum class PluginTemplateType {
    UIPlugin,
    ServicePlugin,
    NetworkPlugin,
    DataProviderPlugin,
    ScriptingPlugin,
    CustomPlugin
};

enum class TemplateComplexity {
    Basic,
    Standard,
    Advanced,
    Expert
};

// Structure to hold code generation options
struct CodeGenerationOptions {
    QString pluginName;
    QString pluginDescription;
    QString authorName;
    QString authorEmail;
    QString organizationName;
    QString pluginVersion;
    QString qtVersion;
    QString cppStandard;
    QString outputDirectory;
    QString customNamespace;
    
    PluginTemplateType templateType;
    TemplateComplexity complexity;
    
    QStringList interfaces;
    QStringList features;
    
    bool generateTests;
    bool generateDocumentation;
    bool generateCMakeFiles;
    bool generateExamples;
    bool useNamespace;
};

/**
 * @brief Plugin Template Generation Wizard
 * 
 * A comprehensive wizard for generating plugin templates with various
 * configuration options and complexity levels.
 */
class PluginTemplateGeneratorWizard : public QWizard
{
    Q_OBJECT

public:
    // Wizard page IDs
    enum WizardPages {
        IntroPage,
        BasicInfoPage,
        TemplateSelectionPage,
        InterfacePage,
        FeaturesPage,
        OutputPage,
        PreviewPage,
        GenerationPage
    };

    explicit PluginTemplateGeneratorWizard(QWidget* parent = nullptr);
    ~PluginTemplateGeneratorWizard();

    // Configuration methods
    CodeGenerationOptions getGenerationOptions() const;
    void setDefaultOptions(const CodeGenerationOptions& options);

protected:
    void initializePage(int id) override;
    bool validateCurrentPage() override;
    void cleanupPage(int id) override;

private slots:
    void onTemplateTypeChanged();
    void onComplexityChanged();
    void onInterfaceSelectionChanged();
    void onFeatureSelectionChanged();
    void onOutputDirectoryBrowse();
    void onPreviewRequested();
    void onGenerationStarted();
    void onGenerationProgress(int percentage, const QString& currentFile);
    void onGenerationCompleted(bool success, const QString& outputPath);

private:
    void setupWizardPages();
    void updatePreview();
    bool validateOptions();

    CodeGenerationOptions m_options;
    std::unique_ptr<PluginTemplateEngine> m_templateEngine;

    // Forward declarations for wizard pages
    class IntroductionPage;
    class BasicInformationPage;
    class TemplateSelectionPage;
    class InterfaceSelectionPage;
    class FeatureSelectionPage;
    class OutputConfigurationPage;
    class PreviewPage;
    class GenerationPage;
};

/**
 * @brief Introduction page for the wizard
 */
class PluginTemplateGeneratorWizard::IntroductionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit IntroductionPage(QWidget* parent = nullptr);

protected:
    void initializePage() override;
    bool isComplete() const override;

private:
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_featuresLabel;
};

/**
 * @brief Basic information page for plugin metadata
 */
class PluginTemplateGeneratorWizard::BasicInformationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit BasicInformationPage(QWidget* parent = nullptr);

    // Getters for form data
    QString getPluginName() const;
    QString getPluginDescription() const;
    QString getAuthorName() const;
    QString getAuthorEmail() const;
    QString getOrganizationName() const;
    QString getPluginVersion() const;

protected:
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;

private slots:
    void onPluginNameChanged();
    void onFieldChanged();

private:
    void setupUI();
    bool validateInput() const;
    void updatePluginNamePreview();

    QLineEdit* m_pluginNameEdit;
    QTextEdit* m_pluginDescriptionEdit;
    QLineEdit* m_authorNameEdit;
    QLineEdit* m_authorEmailEdit;
    QLineEdit* m_organizationEdit;
    QLineEdit* m_versionEdit;
    QComboBox* m_qtVersionCombo;
    QComboBox* m_cppStandardCombo;
    QCheckBox* m_useNamespaceCheck;
    QLineEdit* m_namespaceEdit;
};

/**
 * @brief Template selection page for choosing plugin type and complexity
 */
class PluginTemplateGeneratorWizard::TemplateSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit TemplateSelectionPage(QWidget* parent = nullptr);
    bool isComplete() const override;
};

/**
 * @brief Interface selection page for choosing plugin interfaces
 */
class PluginTemplateGeneratorWizard::InterfaceSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit InterfaceSelectionPage(QWidget* parent = nullptr);
    bool isComplete() const override;
    QStringList getSelectedInterfaces() const;

private:
    QMap<QString, QCheckBox*> m_interfaceChecks;
};

/**
 * @brief Feature selection page for choosing additional features
 */
class PluginTemplateGeneratorWizard::FeatureSelectionPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit FeatureSelectionPage(QWidget* parent = nullptr);
    bool isComplete() const override;
    QStringList getSelectedFeatures() const;

private:
    QMap<QString, QCheckBox*> m_featureChecks;
};

/**
 * @brief Output configuration page for setting output directory and options
 */
class PluginTemplateGeneratorWizard::OutputConfigurationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit OutputConfigurationPage(QWidget* parent = nullptr);
    bool isComplete() const override;
    QString getOutputDirectory() const;
    bool getGenerateTests() const;
    bool getGenerateDocs() const;
    bool getGenerateCMake() const;
    bool getGenerateExamples() const;

private:
    QLineEdit* m_outputDirEdit;
    QCheckBox* m_generateTestsCheck;
    QCheckBox* m_generateDocsCheck;
    QCheckBox* m_generateCMakeCheck;
    QCheckBox* m_generateExamplesCheck;
};

/**
 * @brief Preview page for reviewing generated files
 */
class PluginTemplateGeneratorWizard::PreviewPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit PreviewPage(QWidget* parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;

private slots:
    void updatePreview(const QString& fileName);

private:
    QListWidget* m_fileList;
    QPlainTextEdit* m_previewEdit;
    void updateFileList();
};

/**
 * @brief Generation page for showing progress during template generation
 */
class PluginTemplateGeneratorWizard::GenerationPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit GenerationPage(QWidget* parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;

private:
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QTextEdit* m_logEdit;
};
