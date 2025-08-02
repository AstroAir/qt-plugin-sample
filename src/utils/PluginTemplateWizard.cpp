// PluginTemplateWizard.cpp - Implementation of Plugin Template Generation Wizard
#include "PluginTemplateWizard.h"
#include "PluginTemplateGenerator.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTextBrowser>
#include <QProgressBar>
#include <QSplitter>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QScrollArea>
#include <QPlainTextEdit>
#include <QTimer>
#include <QWizard>
#include <QIcon>
#include <QRegularExpression>

// PluginTemplateGeneratorWizard Implementation
PluginTemplateGeneratorWizard::PluginTemplateGeneratorWizard(QWidget* parent)
    : QWizard(parent)
    , m_templateEngine(std::make_unique<PluginTemplateEngine>(this))
{
    setWindowTitle("Plugin Template Generator");
    setWindowIcon(QIcon(":/icons/plugin_wizard.svg"));
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, true);
    setOption(QWizard::HelpButtonOnRight, false);
    
    // Set default options
    m_options.pluginVersion = "1.0.0";
    m_options.qtVersion = "6.0";
    m_options.cppStandard = "20";
    m_options.templateType = PluginTemplateType::UIPlugin;
    m_options.complexity = TemplateComplexity::Standard;
    m_options.generateTests = true;
    m_options.generateDocumentation = true;
    m_options.generateCMakeFiles = true;
    m_options.useNamespace = false;
    
    setupWizardPages();
    
    // Connect template engine signals
    connect(m_templateEngine.get(), &PluginTemplateEngine::generationProgress,
            this, &PluginTemplateGeneratorWizard::onGenerationProgress);
    connect(m_templateEngine.get(), &PluginTemplateEngine::generationCompleted,
            this, &PluginTemplateGeneratorWizard::onGenerationCompleted);
    connect(m_templateEngine.get(), &PluginTemplateEngine::errorOccurred,
            this, [this](const QString& error) {
                QMessageBox::critical(this, "Generation Error", error);
            });
}

PluginTemplateGeneratorWizard::~PluginTemplateGeneratorWizard() = default;

CodeGenerationOptions PluginTemplateGeneratorWizard::getGenerationOptions() const {
    return m_options;
}

void PluginTemplateGeneratorWizard::setDefaultOptions(const CodeGenerationOptions& options) {
    m_options = options;
}

void PluginTemplateGeneratorWizard::initializePage(int id) {
    QWizard::initializePage(id);
    
    switch (id) {
        case BasicInfoPage:
            // Update options from basic info page
            if (auto page = qobject_cast<BasicInformationPage*>(currentPage())) {
                m_options.pluginName = page->getPluginName();
                m_options.pluginDescription = page->getPluginDescription();
                m_options.authorName = page->getAuthorName();
                m_options.authorEmail = page->getAuthorEmail();
                m_options.organizationName = page->getOrganizationName();
                m_options.pluginVersion = page->getPluginVersion();
            }
            break;
        case PreviewPage:
            updatePreview();
            break;
    }
}

bool PluginTemplateGeneratorWizard::validateCurrentPage() {
    return QWizard::validateCurrentPage();
}

void PluginTemplateGeneratorWizard::cleanupPage(int id) {
    Q_UNUSED(id)
    QWizard::cleanupPage(id);
}

void PluginTemplateGeneratorWizard::onTemplateTypeChanged() {
    // Update available interfaces based on template type
    // This would be implemented to show relevant interfaces
}

void PluginTemplateGeneratorWizard::onComplexityChanged() {
    // Update available features based on complexity
    // This would be implemented to show relevant features
}

void PluginTemplateGeneratorWizard::onInterfaceSelectionChanged() {
    // Update options based on interface selection
    // This would be implemented to update m_options.interfaces
}

void PluginTemplateGeneratorWizard::onFeatureSelectionChanged() {
    // Update options based on feature selection
    // This would be implemented to update m_options.features
}

