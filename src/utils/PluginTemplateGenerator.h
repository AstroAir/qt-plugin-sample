// PluginTemplateGenerator.h - Advanced Plugin Template Generation System
#pragma once

#include <QObject>
#include <QDialog>
#include <QWizard>
#include <QWizardPage>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QUuid>
#include <QVersionNumber>
#include <memory>

// Forward declarations
class PluginTemplateEngine;
class TemplateVariable;
class CodeGenerator;

// Plugin template types
enum class PluginTemplateType {
    UIPlugin,
    ServicePlugin,
    NetworkPlugin,
    DataProviderPlugin,
    ScriptingPlugin,
    CustomPlugin
};

// Template complexity levels
enum class TemplateComplexity {
    Basic,      // Minimal implementation
    Standard,   // Common features included
    Advanced,   // Full-featured with examples
    Expert      // All interfaces and advanced patterns
};

// Code generation options
struct CodeGenerationOptions {
    QString pluginName;
    QString pluginDescription;
    QString authorName;
    QString authorEmail;
    QString organizationName;
    QString pluginVersion;
    QString qtVersion;
    QString cppStandard;
    PluginTemplateType templateType;
    TemplateComplexity complexity;
    QStringList interfaces;
    QStringList features;
    QJsonObject customProperties;
    QString outputDirectory;
    bool generateTests;
    bool generateDocumentation;
    bool generateCMakeFiles;
    bool generateExamples;
    bool useNamespace;
    QString namespacePrefix;
};

// Template variable for dynamic content generation
class TemplateVariable {
public:
    QString name;
    QString value;
    QString description;
    QString defaultValue;
    bool isRequired = false;
    QStringList allowedValues;
    
    TemplateVariable() = default;
    TemplateVariable(const QString& n, const QString& v, const QString& desc = "", bool req = false)
        : name(n), value(v), description(desc), isRequired(req) {}
};

// Plugin template engine for processing templates
class PluginTemplateEngine : public QObject {
    Q_OBJECT

public:
    explicit PluginTemplateEngine(QObject* parent = nullptr);
    ~PluginTemplateEngine() override;

    // Template management
    bool loadTemplate(const QString& templatePath);
    bool loadTemplateFromResource(const QString& resourcePath);
    QStringList availableTemplates() const;
    QString templateDescription(const QString& templateName) const;
    
    // Variable management
    void setVariable(const QString& name, const QString& value);
    void setVariables(const QMap<QString, QString>& variables);
    QString getVariable(const QString& name) const;
    QStringList getRequiredVariables() const;
    QStringList getAllVariables() const;
    
    // Template processing
    QString processTemplate(const QString& templateContent) const;
    QString processTemplateFile(const QString& templatePath) const;
    bool generateFromTemplate(const QString& templateName, const CodeGenerationOptions& options);
    
    // Validation
    bool validateTemplate(const QString& templateContent) const;
    QStringList validateOptions(const CodeGenerationOptions& options) const;

signals:
    void templateProcessed(const QString& templateName);
    void generationProgress(int percentage, const QString& currentFile);
    void generationCompleted(bool success, const QString& outputPath);
    void errorOccurred(const QString& error);

private slots:
    void onFileGenerated(const QString& filePath);

private:
    struct TemplateInfo {
        QString name;
        QString description;
        QString filePath;
        PluginTemplateType type;
        TemplateComplexity complexity;
        QStringList requiredVariables;
        QJsonObject metadata;
    };

    QMap<QString, TemplateInfo> m_templates;
    QMap<QString, QString> m_variables;
    std::unique_ptr<CodeGenerator> m_codeGenerator;
    
    // Template processing helpers
    QString replaceVariables(const QString& content) const;
    QString processConditionals(const QString& content) const;
    QString processLoops(const QString& content) const;
    bool createDirectoryStructure(const QString& basePath, const QStringList& directories);
    bool copyTemplateFiles(const QString& templateDir, const QString& outputDir);

    // Missing method declarations
    void loadBuiltInTemplates();
    void setupVariablesFromOptions(const CodeGenerationOptions& options);
    QString formatClassName(const QString& pluginName) const;
    QString formatFileName(const QString& pluginName) const;
};

