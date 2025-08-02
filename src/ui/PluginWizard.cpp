// PluginWizard.cpp - Implementation of Plugin Creation Wizard
#include "PluginWizard.h"
#include <QApplication>
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
#include <QTimer>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QUuid>
#include <QStandardPaths>
#include <QDebug>

PluginWizard::PluginWizard(QWidget* parent)
    : QWizard(parent)
{
    setWindowTitle("Plugin Creation Wizard");
    setWizardStyle(QWizard::ModernStyle);
    setOption(QWizard::HaveHelpButton, false);
    setOption(QWizard::HaveFinishButtonOnEarlyPages, false);
    
    setupPages();
    setupConnections();
    loadTemplates();
    
    resize(800, 600);
}

PluginWizard::~PluginWizard() = default;

PluginWizard::PluginType PluginWizard::selectedPluginType() const
{
    auto* typePage = qobject_cast<PluginTypeSelectionPage*>(page(TypeSelectionPage));
    return typePage ? typePage->getSelectedType() : UIPlugin;
}

QJsonObject PluginWizard::getPluginInfo() const
{
    auto* infoPage = qobject_cast<PluginInfoPage*>(page(InfoPage));
    return infoPage ? infoPage->getPluginInfo() : QJsonObject();
}

QStringList PluginWizard::getSelectedCapabilities() const
{
    auto* capPage = qobject_cast<PluginCapabilitiesPage*>(page(CapabilitiesPage));
    return capPage ? capPage->getSelectedCapabilities() : QStringList();
}

QStringList PluginWizard::getSelectedInterfaces() const
{
    auto* interfacePage = qobject_cast<PluginInterfacePage*>(page(InterfacePage));
    return interfacePage ? interfacePage->getSelectedInterfaces() : QStringList();
}

QJsonObject PluginWizard::getPluginConfiguration() const
{
    auto* configPage = qobject_cast<PluginConfigurationPage*>(page(ConfigurationPage));
    return configPage ? configPage->getConfiguration() : QJsonObject();
}

QString PluginWizard::getOutputDirectory() const
{
    QJsonObject info = getPluginInfo();
    return info["outputDirectory"].toString();
}

void PluginWizard::loadTemplates()
{
    // Create built-in templates
    auto* uiTemplate = new PluginTemplate("UI Plugin Template", this);
    uiTemplate->setDescription("Template for creating UI plugins with widgets and user interfaces");
    uiTemplate->setType(UIPlugin);
    uiTemplate->setRequiredCapabilities({"UI", "Configuration"});
    uiTemplate->setDefaultInterfaces({"IUIPlugin"});
    m_templates["UI Plugin Template"] = uiTemplate;
    
    auto* serviceTemplate = new PluginTemplate("Service Plugin Template", this);
    serviceTemplate->setDescription("Template for creating background service plugins");
    serviceTemplate->setType(ServicePlugin);
    serviceTemplate->setRequiredCapabilities({"Service", "Threading"});
    serviceTemplate->setDefaultInterfaces({"IServicePlugin"});
    m_templates["Service Plugin Template"] = serviceTemplate;
    
    auto* networkTemplate = new PluginTemplate("Network Plugin Template", this);
    networkTemplate->setDescription("Template for creating network communication plugins");
    networkTemplate->setType(NetworkPlugin);
    networkTemplate->setRequiredCapabilities({"Network", "Threading"});
    networkTemplate->setDefaultInterfaces({"INetworkPlugin"});
    m_templates["Network Plugin Template"] = networkTemplate;
    
    auto* dataTemplate = new PluginTemplate("Data Provider Template", this);
    dataTemplate->setDescription("Template for creating data processing and provider plugins");
    dataTemplate->setType(DataProviderPlugin);
    dataTemplate->setRequiredCapabilities({"Database", "FileSystem"});
    dataTemplate->setDefaultInterfaces({"IDataProviderPlugin"});
    m_templates["Data Provider Template"] = dataTemplate;
}

QStringList PluginWizard::availableTemplates() const
{
    return m_templates.keys();
}

PluginTemplate* PluginWizard::getTemplate(const QString& name) const
{
    return m_templates.value(name, nullptr);
}

void PluginWizard::generatePlugin()
{
    auto* genPage = qobject_cast<PluginGenerationPage*>(page(GenerationPage));
    if (genPage) {
        genPage->generatePlugin();
    }
}