void PluginTemplateGeneratorWizard::onOutputDirectoryBrowse() {
    QString dir = QFileDialog::getExistingDirectory(this, 
        "Select Output Directory", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    
    if (!dir.isEmpty()) {
        m_options.outputDirectory = dir;
        // Update the UI to show selected directory
    }
}

void PluginTemplateGeneratorWizard::onPreviewRequested() {
    updatePreview();
}

void PluginTemplateGeneratorWizard::onGenerationStarted() {
    // Start the generation process
    if (!validateOptions()) {
        return;
    }
    
    QString templateName = "UI Plugin"; // This would be selected from UI
    m_templateEngine->generateFromTemplate(templateName, m_options);
}

void PluginTemplateGeneratorWizard::onGenerationProgress(int percentage, const QString& currentFile) {
    // Update progress bar and status
    if (auto page = qobject_cast<GenerationPage*>(currentPage())) {
        // Update progress in generation page
        Q_UNUSED(percentage)
        Q_UNUSED(currentFile)
    }
}

void PluginTemplateGeneratorWizard::onGenerationCompleted(bool success, const QString& outputPath) {
    if (success) {
        QMessageBox::information(this, "Generation Complete", 
            QString("Plugin template generated successfully in:\n%1").arg(outputPath));
        accept();
    } else {
        QMessageBox::critical(this, "Generation Failed", 
            "Failed to generate plugin template. Please check the error log.");
    }
}

void PluginTemplateGeneratorWizard::setupWizardPages() {
    setPage(IntroPage, new IntroductionPage(this));
    setPage(BasicInfoPage, new BasicInformationPage(this));
    setPage(TemplateSelectionPage, new TemplateSelectionPage(this));
    setPage(InterfacePage, new InterfaceSelectionPage(this));
    setPage(FeaturesPage, new FeatureSelectionPage(this));
    setPage(OutputPage, new OutputConfigurationPage(this));
    setPage(PreviewPage, new PreviewPage(this));
    setPage(GenerationPage, new GenerationPage(this));
}

void PluginTemplateGeneratorWizard::updatePreview() {
    // Generate preview of the files that will be created
    // This would show a preview of the generated code
}

bool PluginTemplateGeneratorWizard::validateOptions() {
    QStringList errors = m_templateEngine->validateOptions(m_options);
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Validation Errors", 
            "Please fix the following issues:\n" + errors.join("\n"));
        return false;
    }
    return true;
}

// IntroductionPage Implementation
PluginTemplateGeneratorWizard::IntroductionPage::IntroductionPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle("Welcome to Plugin Template Generator");
    setSubTitle("This wizard will help you create a new plugin template with all necessary files.");
    
    auto layout = new QVBoxLayout(this);
    
    m_titleLabel = new QLabel("Plugin Template Generator", this);
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    layout->addWidget(m_titleLabel);
    
    layout->addSpacing(20);
    
    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setText(
        "This wizard will guide you through the process of creating a new plugin template. "
        "You'll be able to:\n\n"
        "• Choose from different plugin types (UI, Service, Network, etc.)\n"
        "• Select the complexity level and features\n"
        "• Configure plugin metadata and author information\n"
        "• Generate complete source code with documentation\n"
        "• Create build files and unit tests\n\n"
        "Click Next to begin the configuration process."
    );
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet("font-size: 12px; line-height: 1.4;");
    layout->addWidget(m_descriptionLabel);
    
    layout->addSpacing(20);
    
    m_featuresLabel = new QLabel(this);
    m_featuresLabel->setText(
        "<b>Key Features:</b><br>"
        "• Multiple plugin template types<br>"
        "• Customizable complexity levels<br>"
        "• Automatic code generation<br>"
        "• CMake build system integration<br>"
        "• Unit test generation<br>"
        "• Documentation generation<br>"
        "• Qt6 and modern C++ support"
    );
    m_featuresLabel->setStyleSheet("font-size: 11px; color: #34495e; background-color: #ecf0f1; padding: 10px; border-radius: 5px;");
    layout->addWidget(m_featuresLabel);
    
    layout->addStretch();
}