// Code generator for creating plugin files
class CodeGenerator : public QObject {
    Q_OBJECT

public:
    explicit CodeGenerator(QObject* parent = nullptr);
    ~CodeGenerator() override;

    // File generation
    bool generateHeaderFile(const CodeGenerationOptions& options, const QString& outputPath);
    bool generateSourceFile(const CodeGenerationOptions& options, const QString& outputPath);
    bool generateMetadataFile(const CodeGenerationOptions& options, const QString& outputPath);
    bool generateCMakeFile(const CodeGenerationOptions& options, const QString& outputPath);
    bool generateTestFile(const CodeGenerationOptions& options, const QString& outputPath);
    bool generateDocumentationFile(const CodeGenerationOptions& options, const QString& outputPath);
    
    // Code snippets generation
    QString generateClassDeclaration(const CodeGenerationOptions& options) const;
    QString generateInterfaceImplementation(const CodeGenerationOptions& options) const;
    QString generateConstructorImplementation(const CodeGenerationOptions& options) const;
    QString generateMethodImplementations(const CodeGenerationOptions& options) const;
    QString generateSignalSlotConnections(const CodeGenerationOptions& options) const;
    
    // Utility methods
    QString generateIncludeGuard(const QString& className) const;
    QString generateNamespaceWrapper(const QString& content, const QString& namespaceName) const;
    QString generateLicenseHeader(const CodeGenerationOptions& options) const;
    QString generatePluginMetadata(const CodeGenerationOptions& options) const;

signals:
    void fileGenerated(const QString& filePath);
    void generationProgress(int percentage);

private:
    // Code generation helpers
    QString formatClassName(const QString& pluginName) const;
    QString formatFileName(const QString& pluginName) const;
    QString formatVariableName(const QString& name) const;
    QString generateInterfaceIncludes(const QStringList& interfaces) const;
    QString generateInterfaceInheritance(const QStringList& interfaces) const;
    QString generateInterfaceMethods(const QStringList& interfaces, TemplateComplexity complexity) const;
    
    // Template content
    QMap<QString, QString> m_codeTemplates;
    void initializeCodeTemplates();
};

// Main plugin template generator wizard
class PluginTemplateGeneratorWizard : public QWizard {
    Q_OBJECT

public:
    explicit PluginTemplateGeneratorWizard(QWidget* parent = nullptr);
    ~PluginTemplateGeneratorWizard() override;

    // Wizard configuration
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
    enum PageId {
        IntroPage = 0,
        BasicInfoPage,
        TemplateSelectionPage,
        InterfacePage,
        FeaturesPage,
        OutputPage,
        PreviewPage,
        GenerationPage
    };

    // Wizard pages
    class IntroductionPage;
    class BasicInformationPage;
    class TemplateSelectionPage;
    class InterfaceSelectionPage;
    class FeatureSelectionPage;
    class OutputConfigurationPage;
    class PreviewPage;
    class GenerationPage;

    std::unique_ptr<PluginTemplateEngine> m_templateEngine;
    CodeGenerationOptions m_options;
    
    void setupWizardPages();
    void updatePreview();
    bool validateOptions();
};

// Wizard page implementations
class PluginTemplateGeneratorWizard::IntroductionPage : public QWizardPage {
    Q_OBJECT

public:
    explicit IntroductionPage(QWidget* parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;

private:
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_featuresLabel;
};

class PluginTemplateGeneratorWizard::BasicInformationPage : public QWizardPage {
    Q_OBJECT

public:
    explicit BasicInformationPage(QWidget* parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;

    // Data access
    QString getPluginName() const;
    QString getPluginDescription() const;
    QString getAuthorName() const;
    QString getAuthorEmail() const;
    QString getOrganizationName() const;
    QString getPluginVersion() const;

private slots:
    void onPluginNameChanged();
    void onFieldChanged();

private:
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
    
    void setupUI();
    bool validateInput() const;
    void updatePluginNamePreview();
};