void PluginWizard::onCurrentIdChanged(int id)
{
    Q_UNUSED(id)
    // Update wizard data when page changes
    m_wizardData["selectedType"] = static_cast<int>(selectedPluginType());
    m_wizardData["pluginInfo"] = getPluginInfo();
    m_wizardData["capabilities"] = QJsonArray::fromStringList(getSelectedCapabilities());
    m_wizardData["interfaces"] = QJsonArray::fromStringList(getSelectedInterfaces());
    m_wizardData["configuration"] = getPluginConfiguration();
}

void PluginWizard::onFinishClicked()
{
    generatePlugin();
}

void PluginWizard::setupPages()
{
    setPage(TypeSelectionPage, new PluginTypeSelectionPage(this));
    setPage(InfoPage, new PluginInfoPage(this));
    setPage(CapabilitiesPage, new PluginCapabilitiesPage(this));
    setPage(InterfacePage, new PluginInterfacePage(this));
    setPage(ConfigurationPage, new PluginConfigurationPage(this));
    setPage(GenerationPage, new PluginGenerationPage(this));
    
    setStartId(TypeSelectionPage);
}

void PluginWizard::setupConnections()
{
    connect(this, &QWizard::currentIdChanged, this, &PluginWizard::onCurrentIdChanged);
    connect(this, &QWizard::finished, this, &PluginWizard::onFinishClicked);
}

bool PluginWizard::validateCurrentPage()
{
    auto* currentPage = page(currentId());
    return currentPage ? currentPage->isComplete() : false;
}

// PluginTypeSelectionPage Implementation
PluginTypeSelectionPage::PluginTypeSelectionPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle("Select Plugin Type");
    setSubTitle("Choose the type of plugin you want to create and select a template.");
    
    setupUI();
}

void PluginTypeSelectionPage::initializePage()
{
    updateTemplateList();
    updateDescription();
}

bool PluginTypeSelectionPage::isComplete() const
{
    return m_typeCombo && m_typeCombo->currentIndex() >= 0;
}

int PluginTypeSelectionPage::nextId() const
{
    return PluginWizard::InfoPage;
}

PluginWizard::PluginType PluginTypeSelectionPage::getSelectedType() const
{
    if (!m_typeCombo) return PluginWizard::UIPlugin;
    
    QString typeText = m_typeCombo->currentText();
    if (typeText == "UI Plugin") return PluginWizard::UIPlugin;
    if (typeText == "Service Plugin") return PluginWizard::ServicePlugin;
    if (typeText == "Network Plugin") return PluginWizard::NetworkPlugin;
    if (typeText == "Data Provider Plugin") return PluginWizard::DataProviderPlugin;
    if (typeText == "Scripting Plugin") return PluginWizard::ScriptingPlugin;
    return PluginWizard::CustomPlugin;
}

void PluginTypeSelectionPage::onTypeSelectionChanged()
{
    updateTemplateList();
    updateDescription();
    emit completeChanged();
}

void PluginTypeSelectionPage::onTemplateSelectionChanged()
{
    updateDescription();
}

void PluginTypeSelectionPage::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    
    // Plugin type selection
    auto* typeGroup = new QGroupBox("Plugin Type");
    auto* typeLayout = new QFormLayout(typeGroup);
    
    m_typeCombo = new QComboBox;
    m_typeCombo->addItems({"UI Plugin", "Service Plugin", "Network Plugin", "Data Provider Plugin", "Scripting Plugin", "Custom Plugin"});
    typeLayout->addRow("Type:", m_typeCombo);
    
    layout->addWidget(typeGroup);
    
    // Template selection
    auto* templateGroup = new QGroupBox("Template");
    auto* templateLayout = new QVBoxLayout(templateGroup);
    
    m_templateList = new QListWidget;
    m_templateList->setMaximumHeight(150);
    templateLayout->addWidget(m_templateList);
    
    layout->addWidget(templateGroup);
    
    // Description
    auto* descGroup = new QGroupBox("Description");
    auto* descLayout = new QVBoxLayout(descGroup);
    
    m_descriptionText = new QTextEdit;
    m_descriptionText->setReadOnly(true);
    m_descriptionText->setMaximumHeight(100);
    descLayout->addWidget(m_descriptionText);
    
    layout->addWidget(descGroup);
    
    // Preview
    m_previewLabel = new QLabel("Preview will be shown here");
    m_previewLabel->setAlignment(Qt::AlignCenter);
    m_previewLabel->setStyleSheet("border: 1px solid #ccc; padding: 20px; background-color: #f9f9f9;");
    layout->addWidget(m_previewLabel);
    
    // Connect signals
    connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &PluginTypeSelectionPage::onTypeSelectionChanged);
    connect(m_templateList, &QListWidget::currentRowChanged, 
            this, &PluginTypeSelectionPage::onTemplateSelectionChanged);
}