void PluginTemplateGeneratorWizard::IntroductionPage::initializePage() {
    // Nothing special needed for intro page
}

bool PluginTemplateGeneratorWizard::IntroductionPage::isComplete() const {
    return true; // Always complete
}

// BasicInformationPage Implementation
PluginTemplateGeneratorWizard::BasicInformationPage::BasicInformationPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle("Basic Plugin Information");
    setSubTitle("Enter the basic information about your plugin.");
    
    setupUI();
    
    // Connect signals for validation
    connect(m_pluginNameEdit, &QLineEdit::textChanged, this, &BasicInformationPage::onPluginNameChanged);
    connect(m_pluginDescriptionEdit, &QTextEdit::textChanged, this, &BasicInformationPage::onFieldChanged);
    connect(m_authorNameEdit, &QLineEdit::textChanged, this, &BasicInformationPage::onFieldChanged);
    connect(m_authorEmailEdit, &QLineEdit::textChanged, this, &BasicInformationPage::onFieldChanged);
    connect(m_organizationEdit, &QLineEdit::textChanged, this, &BasicInformationPage::onFieldChanged);
    connect(m_versionEdit, &QLineEdit::textChanged, this, &BasicInformationPage::onFieldChanged);
    connect(m_useNamespaceCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_namespaceEdit->setEnabled(checked);
        onFieldChanged();
    });
}

void PluginTemplateGeneratorWizard::BasicInformationPage::initializePage() {
    // Set default values
    if (m_pluginNameEdit->text().isEmpty()) {
        m_pluginNameEdit->setText("MyPlugin");
    }
    if (m_pluginDescriptionEdit->toPlainText().isEmpty()) {
        m_pluginDescriptionEdit->setPlainText("A custom plugin for the application");
    }
    if (m_versionEdit->text().isEmpty()) {
        m_versionEdit->setText("1.0.0");
    }
}

bool PluginTemplateGeneratorWizard::BasicInformationPage::validatePage() {
    return validateInput();
}

bool PluginTemplateGeneratorWizard::BasicInformationPage::isComplete() const {
    return validateInput();
}

QString PluginTemplateGeneratorWizard::BasicInformationPage::getPluginName() const {
    return m_pluginNameEdit->text().trimmed();
}

QString PluginTemplateGeneratorWizard::BasicInformationPage::getPluginDescription() const {
    return m_pluginDescriptionEdit->toPlainText().trimmed();
}

QString PluginTemplateGeneratorWizard::BasicInformationPage::getAuthorName() const {
    return m_authorNameEdit->text().trimmed();
}

QString PluginTemplateGeneratorWizard::BasicInformationPage::getAuthorEmail() const {
    return m_authorEmailEdit->text().trimmed();
}

QString PluginTemplateGeneratorWizard::BasicInformationPage::getOrganizationName() const {
    return m_organizationEdit->text().trimmed();
}

QString PluginTemplateGeneratorWizard::BasicInformationPage::getPluginVersion() const {
    return m_versionEdit->text().trimmed();
}

void PluginTemplateGeneratorWizard::BasicInformationPage::onPluginNameChanged() {
    updatePluginNamePreview();
    onFieldChanged();
}

void PluginTemplateGeneratorWizard::BasicInformationPage::onFieldChanged() {
    emit completeChanged();
}