void PluginTypeSelectionPage::updateTemplateList()
{
    if (!m_templateList) return;
    
    m_templateList->clear();
    
    auto* wizard = qobject_cast<PluginWizard*>(this->wizard());
    if (!wizard) return;
    
    QStringList templates = wizard->availableTemplates();
    PluginWizard::PluginType selectedType = getSelectedType();
    
    for (const QString& templateName : templates) {
        auto* template_ = wizard->getTemplate(templateName);
        if (template_ && template_->type() == selectedType) {
            m_templateList->addItem(templateName);
        }
    }
    
    if (m_templateList->count() > 0) {
        m_templateList->setCurrentRow(0);
    }
}

void PluginTypeSelectionPage::updateDescription()
{
    if (!m_descriptionText || !m_templateList) return;
    
    QString selectedTemplate = m_templateList->currentItem() ? 
                              m_templateList->currentItem()->text() : QString();
    
    if (selectedTemplate.isEmpty()) {
        m_descriptionText->setText("Select a template to see its description.");
        return;
    }
    
    auto* wizard = qobject_cast<PluginWizard*>(this->wizard());
    if (!wizard) return;
    
    auto* template_ = wizard->getTemplate(selectedTemplate);
    if (template_) {
        m_descriptionText->setText(template_->description());
    }
}

// PluginInfoPage Implementation
PluginInfoPage::PluginInfoPage(QWidget* parent)
    : QWizardPage(parent)
{
    setTitle("Plugin Information");
    setSubTitle("Enter basic information about your plugin.");
    
    setupUI();
}

void PluginInfoPage::initializePage()
{
    updateCompleteStatus();
}

bool PluginInfoPage::isComplete() const
{
    return validateFields();
}

bool PluginInfoPage::validatePage()
{
    return validateFields();
}

QJsonObject PluginInfoPage::getPluginInfo() const
{
    QJsonObject info;
    if (m_nameEdit) info["name"] = m_nameEdit->text();
    if (m_classNameEdit) info["className"] = m_classNameEdit->text();
    if (m_descriptionEdit) info["description"] = m_descriptionEdit->toPlainText();
    if (m_authorEdit) info["author"] = m_authorEdit->text();
    if (m_versionEdit) info["version"] = m_versionEdit->text();
    if (m_homepageEdit) info["homepage"] = m_homepageEdit->text();
    if (m_licenseCombo) info["license"] = m_licenseCombo->currentText();
    if (m_uuidEdit) info["uuid"] = m_uuidEdit->text();
    if (m_outputDirEdit) info["outputDirectory"] = m_outputDirEdit->text();
    return info;
}

void PluginInfoPage::onFieldChanged()
{
    updateCompleteStatus();
}

void PluginInfoPage::onBrowseOutputDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Output Directory", 
                                                   QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    if (!dir.isEmpty() && m_outputDirEdit) {
        m_outputDirEdit->setText(dir);
        updateCompleteStatus();
    }
}

void PluginInfoPage::onGenerateUuid()
{
    if (m_uuidEdit) {
        m_uuidEdit->setText(QUuid::createUuid().toString());
        updateCompleteStatus();
    }
}