void PluginTemplateGeneratorWizard::BasicInformationPage::setupUI() {
    auto layout = new QFormLayout(this);

    // Plugin name
    m_pluginNameEdit = new QLineEdit(this);
    m_pluginNameEdit->setPlaceholderText("Enter plugin name (e.g., MyAwesomePlugin)");
    layout->addRow("Plugin Name*:", m_pluginNameEdit);

    // Plugin description
    m_pluginDescriptionEdit = new QTextEdit(this);
    m_pluginDescriptionEdit->setMaximumHeight(80);
    m_pluginDescriptionEdit->setPlaceholderText("Enter a brief description of your plugin...");
    layout->addRow("Description*:", m_pluginDescriptionEdit);

    // Author information
    m_authorNameEdit = new QLineEdit(this);
    m_authorNameEdit->setPlaceholderText("Your name");
    layout->addRow("Author Name*:", m_authorNameEdit);

    m_authorEmailEdit = new QLineEdit(this);
    m_authorEmailEdit->setPlaceholderText("your.email@example.com");
    layout->addRow("Author Email:", m_authorEmailEdit);

    m_organizationEdit = new QLineEdit(this);
    m_organizationEdit->setPlaceholderText("Your organization or company");
    layout->addRow("Organization:", m_organizationEdit);

    // Version
    m_versionEdit = new QLineEdit(this);
    m_versionEdit->setPlaceholderText("1.0.0");
    layout->addRow("Version*:", m_versionEdit);

    // Technical settings
    layout->addRow("", new QLabel("")); // Spacer

    m_qtVersionCombo = new QComboBox(this);
    m_qtVersionCombo->addItems({"6.0", "6.1", "6.2", "6.3", "6.4", "6.5", "6.6"});
    m_qtVersionCombo->setCurrentText("6.5");
    layout->addRow("Qt Version:", m_qtVersionCombo);

    m_cppStandardCombo = new QComboBox(this);
    m_cppStandardCombo->addItems({"17", "20", "23"});
    m_cppStandardCombo->setCurrentText("20");
    layout->addRow("C++ Standard:", m_cppStandardCombo);

    // Namespace options
    m_useNamespaceCheck = new QCheckBox("Use custom namespace", this);
    layout->addRow("", m_useNamespaceCheck);

    m_namespaceEdit = new QLineEdit(this);
    m_namespaceEdit->setPlaceholderText("MyCompany::Plugins");
    m_namespaceEdit->setEnabled(false);
    layout->addRow("Namespace:", m_namespaceEdit);
}

bool PluginTemplateGeneratorWizard::BasicInformationPage::validateInput() const {
    if (getPluginName().isEmpty()) return false;
    if (getPluginDescription().isEmpty()) return false;
    if (getAuthorName().isEmpty()) return false;
    if (getPluginVersion().isEmpty()) return false;

    // Validate plugin name format
    QRegularExpression nameRegex("^[A-Za-z][A-Za-z0-9_]*$");
    if (!nameRegex.match(getPluginName()).hasMatch()) {
        return false;
    }

    // Validate version format
    QRegularExpression versionRegex(R"(^\d+\.\d+\.\d+$)");
    if (!versionRegex.match(getPluginVersion()).hasMatch()) {
        return false;
    }

    return true;
}

void PluginTemplateGeneratorWizard::BasicInformationPage::updatePluginNamePreview() {
    QString name = getPluginName();
    if (!name.isEmpty()) {
        // Show preview of generated class name and file names
        QString className = name;
        if (!className.isEmpty()) {
            className[0] = className[0].toUpper();
        }

        QString fileName = name.toLower();
        fileName.replace(QRegularExpression("[^a-z0-9_]"), "_");

        // This could update a preview label showing the generated names
    }
}

// Template Selection Page Implementation
PluginTemplateGeneratorWizard::TemplateSelectionPage::TemplateSelectionPage(QWidget* parent) : QWizardPage(parent) {
        setTitle("Template Selection");
        setSubTitle("Choose the type and complexity of your plugin template.");

        auto layout = new QVBoxLayout(this);

        // Template type selection
        auto typeGroup = new QGroupBox("Plugin Type", this);
        auto typeLayout = new QVBoxLayout(typeGroup);

        auto typeButtonGroup = new QButtonGroup(this);

        auto uiRadio = new QRadioButton("UI Plugin - Provides user interface components", this);
        auto serviceRadio = new QRadioButton("Service Plugin - Background service functionality", this);
        auto networkRadio = new QRadioButton("Network Plugin - Network communication features", this);
        auto dataRadio = new QRadioButton("Data Provider Plugin - Data processing and management", this);
        auto scriptRadio = new QRadioButton("Scripting Plugin - Scripting engine integration", this);
        auto customRadio = new QRadioButton("Custom Plugin - Basic plugin template", this);

        uiRadio->setChecked(true); // Default selection

        typeButtonGroup->addButton(uiRadio, static_cast<int>(PluginTemplateType::UIPlugin));
        typeButtonGroup->addButton(serviceRadio, static_cast<int>(PluginTemplateType::ServicePlugin));
        typeButtonGroup->addButton(networkRadio, static_cast<int>(PluginTemplateType::NetworkPlugin));
        typeButtonGroup->addButton(dataRadio, static_cast<int>(PluginTemplateType::DataProviderPlugin));
        typeButtonGroup->addButton(scriptRadio, static_cast<int>(PluginTemplateType::ScriptingPlugin));
        typeButtonGroup->addButton(customRadio, static_cast<int>(PluginTemplateType::CustomPlugin));

        typeLayout->addWidget(uiRadio);
        typeLayout->addWidget(serviceRadio);
        typeLayout->addWidget(networkRadio);
        typeLayout->addWidget(dataRadio);
        typeLayout->addWidget(scriptRadio);
        typeLayout->addWidget(customRadio);

        layout->addWidget(typeGroup);

        // Complexity selection
        auto complexityGroup = new QGroupBox("Template Complexity", this);
        auto complexityLayout = new QVBoxLayout(complexityGroup);

        auto complexityButtonGroup = new QButtonGroup(this);

        auto basicRadio = new QRadioButton("Basic - Minimal implementation with core functionality", this);
        auto standardRadio = new QRadioButton("Standard - Common features and examples included", this);
        auto advancedRadio = new QRadioButton("Advanced - Full-featured with comprehensive examples", this);
        auto expertRadio = new QRadioButton("Expert - All interfaces and advanced patterns", this);

        standardRadio->setChecked(true); // Default selection

        complexityButtonGroup->addButton(basicRadio, static_cast<int>(TemplateComplexity::Basic));
        complexityButtonGroup->addButton(standardRadio, static_cast<int>(TemplateComplexity::Standard));
        complexityButtonGroup->addButton(advancedRadio, static_cast<int>(TemplateComplexity::Advanced));
        complexityButtonGroup->addButton(expertRadio, static_cast<int>(TemplateComplexity::Expert));

        complexityLayout->addWidget(basicRadio);
        complexityLayout->addWidget(standardRadio);
        complexityLayout->addWidget(advancedRadio);
        complexityLayout->addWidget(expertRadio);

        layout->addWidget(complexityGroup);

        // Connect signals
        connect(typeButtonGroup, &QButtonGroup::idClicked,
                this, [this](int id) {
                    // Update template type
                    Q_UNUSED(id)
                    emit completeChanged();
                });

        connect(complexityButtonGroup, &QButtonGroup::idClicked,
                this, [this](int id) {
                    // Update complexity
                    Q_UNUSED(id)
                    emit completeChanged();
                });
}

bool PluginTemplateGeneratorWizard::TemplateSelectionPage::isComplete() const {
    return true; // Always complete as we have defaults
}