void PluginInfoPage::setupUI()
{
    auto* layout = new QFormLayout(this);
    
    m_nameEdit = new QLineEdit;
    m_nameEdit->setPlaceholderText("My Awesome Plugin");
    layout->addRow("Plugin Name:", m_nameEdit);
    
    m_classNameEdit = new QLineEdit;
    m_classNameEdit->setPlaceholderText("MyAwesomePlugin");
    layout->addRow("Class Name:", m_classNameEdit);
    
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setPlaceholderText("Enter a description of your plugin...");
    m_descriptionEdit->setMaximumHeight(80);
    layout->addRow("Description:", m_descriptionEdit);
    
    m_authorEdit = new QLineEdit;
    m_authorEdit->setPlaceholderText("Your Name");
    layout->addRow("Author:", m_authorEdit);
    
    m_versionEdit = new QLineEdit;
    m_versionEdit->setText("1.0.0");
    layout->addRow("Version:", m_versionEdit);
    
    m_homepageEdit = new QLineEdit;
    m_homepageEdit->setPlaceholderText("https://example.com");
    layout->addRow("Homepage:", m_homepageEdit);
    
    m_licenseCombo = new QComboBox;
    m_licenseCombo->addItems({"MIT", "GPL-3.0", "Apache-2.0", "BSD-3-Clause", "Custom"});
    layout->addRow("License:", m_licenseCombo);
    
    auto* uuidLayout = new QHBoxLayout;
    m_uuidEdit = new QLineEdit;
    m_uuidEdit->setReadOnly(true);
    m_generateUuidBtn = new QPushButton("Generate");
    uuidLayout->addWidget(m_uuidEdit);
    uuidLayout->addWidget(m_generateUuidBtn);
    layout->addRow("UUID:", uuidLayout);
    
    auto* outputLayout = new QHBoxLayout;
    m_outputDirEdit = new QLineEdit;
    m_outputDirEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/MyPlugin");
    m_browseBtn = new QPushButton("Browse...");
    outputLayout->addWidget(m_outputDirEdit);
    outputLayout->addWidget(m_browseBtn);
    layout->addRow("Output Directory:", outputLayout);
    
    // Connect signals
    connect(m_nameEdit, &QLineEdit::textChanged, this, &PluginInfoPage::onFieldChanged);
    connect(m_classNameEdit, &QLineEdit::textChanged, this, &PluginInfoPage::onFieldChanged);
    connect(m_descriptionEdit, &QTextEdit::textChanged, this, &PluginInfoPage::onFieldChanged);
    connect(m_authorEdit, &QLineEdit::textChanged, this, &PluginInfoPage::onFieldChanged);
    connect(m_versionEdit, &QLineEdit::textChanged, this, &PluginInfoPage::onFieldChanged);
    connect(m_outputDirEdit, &QLineEdit::textChanged, this, &PluginInfoPage::onFieldChanged);
    connect(m_browseBtn, &QPushButton::clicked, this, &PluginInfoPage::onBrowseOutputDirectory);
    connect(m_generateUuidBtn, &QPushButton::clicked, this, &PluginInfoPage::onGenerateUuid);
    
    // Auto-generate UUID
    onGenerateUuid();
    
    // Auto-update class name based on plugin name
    connect(m_nameEdit, &QLineEdit::textChanged, [this](const QString& text) {
        if (m_classNameEdit) {
            QString className = text;
            className.remove(QRegularExpression("[^a-zA-Z0-9]"));
            if (!className.isEmpty()) {
                className[0] = className[0].toUpper();
            }
            m_classNameEdit->setText(className);
        }
    });
}

void PluginInfoPage::updateCompleteStatus()
{
    emit completeChanged();
}

bool PluginInfoPage::validateFields() const
{
    return m_nameEdit && !m_nameEdit->text().isEmpty() &&
           m_classNameEdit && !m_classNameEdit->text().isEmpty() &&
           m_authorEdit && !m_authorEdit->text().isEmpty() &&
           m_versionEdit && !m_versionEdit->text().isEmpty() &&
           m_outputDirEdit && !m_outputDirEdit->text().isEmpty();
}

// Placeholder implementations for other pages
PluginCapabilitiesPage::PluginCapabilitiesPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle("Plugin Capabilities");
    setSubTitle("Select the capabilities your plugin will use.");
    
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("Capabilities selection will be implemented here.");
    layout->addWidget(label);
}

void PluginCapabilitiesPage::initializePage() {}
bool PluginCapabilitiesPage::isComplete() const { return true; }
QStringList PluginCapabilitiesPage::getSelectedCapabilities() const { return {"UI", "Configuration"}; }
QJsonObject PluginCapabilitiesPage::getCapabilityConfiguration() const { return QJsonObject(); }
void PluginCapabilitiesPage::onCapabilityToggled() {}
void PluginCapabilitiesPage::onCapabilityConfigChanged() {}
void PluginCapabilitiesPage::setupUI() {}
void PluginCapabilitiesPage::updateCapabilityConfig() {}