// Interface Selection Page Implementation
PluginTemplateGeneratorWizard::InterfaceSelectionPage::InterfaceSelectionPage(QWidget* parent) : QWizardPage(parent) {
        setTitle("Interface Selection");
        setSubTitle("Select the plugin interfaces to implement.");

        auto layout = new QVBoxLayout(this);

        auto interfaceGroup = new QGroupBox("Available Interfaces", this);
        auto interfaceLayout = new QVBoxLayout(interfaceGroup);

        // Create checkboxes for different interfaces
        m_interfaceChecks["IPlugin"] = new QCheckBox("IPlugin - Base plugin interface (always included)", this);
        m_interfaceChecks["IPlugin"]->setChecked(true);
        m_interfaceChecks["IPlugin"]->setEnabled(false);

        m_interfaceChecks["IUIPlugin"] = new QCheckBox("IUIPlugin - User interface plugin interface", this);
        m_interfaceChecks["IServicePlugin"] = new QCheckBox("IServicePlugin - Background service interface", this);
        m_interfaceChecks["INetworkPlugin"] = new QCheckBox("INetworkPlugin - Network communication interface", this);
        m_interfaceChecks["IDataProviderPlugin"] = new QCheckBox("IDataProviderPlugin - Data processing interface", this);
        m_interfaceChecks["IScriptingPlugin"] = new QCheckBox("IScriptingPlugin - Scripting engine interface", this);

        for (auto it = m_interfaceChecks.begin(); it != m_interfaceChecks.end(); ++it) {
            interfaceLayout->addWidget(it.value());
            connect(it.value(), &QCheckBox::toggled, this, &InterfaceSelectionPage::completeChanged);
        }

        layout->addWidget(interfaceGroup);
        layout->addStretch();
}

bool PluginTemplateGeneratorWizard::InterfaceSelectionPage::isComplete() const {
    // At least IPlugin must be selected (which is always true)
    return true;
}

QStringList PluginTemplateGeneratorWizard::InterfaceSelectionPage::getSelectedInterfaces() const {
    QStringList interfaces;
    for (auto it = m_interfaceChecks.begin(); it != m_interfaceChecks.end(); ++it) {
        if (it.value()->isChecked()) {
            interfaces.append(it.key());
        }
    }
    return interfaces;
}

// Feature Selection Page Implementation
PluginTemplateGeneratorWizard::FeatureSelectionPage::FeatureSelectionPage(QWidget* parent) : QWizardPage(parent) {
        setTitle("Feature Selection");
        setSubTitle("Select additional features to include in your plugin.");

        auto layout = new QVBoxLayout(this);

        auto featureGroup = new QGroupBox("Available Features", this);
        auto featureLayout = new QVBoxLayout(featureGroup);

        // Create checkboxes for different features
        m_featureChecks["Configuration UI"] = new QCheckBox("Configuration UI - Auto-generated settings dialog", this);
        m_featureChecks["Logging Support"] = new QCheckBox("Logging Support - Integrated logging system", this);
        m_featureChecks["Internationalization"] = new QCheckBox("Internationalization - Multi-language support", this);
        m_featureChecks["Plugin Dependencies"] = new QCheckBox("Plugin Dependencies - Dependency management", this);
        m_featureChecks["Hot Reload"] = new QCheckBox("Hot Reload - Runtime plugin reloading", this);
        m_featureChecks["Performance Monitoring"] = new QCheckBox("Performance Monitoring - Built-in profiling", this);
        m_featureChecks["Error Handling"] = new QCheckBox("Error Handling - Comprehensive error management", this);
        m_featureChecks["Threading Support"] = new QCheckBox("Threading Support - Multi-threaded operations", this);

        // Set some defaults
        m_featureChecks["Logging Support"]->setChecked(true);
        m_featureChecks["Error Handling"]->setChecked(true);

        for (auto it = m_featureChecks.begin(); it != m_featureChecks.end(); ++it) {
            featureLayout->addWidget(it.value());
            connect(it.value(), &QCheckBox::toggled, this, &FeatureSelectionPage::completeChanged);
        }

        layout->addWidget(featureGroup);
        layout->addStretch();
}

bool PluginTemplateGeneratorWizard::FeatureSelectionPage::isComplete() const {
    return true; // Always complete
}

QStringList PluginTemplateGeneratorWizard::FeatureSelectionPage::getSelectedFeatures() const {
    QStringList features;
    for (auto it = m_featureChecks.begin(); it != m_featureChecks.end(); ++it) {
        if (it.value()->isChecked()) {
            features.append(it.key());
        }
    }
    return features;
}

// Output Configuration Page Implementation
PluginTemplateGeneratorWizard::OutputConfigurationPage::OutputConfigurationPage(QWidget* parent) : QWizardPage(parent) {
        setTitle("Output Configuration");
        setSubTitle("Configure the output directory and generation options.");

        auto layout = new QFormLayout(this);

        // Output directory
        auto dirLayout = new QHBoxLayout();
        m_outputDirEdit = new QLineEdit(this);
        m_outputDirEdit->setPlaceholderText("Select output directory...");
        m_outputDirEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/MyPlugin");

        auto browseButton = new QPushButton("Browse...", this);
        connect(browseButton, &QPushButton::clicked, this, [this]() {
            QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", m_outputDirEdit->text());
            if (!dir.isEmpty()) {
                m_outputDirEdit->setText(dir);
                emit completeChanged();
            }
        });

        dirLayout->addWidget(m_outputDirEdit);
        dirLayout->addWidget(browseButton);
        layout->addRow("Output Directory*:", dirLayout);

        // Generation options
        layout->addRow("", new QLabel("")); // Spacer

        m_generateTestsCheck = new QCheckBox("Generate unit tests", this);
        m_generateTestsCheck->setChecked(true);
        layout->addRow("", m_generateTestsCheck);

        m_generateDocsCheck = new QCheckBox("Generate documentation", this);
        m_generateDocsCheck->setChecked(true);
        layout->addRow("", m_generateDocsCheck);

        m_generateCMakeCheck = new QCheckBox("Generate CMake files", this);
        m_generateCMakeCheck->setChecked(true);
        layout->addRow("", m_generateCMakeCheck);

        m_generateExamplesCheck = new QCheckBox("Generate example code", this);
        m_generateExamplesCheck->setChecked(false);
        layout->addRow("", m_generateExamplesCheck);

        // Connect signals
        connect(m_outputDirEdit, &QLineEdit::textChanged, this, &OutputConfigurationPage::completeChanged);
        connect(m_generateTestsCheck, &QCheckBox::toggled, this, &OutputConfigurationPage::completeChanged);
        connect(m_generateDocsCheck, &QCheckBox::toggled, this, &OutputConfigurationPage::completeChanged);
        connect(m_generateCMakeCheck, &QCheckBox::toggled, this, &OutputConfigurationPage::completeChanged);
        connect(m_generateExamplesCheck, &QCheckBox::toggled, this, &OutputConfigurationPage::completeChanged);
    }

    bool isComplete() const override {
        return !m_outputDirEdit->text().trimmed().isEmpty();
    }

    QString getOutputDirectory() const {
        return m_outputDirEdit->text().trimmed();
    }

    bool getGenerateTests() const { return m_generateTestsCheck->isChecked(); }
    bool getGenerateDocs() const { return m_generateDocsCheck->isChecked(); }
    bool getGenerateCMake() const { return m_generateCMakeCheck->isChecked(); }
    bool getGenerateExamples() const { return m_generateExamplesCheck->isChecked(); }

private:
    QLineEdit* m_outputDirEdit;
    QCheckBox* m_generateTestsCheck;
    QCheckBox* m_generateDocsCheck;
    QCheckBox* m_generateCMakeCheck;
    QCheckBox* m_generateExamplesCheck;
};