PluginInterfacePage::PluginInterfacePage(QWidget* parent) : QWizardPage(parent)
{
    setTitle("Plugin Interfaces");
    setSubTitle("Select the interfaces your plugin will implement.");
    
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("Interface selection will be implemented here.");
    layout->addWidget(label);
}

void PluginInterfacePage::initializePage() {}
bool PluginInterfacePage::isComplete() const { return true; }
QStringList PluginInterfacePage::getSelectedInterfaces() const { return {"IPlugin"}; }
QJsonObject PluginInterfacePage::getInterfaceConfiguration() const { return QJsonObject(); }
void PluginInterfacePage::onInterfaceToggled() {}
void PluginInterfacePage::onMethodToggled() {}
void PluginInterfacePage::onAddCustomMethod() {}
void PluginInterfacePage::onRemoveCustomMethod() {}
void PluginInterfacePage::setupUI() {}
void PluginInterfacePage::updateInterfaceMethods() {}
void PluginInterfacePage::addCustomMethodWidget() {}

PluginConfigurationPage::PluginConfigurationPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle("Plugin Configuration");
    setSubTitle("Define configuration options for your plugin.");
    
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("Configuration setup will be implemented here.");
    layout->addWidget(label);
}

void PluginConfigurationPage::initializePage() {}
bool PluginConfigurationPage::isComplete() const { return true; }
QJsonObject PluginConfigurationPage::getConfiguration() const { return QJsonObject(); }
void PluginConfigurationPage::onAddConfigItem() {}
void PluginConfigurationPage::onRemoveConfigItem() {}
void PluginConfigurationPage::onConfigItemChanged() {}
void PluginConfigurationPage::onImportConfig() {}
void PluginConfigurationPage::onExportConfig() {}
void PluginConfigurationPage::setupUI() {}
void PluginConfigurationPage::addConfigItemWidget(const QString&, const QString&, const QVariant&) {}
void PluginConfigurationPage::updateConfigPreview() {}

PluginGenerationPage::PluginGenerationPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle("Generate Plugin");
    setSubTitle("Review your settings and generate the plugin project.");
    
    setupUI();
}

void PluginGenerationPage::initializePage()
{
    updateSummary();
}

bool PluginGenerationPage::isComplete() const
{
    return m_generationComplete;
}

void PluginGenerationPage::generatePlugin()
{
    if (!m_progressBar || !m_statusLabel) return;
    
    m_progressBar->setVisible(true);
    m_statusLabel->setText("Generating plugin project...");
    
    // Simulate plugin generation
    QTimer::singleShot(1000, this, &PluginGenerationPage::onGenerationFinished);
}

void PluginGenerationPage::onGenerationProgress(int percentage)
{
    if (m_progressBar) {
        m_progressBar->setValue(percentage);
    }
}

void PluginGenerationPage::onGenerationFinished()
{
    m_generationComplete = true;
    if (m_progressBar) {
        m_progressBar->setValue(100);
        m_progressBar->setVisible(false);
    }
    if (m_statusLabel) {
        m_statusLabel->setText("Plugin generated successfully!");
    }
    if (m_openDirBtn) {
        m_openDirBtn->setVisible(true);
    }
    if (m_openIDEBtn) {
        m_openIDEBtn->setVisible(true);
    }
    emit completeChanged();
}

void PluginGenerationPage::onGenerationError(const QString& error)
{
    if (m_statusLabel) {
        m_statusLabel->setText("Error: " + error);
    }
    if (m_progressBar) {
        m_progressBar->setVisible(false);
    }
}

void PluginGenerationPage::onOpenOutputDirectory()
{
    auto* wizard = qobject_cast<PluginWizard*>(this->wizard());
    if (wizard) {
        QString outputDir = wizard->getOutputDirectory();
        QDesktopServices::openUrl(QUrl::fromLocalFile(outputDir));
    }
}

void PluginGenerationPage::onOpenInIDE()
{
    QMessageBox::information(this, "Open in IDE", "IDE integration will be implemented here.");
}

void PluginGenerationPage::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    
    // Summary
    auto* summaryGroup = new QGroupBox("Summary");
    auto* summaryLayout = new QVBoxLayout(summaryGroup);
    
    m_summaryText = new QTextEdit;
    m_summaryText->setReadOnly(true);
    m_summaryText->setMaximumHeight(200);
    summaryLayout->addWidget(m_summaryText);
    
    layout->addWidget(summaryGroup);
    
    // Progress
    auto* progressGroup = new QGroupBox("Generation Progress");
    auto* progressLayout = new QVBoxLayout(progressGroup);
    
    m_statusLabel = new QLabel("Ready to generate plugin");
    progressLayout->addWidget(m_statusLabel);
    
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    progressLayout->addWidget(m_progressBar);
    
    layout->addWidget(progressGroup);
    
    // Actions
    auto* actionsGroup = new QGroupBox("Actions");
    auto* actionsLayout = new QHBoxLayout(actionsGroup);
    
    m_openDirBtn = new QPushButton("Open Output Directory");
    m_openDirBtn->setVisible(false);
    actionsLayout->addWidget(m_openDirBtn);
    
    m_openIDEBtn = new QPushButton("Open in IDE");
    m_openIDEBtn->setVisible(false);
    actionsLayout->addWidget(m_openIDEBtn);
    
    actionsLayout->addStretch();
    
    layout->addWidget(actionsGroup);
    
    // Connect signals
    connect(m_openDirBtn, &QPushButton::clicked, this, &PluginGenerationPage::onOpenOutputDirectory);
    connect(m_openIDEBtn, &QPushButton::clicked, this, &PluginGenerationPage::onOpenInIDE);
}

void PluginGenerationPage::updateSummary()
{
    if (!m_summaryText) return;
    
    auto* wizard = qobject_cast<PluginWizard*>(this->wizard());
    if (!wizard) return;
    
    QJsonObject info = wizard->getPluginInfo();
    QStringList capabilities = wizard->getSelectedCapabilities();
    QStringList interfaces = wizard->getSelectedInterfaces();
    
    QString summary;
    summary += "Plugin Name: " + info["name"].toString() + "\n";
    summary += "Class Name: " + info["className"].toString() + "\n";
    summary += "Author: " + info["author"].toString() + "\n";
    summary += "Version: " + info["version"].toString() + "\n";
    summary += "License: " + info["license"].toString() + "\n";
    summary += "Output Directory: " + info["outputDirectory"].toString() + "\n\n";
    summary += "Capabilities: " + capabilities.join(", ") + "\n";
    summary += "Interfaces: " + interfaces.join(", ") + "\n";
    
    m_summaryText->setText(summary);
}

// PluginTemplate Implementation
PluginTemplate::PluginTemplate(const QString& name, QObject* parent)
    : QObject(parent), m_name(name), m_type(PluginWizard::UIPlugin)
{
}

void PluginTemplate::addTemplateFile(const QString& relativePath, const QString& content)
{
    m_templateFiles[relativePath] = content;
}

QStringList PluginTemplate::templateFiles() const
{
    return m_templateFiles.keys();
}

QString PluginTemplate::getTemplateFileContent(const QString& relativePath) const
{
    return m_templateFiles.value(relativePath);
}

bool PluginTemplate::generatePlugin(const QJsonObject& wizardData, const QString& outputDir)
{
    qDebug() << "Generating plugin with template:" << m_name;
    qDebug() << "Output directory:" << outputDir;
    qDebug() << "Wizard data:" << wizardData;
    
    // TODO: Implement actual plugin generation
    emit generationFinished();
    return true;
}

QString PluginTemplate::processTemplate(const QString& templateContent, const QJsonObject& data)
{
    QString result = templateContent;
    
    // Simple template variable replacement
    QJsonObject info = data["pluginInfo"].toObject();
    result.replace("{{PLUGIN_NAME}}", info["name"].toString());
    result.replace("{{CLASS_NAME}}", info["className"].toString());
    result.replace("{{AUTHOR}}", info["author"].toString());
    result.replace("{{VERSION}}", info["version"].toString());
    result.replace("{{DESCRIPTION}}", info["description"].toString());
    result.replace("{{UUID}}", info["uuid"].toString());
    
    return result;
}