// Preview Page Implementation
class PluginTemplateGeneratorWizard::PreviewPage : public QWizardPage {
    Q_OBJECT

public:
    explicit PreviewPage(QWidget* parent = nullptr) : QWizardPage(parent) {
        setTitle("Preview");
        setSubTitle("Review the files that will be generated.");

        auto layout = new QVBoxLayout(this);

        // File list
        auto fileGroup = new QGroupBox("Files to be generated:", this);
        auto fileLayout = new QVBoxLayout(fileGroup);

        m_fileList = new QListWidget(this);
        fileLayout->addWidget(m_fileList);

        layout->addWidget(fileGroup);

        // Preview area
        auto previewGroup = new QGroupBox("Code Preview:", this);
        auto previewLayout = new QVBoxLayout(previewGroup);

        m_previewEdit = new QPlainTextEdit(this);
        m_previewEdit->setReadOnly(true);
        m_previewEdit->setFont(QFont("Consolas", 10));
        previewLayout->addWidget(m_previewEdit);

        layout->addWidget(previewGroup);

        // Connect file list selection to preview
        connect(m_fileList, &QListWidget::currentTextChanged, this, &PreviewPage::updatePreview);
    }

    void initializePage() override {
        updateFileList();
        if (m_fileList->count() > 0) {
            m_fileList->setCurrentRow(0);
        }
    }

    bool isComplete() const override {
        return true;
    }

private slots:
    void updatePreview(const QString& fileName) {
        // Show preview of the selected file
        if (fileName.endsWith(".h")) {
            m_previewEdit->setPlainText("// Header file preview\n// This would show the actual generated header content");
        } else if (fileName.endsWith(".cpp")) {
            m_previewEdit->setPlainText("// Source file preview\n// This would show the actual generated source content");
        } else if (fileName.endsWith(".json")) {
            m_previewEdit->setPlainText("{\n  \"name\": \"Plugin metadata preview\",\n  \"description\": \"This would show the actual metadata\"\n}");
        } else {
            m_previewEdit->setPlainText("// Preview for " + fileName);
        }
    }

private:
    QListWidget* m_fileList;
    QPlainTextEdit* m_previewEdit;

    void updateFileList() {
        m_fileList->clear();

        // Add files that will be generated
        m_fileList->addItem("plugin_name.h");
        m_fileList->addItem("plugin_name.cpp");
        m_fileList->addItem("plugin_name.json");
        m_fileList->addItem("CMakeLists.txt");
        m_fileList->addItem("test_plugin_name.cpp");
        m_fileList->addItem("README.md");
    }
};

// Generation Page Implementation
class PluginTemplateGeneratorWizard::GenerationPage : public QWizardPage {
    Q_OBJECT

public:
    explicit GenerationPage(QWidget* parent = nullptr) : QWizardPage(parent) {
        setTitle("Generating Plugin Template");
        setSubTitle("Please wait while your plugin template is being generated...");

        auto layout = new QVBoxLayout(this);

        m_progressBar = new QProgressBar(this);
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
        layout->addWidget(m_progressBar);

        m_statusLabel = new QLabel("Initializing...", this);
        layout->addWidget(m_statusLabel);

        m_logEdit = new QTextEdit(this);
        m_logEdit->setMaximumHeight(200);
        m_logEdit->setReadOnly(true);
        layout->addWidget(m_logEdit);

        layout->addStretch();
    }

    void initializePage() override {
        // Start generation process
        m_progressBar->setValue(0);
        m_statusLabel->setText("Starting generation...");
        m_logEdit->clear();

        // This would trigger the actual generation
        QTimer::singleShot(100, this, [this]() {
            // Simulate generation process
            for (int i = 0; i <= 100; i += 10) {
                QTimer::singleShot(i * 50, this, [this, i]() {
                    m_progressBar->setValue(i);
                    m_statusLabel->setText(QString("Generating files... %1%").arg(i));
                    m_logEdit->append(QString("Step %1 completed").arg(i / 10));

                    if (i == 100) {
                        m_statusLabel->setText("Generation completed successfully!");
                        emit completeChanged();
                    }
                });
            }
        });
    }

    bool isComplete() const override {
        return m_progressBar->value() == 100;
    }

private:
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QTextEdit* m_logEdit;
};

#include "PluginTemplateWizard.moc"