QString PluginTemplate::generateClassName(const QString& pluginName)
{
    QString className = pluginName;
    className.remove(QRegularExpression("[^a-zA-Z0-9]"));
    if (!className.isEmpty()) {
        className[0] = className[0].toUpper();
    }
    return className;
}

QString PluginTemplate::generateFileName(const QString& className, const QString& extension)
{
    return className + "." + extension;
}

bool PluginTemplate::createDirectoryStructure(const QString& outputDir)
{
    QDir dir;
    return dir.mkpath(outputDir);
}

bool PluginTemplate::writeFile(const QString& filePath, const QString& content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << content;
    return true;
}

// PluginGenerator Implementation
PluginGenerator::PluginGenerator(QObject* parent)
    : QObject(parent)
    , m_template(nullptr)
{
}

bool PluginGenerator::generatePlugin(const QJsonObject& wizardData, PluginTemplate* pluginTemplate, const QString& outputDir)
{
    m_wizardData = wizardData;
    m_template = pluginTemplate;
    m_outputDir = outputDir;
    m_pluginDir = QDir(outputDir).absoluteFilePath(wizardData.value("name").toString());
    m_generatedFiles.clear();

    // Start generation process
    QTimer::singleShot(0, this, &PluginGenerator::performGeneration);

    return true;
}

void PluginGenerator::performGeneration()
{
    emit progress(0);

    if (!validateInput()) {
        emit error("Invalid input data");
        return;
    }

    emit progress(10);

    if (!createProjectStructure()) {
        emit error("Failed to create project structure");
        return;
    }

    emit progress(30);

    if (!generateSourceFiles()) {
        emit error("Failed to generate source files");
        return;
    }

    emit progress(50);

    if (!generateConfigFiles()) {
        emit error("Failed to generate configuration files");
        return;
    }

    emit progress(70);

    if (!generateDocumentation()) {
        emit error("Failed to generate documentation");
        return;
    }

    emit progress(90);

    if (!generateBuildFiles()) {
        emit error("Failed to generate build files");
        return;
    }

    emit progress(100);
    emit finished();
}

bool PluginGenerator::validateInput()
{
    if (m_wizardData.isEmpty()) {
        return false;
    }

    if (!m_template) {
        return false;
    }

    QString pluginName = m_wizardData.value("name").toString();
    if (pluginName.isEmpty()) {
        return false;
    }

    return true;
}

bool PluginGenerator::createProjectStructure()
{
    QDir dir;

    // Create main plugin directory
    if (!dir.mkpath(m_pluginDir)) {
        return false;
    }

    // Create subdirectories
    QStringList subdirs = {"src", "include", "resources", "docs", "tests"};
    for (const QString& subdir : subdirs) {
        QString subdirPath = QDir(m_pluginDir).absoluteFilePath(subdir);
        if (!dir.mkpath(subdirPath)) {
            return false;
        }
    }

    return true;
}

bool PluginGenerator::writeFile(const QString& filePath, const QString& content)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << content;
    return true;
}

bool PluginGenerator::generateSourceFiles()
{
    if (!m_template) {
        return false;
    }

    QString pluginName = m_wizardData.value("name").toString();
    QString className = m_wizardData.value("className").toString();

    // Generate header file using template
    QString headerContent = m_template->getTemplateFileContent("header.h");
    if (headerContent.isEmpty()) {
        // Fallback to basic header template
        headerContent = QString(
            "#pragma once\n\n"
            "#include <QObject>\n"
            "#include \"PluginInterface.h\"\n\n"
            "class %1 : public QObject, public PluginInterface\n"
            "{\n"
            "    Q_OBJECT\n"
            "    Q_PLUGIN_METADATA(IID \"com.example.PluginInterface\")\n"
            "    Q_INTERFACES(PluginInterface)\n\n"
            "public:\n"
            "    explicit %1(QObject* parent = nullptr);\n"
            "    ~%1() override;\n\n"
            "    // PluginInterface implementation\n"
            "    QString name() const override;\n"
            "    QString version() const override;\n"
            "    QString description() const override;\n"
            "    bool initialize() override;\n"
            "    void shutdown() override;\n"
            "};\n"
        ).arg(className);
    }

    QString headerPath = QDir(m_pluginDir).absoluteFilePath(QString("include/%1.h").arg(className));
    if (!writeFile(headerPath, headerContent)) {
        return false;
    }
    m_generatedFiles.append(headerPath);

    // Generate source file using template
    QString sourceContent = m_template->getTemplateFileContent("source.cpp");
    if (sourceContent.isEmpty()) {
        // Fallback to basic source template
        sourceContent = QString(
            "#include \"%1.h\"\n\n"
            "%1::%1(QObject* parent)\n"
            "    : QObject(parent)\n"
            "{\n"
            "}\n\n"
            "%1::~%1() = default;\n\n"
            "QString %1::name() const\n"
            "{\n"
            "    return \"%2\";\n"
            "}\n\n"
            "QString %1::version() const\n"
            "{\n"
            "    return \"1.0.0\";\n"
            "}\n\n"
            "QString %1::description() const\n"
            "{\n"
            "    return \"%3\";\n"
            "}\n\n"
            "bool %1::initialize()\n"
            "{\n"
            "    // TODO: Initialize plugin\n"
            "    return true;\n"
            "}\n\n"
            "void %1::shutdown()\n"
            "{\n"
            "    // TODO: Cleanup plugin\n"
            "}\n"
        ).arg(className, pluginName, m_wizardData.value("description").toString());
    }

    QString sourcePath = QDir(m_pluginDir).absoluteFilePath(QString("src/%1.cpp").arg(className));
    if (!writeFile(sourcePath, sourceContent)) {
        return false;
    }
    m_generatedFiles.append(sourcePath);

    return true;
}

bool PluginGenerator::generateConfigFiles()
{
    QString pluginName = m_wizardData.value("name").toString();

    // Generate plugin.json
    QJsonObject pluginJson;
    pluginJson["name"] = pluginName;
    pluginJson["version"] = m_wizardData.value("version").toString("1.0.0");
    pluginJson["author"] = m_wizardData.value("author").toString();
    pluginJson["description"] = m_wizardData.value("description").toString();
    pluginJson["type"] = m_wizardData.value("type").toString();

    QString configPath = QDir(m_pluginDir).absoluteFilePath("plugin.json");
    if (!writeFile(configPath, QJsonDocument(pluginJson).toJson())) {
        return false;
    }
    m_generatedFiles.append(configPath);

    return true;
}

bool PluginGenerator::generateDocumentation()
{
    QString pluginName = m_wizardData.value("name").toString();
    QString description = m_wizardData.value("description").toString();

    // Generate README.md
    QString readmeContent = QString(
        "# %1\n\n"
        "%2\n\n"
        "## Installation\n\n"
        "1. Copy the plugin files to the plugins directory\n"
        "2. Restart the application\n"
        "3. Enable the plugin in the plugin manager\n\n"
        "## Usage\n\n"
        "TODO: Add usage instructions\n\n"
        "## License\n\n"
        "TODO: Add license information\n"
    ).arg(pluginName, description);

    QString readmePath = QDir(m_pluginDir).absoluteFilePath("README.md");
    if (!writeFile(readmePath, readmeContent)) {
        return false;
    }
    m_generatedFiles.append(readmePath);

    return true;
}

bool PluginGenerator::generateBuildFiles()
{
    QString pluginName = m_wizardData.value("name").toString();
    QString className = m_wizardData.value("className").toString();

    // Generate CMakeLists.txt
    QString cmakeContent = QString(
        "cmake_minimum_required(VERSION 3.16)\n"
        "project(%1)\n\n"
        "set(CMAKE_CXX_STANDARD 17)\n"
        "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n"
        "find_package(Qt6 REQUIRED COMPONENTS Core Widgets)\n\n"
        "set(SOURCES\n"
        "    src/%2.cpp\n"
        ")\n\n"
        "set(HEADERS\n"
        "    include/%2.h\n"
        ")\n\n"
        "add_library(%1 SHARED ${SOURCES} ${HEADERS})\n\n"
        "target_link_libraries(%1 Qt6::Core Qt6::Widgets)\n\n"
        "target_include_directories(%1 PRIVATE include)\n"
    ).arg(pluginName, className);

    QString cmakePath = QDir(m_pluginDir).absoluteFilePath("CMakeLists.txt");
    if (!writeFile(cmakePath, cmakeContent)) {
        return false;
    }
    m_generatedFiles.append(cmakePath);

    return true;
}


