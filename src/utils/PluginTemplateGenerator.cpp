// PluginTemplateGenerator.cpp - Implementation of Plugin Template Generation System
#include "PluginTemplateGenerator.h"
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QCryptographicHash>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(templateGenerator, "plugin.template.generator")

// PluginTemplateEngine Implementation
PluginTemplateEngine::PluginTemplateEngine(QObject* parent)
    : QObject(parent)
    , m_codeGenerator(std::make_unique<CodeGenerator>(this))
{
    // Connect code generator signals
    connect(m_codeGenerator.get(), &CodeGenerator::fileGenerated,
            this, &PluginTemplateEngine::onFileGenerated);
    connect(m_codeGenerator.get(), &CodeGenerator::generationProgress,
            this, [this](int percentage) {
                emit generationProgress(percentage, "");
            });
    
    // Load built-in templates
    loadBuiltInTemplates();
}

PluginTemplateEngine::~PluginTemplateEngine() = default;

bool PluginTemplateEngine::loadTemplate(const QString& templatePath) {
    QFileInfo fileInfo(templatePath);
    if (!fileInfo.exists() || !fileInfo.isReadable()) {
        qCWarning(templateGenerator) << "Template file not found or not readable:" << templatePath;
        return false;
    }
    
    QFile file(templatePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to open template file:" << templatePath;
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        qCWarning(templateGenerator) << "Invalid template format:" << templatePath;
        return false;
    }
    
    QJsonObject templateObj = doc.object();
    TemplateInfo info;
    info.name = templateObj["name"].toString();
    info.description = templateObj["description"].toString();
    info.filePath = templatePath;
    info.type = static_cast<PluginTemplateType>(templateObj["type"].toInt());
    info.complexity = static_cast<TemplateComplexity>(templateObj["complexity"].toInt());
    info.metadata = templateObj;
    
    // Extract required variables
    QJsonArray varsArray = templateObj["requiredVariables"].toArray();
    for (const auto& var : varsArray) {
        info.requiredVariables.append(var.toString());
    }
    
    m_templates[info.name] = info;
    qCInfo(templateGenerator) << "Loaded template:" << info.name;
    return true;
}

bool PluginTemplateEngine::loadTemplateFromResource(const QString& resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to open resource template:" << resourcePath;
        return false;
    }
    
    // Create temporary file and load it
    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                      + "/plugin_template_" + QUuid::createUuid().toString(QUuid::WithoutBraces) + ".json";
    
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    tempFile.write(file.readAll());
    tempFile.close();
    
    bool result = loadTemplate(tempPath);
    QFile::remove(tempPath);
    return result;
}

QStringList PluginTemplateEngine::availableTemplates() const {
    return m_templates.keys();
}

QString PluginTemplateEngine::templateDescription(const QString& templateName) const {
    auto it = m_templates.find(templateName);
    return (it != m_templates.end()) ? it->description : QString();
}

void PluginTemplateEngine::setVariable(const QString& name, const QString& value) {
    m_variables[name] = value;
}

void PluginTemplateEngine::setVariables(const QMap<QString, QString>& variables) {
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        m_variables[it.key()] = it.value();
    }
}

QString PluginTemplateEngine::getVariable(const QString& name) const {
    return m_variables.value(name);
}

QStringList PluginTemplateEngine::getRequiredVariables() const {
    QStringList required;
    for (const auto& templateInfo : m_templates) {
        required.append(templateInfo.requiredVariables);
    }
    required.removeDuplicates();
    return required;
}

QStringList PluginTemplateEngine::getAllVariables() const {
    return m_variables.keys();
}

QString PluginTemplateEngine::processTemplate(const QString& templateContent) const {
    QString result = templateContent;
    
    // Replace variables
    result = replaceVariables(result);
    
    // Process conditionals
    result = processConditionals(result);
    
    // Process loops
    result = processLoops(result);
    
    return result;
}

QString PluginTemplateEngine::processTemplateFile(const QString& templatePath) const {
    QFile file(templatePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to open template file:" << templatePath;
        return QString();
    }
    
    QTextStream stream(&file);
    QString content = stream.readAll();
    return processTemplate(content);
}

bool PluginTemplateEngine::generateFromTemplate(const QString& templateName, const CodeGenerationOptions& options) {
    auto it = m_templates.find(templateName);
    if (it == m_templates.end()) {
        emit errorOccurred(QString("Template not found: %1").arg(templateName));
        return false;
    }
    
    // Validate options
    QStringList errors = validateOptions(options);
    if (!errors.isEmpty()) {
        emit errorOccurred(QString("Validation errors: %1").arg(errors.join(", ")));
        return false;
    }
    
    // Set up variables from options
    setupVariablesFromOptions(options);
    
    // Create output directory
    QDir outputDir(options.outputDirectory);
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        emit errorOccurred(QString("Failed to create output directory: %1").arg(options.outputDirectory));
        return false;
    }
    
    emit generationProgress(10, "Setting up generation...");
    
    // Generate files based on template type
    bool success = true;
    
    try {
        // Generate header file
        emit generationProgress(20, "Generating header file...");
        success &= m_codeGenerator->generateHeaderFile(options, 
            outputDir.filePath(formatFileName(options.pluginName) + ".h"));
        
        // Generate source file
        emit generationProgress(40, "Generating source file...");
        success &= m_codeGenerator->generateSourceFile(options,
            outputDir.filePath(formatFileName(options.pluginName) + ".cpp"));
        
        // Generate metadata file
        emit generationProgress(60, "Generating metadata file...");
        success &= m_codeGenerator->generateMetadataFile(options,
            outputDir.filePath(formatFileName(options.pluginName) + ".json"));
        
        // Generate CMake file if requested
        if (options.generateCMakeFiles) {
            emit generationProgress(70, "Generating CMake file...");
            success &= m_codeGenerator->generateCMakeFile(options,
                outputDir.filePath("CMakeLists.txt"));
        }
        
        // Generate test file if requested
        if (options.generateTests) {
            emit generationProgress(80, "Generating test file...");
            success &= m_codeGenerator->generateTestFile(options,
                outputDir.filePath("test_" + formatFileName(options.pluginName) + ".cpp"));
        }
        
        // Generate documentation if requested
        if (options.generateDocumentation) {
            emit generationProgress(90, "Generating documentation...");
            success &= m_codeGenerator->generateDocumentationFile(options,
                outputDir.filePath("README.md"));
        }
        
        emit generationProgress(100, "Generation completed");
        emit generationCompleted(success, options.outputDirectory);
        
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Generation failed: %1").arg(e.what()));
        success = false;
    }
    
    return success;
}

bool PluginTemplateEngine::validateTemplate(const QString& templateContent) const {
    // Basic validation - check for required template markers
    QStringList requiredMarkers = {
        "{{PLUGIN_NAME}}",
        "{{PLUGIN_CLASS}}",
        "{{PLUGIN_DESCRIPTION}}"
    };
    
    for (const QString& marker : requiredMarkers) {
        if (!templateContent.contains(marker)) {
            return false;
        }
    }
    
    return true;
}

QStringList PluginTemplateEngine::validateOptions(const CodeGenerationOptions& options) const {
    QStringList errors;
    
    if (options.pluginName.isEmpty()) {
        errors << "Plugin name is required";
    }
    
    if (options.pluginDescription.isEmpty()) {
        errors << "Plugin description is required";
    }
    
    if (options.authorName.isEmpty()) {
        errors << "Author name is required";
    }
    
    if (options.outputDirectory.isEmpty()) {
        errors << "Output directory is required";
    }
    
    // Validate plugin name format
    QRegularExpression nameRegex("^[A-Za-z][A-Za-z0-9_]*$");
    if (!nameRegex.match(options.pluginName).hasMatch()) {
        errors << "Plugin name must start with a letter and contain only letters, numbers, and underscores";
    }
    
    return errors;
}

void PluginTemplateEngine::onFileGenerated(const QString& filePath) {
    qCInfo(templateGenerator) << "Generated file:" << filePath;
}

QString PluginTemplateEngine::replaceVariables(const QString& content) const {
    QString result = content;
    
    // Replace all variables in the format {{VARIABLE_NAME}}
    for (auto it = m_variables.begin(); it != m_variables.end(); ++it) {
        QString placeholder = QString("{{%1}}").arg(it.key());
        result.replace(placeholder, it.value());
    }
    
    return result;
}

QString PluginTemplateEngine::processConditionals(const QString& content) const {
    QString result = content;
    
    // Process conditionals in the format {{#if CONDITION}} ... {{/if}}
    QRegularExpression ifRegex(R"(\{\{#if\s+(\w+)\}\}(.*?)\{\{/if\}\})", 
                              QRegularExpression::DotMatchesEverythingOption);
    
    QRegularExpressionMatchIterator it = ifRegex.globalMatch(result);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString condition = match.captured(1);
        QString ifContent = match.captured(2);
        
        // Check if condition variable exists and is true
        bool conditionMet = m_variables.contains(condition) && 
                           (m_variables[condition].toLower() == "true" || 
                            m_variables[condition] == "1");
        
        result.replace(match.captured(0), conditionMet ? ifContent : "");
    }
    
    return result;
}

QString PluginTemplateEngine::processLoops(const QString& content) const {
    QString result = content;
    
    // Process loops in the format {{#each ARRAY}} ... {{/each}}
    QRegularExpression eachRegex(R"(\{\{#each\s+(\w+)\}\}(.*?)\{\{/each\}\})", 
                                QRegularExpression::DotMatchesEverythingOption);
    
    QRegularExpressionMatchIterator it = eachRegex.globalMatch(result);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString arrayName = match.captured(1);
        QString loopContent = match.captured(2);
        
        // For now, implement basic array processing
        // This could be extended to handle JSON arrays
        QString replacement;
        if (m_variables.contains(arrayName)) {
            QStringList items = m_variables[arrayName].split(",");
            for (const QString& item : items) {
                QString itemContent = loopContent;
                itemContent.replace("{{this}}", item.trimmed());
                replacement += itemContent;
            }
        }
        
        result.replace(match.captured(0), replacement);
    }
    
    return result;
}

void PluginTemplateEngine::setupVariablesFromOptions(const CodeGenerationOptions& options) {
    m_variables.clear();
    
    // Basic information
    m_variables["PLUGIN_NAME"] = options.pluginName;
    m_variables["PLUGIN_CLASS"] = formatClassName(options.pluginName);
    m_variables["PLUGIN_DESCRIPTION"] = options.pluginDescription;
    m_variables["AUTHOR_NAME"] = options.authorName;
    m_variables["AUTHOR_EMAIL"] = options.authorEmail;
    m_variables["ORGANIZATION_NAME"] = options.organizationName;
    m_variables["PLUGIN_VERSION"] = options.pluginVersion;
    m_variables["QT_VERSION"] = options.qtVersion;
    m_variables["CPP_STANDARD"] = options.cppStandard;
    
    // Template type and complexity
    m_variables["TEMPLATE_TYPE"] = QString::number(static_cast<int>(options.templateType));
    m_variables["COMPLEXITY"] = QString::number(static_cast<int>(options.complexity));
    
    // Features and interfaces
    m_variables["INTERFACES"] = options.interfaces.join(",");
    m_variables["FEATURES"] = options.features.join(",");
    
    // Generation options
    m_variables["GENERATE_TESTS"] = options.generateTests ? "true" : "false";
    m_variables["GENERATE_DOCS"] = options.generateDocumentation ? "true" : "false";
    m_variables["GENERATE_CMAKE"] = options.generateCMakeFiles ? "true" : "false";
    m_variables["USE_NAMESPACE"] = options.useNamespace ? "true" : "false";
    m_variables["NAMESPACE_PREFIX"] = options.namespacePrefix;
    
    // Timestamps and IDs
    m_variables["GENERATION_DATE"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    m_variables["PLUGIN_UUID"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    // File names
    m_variables["HEADER_FILE"] = formatFileName(options.pluginName) + ".h";
    m_variables["SOURCE_FILE"] = formatFileName(options.pluginName) + ".cpp";
    m_variables["METADATA_FILE"] = formatFileName(options.pluginName) + ".json";
}

QString PluginTemplateEngine::formatClassName(const QString& pluginName) const {
    QString className = pluginName;
    
    // Ensure first character is uppercase
    if (!className.isEmpty()) {
        className[0] = className[0].toUpper();
    }
    
    // Remove invalid characters and ensure camelCase
    className.remove(QRegularExpression("[^A-Za-z0-9_]"));
    
    return className;
}

QString PluginTemplateEngine::formatFileName(const QString& pluginName) const {
    QString fileName = pluginName;
    
    // Convert to lowercase and replace spaces/invalid chars with underscores
    fileName = fileName.toLower();
    fileName.replace(QRegularExpression("[^a-z0-9_]"), "_");
    
    // Remove consecutive underscores
    fileName.replace(QRegularExpression("_+"), "_");
    
    // Remove leading/trailing underscores
    fileName.remove(QRegularExpression("^_+|_+$"));
    
    return fileName;
}

void PluginTemplateEngine::loadBuiltInTemplates() {
    // This would load built-in templates from resources
    // For now, we'll create basic template definitions
    
    TemplateInfo uiTemplate;
    uiTemplate.name = "UI Plugin";
    uiTemplate.description = "Creates a plugin that provides user interface components";
    uiTemplate.type = PluginTemplateType::UIPlugin;
    uiTemplate.complexity = TemplateComplexity::Standard;
    uiTemplate.requiredVariables = {"PLUGIN_NAME", "PLUGIN_DESCRIPTION", "AUTHOR_NAME"};
    m_templates[uiTemplate.name] = uiTemplate;
    
    TemplateInfo serviceTemplate;
    serviceTemplate.name = "Service Plugin";
    serviceTemplate.description = "Creates a background service plugin";
    serviceTemplate.type = PluginTemplateType::ServicePlugin;
    serviceTemplate.complexity = TemplateComplexity::Standard;
    serviceTemplate.requiredVariables = {"PLUGIN_NAME", "PLUGIN_DESCRIPTION", "AUTHOR_NAME"};
    m_templates[serviceTemplate.name] = serviceTemplate;
    
    // Add more built-in templates...
}

// CodeGenerator Implementation
CodeGenerator::CodeGenerator(QObject* parent)
    : QObject(parent)
{
    initializeCodeTemplates();
}

CodeGenerator::~CodeGenerator() = default;

bool CodeGenerator::generateHeaderFile(const CodeGenerationOptions& options, const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to create header file:" << outputPath;
        return false;
    }

    QTextStream stream(&file);

    // Generate license header
    stream << generateLicenseHeader(options) << "\n";

    // Generate include guard
    QString className = formatClassName(options.pluginName);
    stream << generateIncludeGuard(className) << "\n";

    // Generate includes
    stream << generateInterfaceIncludes(options.interfaces) << "\n";

    // Generate namespace wrapper if needed
    QString classContent = generateClassDeclaration(options);
    if (options.useNamespace && !options.namespacePrefix.isEmpty()) {
        classContent = generateNamespaceWrapper(classContent, options.namespacePrefix);
    }

    stream << classContent;
    stream << "\n#endif // " << className.toUpper() << "_H\n";

    emit fileGenerated(outputPath);
    return true;
}

bool CodeGenerator::generateSourceFile(const CodeGenerationOptions& options, const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to create source file:" << outputPath;
        return false;
    }

    QTextStream stream(&file);

    // Generate license header
    stream << generateLicenseHeader(options) << "\n";

    // Include the header file
    QString headerFile = formatFileName(options.pluginName) + ".h";
    stream << "#include \"" << headerFile << "\"\n\n";

    // Generate Qt includes
    stream << "#include <QApplication>\n";
    stream << "#include <QDebug>\n";
    stream << "#include <QJsonObject>\n";
    stream << "#include <QJsonDocument>\n\n";

    // Generate namespace wrapper if needed
    QString implementationContent = generateConstructorImplementation(options) + "\n\n";
    implementationContent += generateMethodImplementations(options);

    if (options.useNamespace && !options.namespacePrefix.isEmpty()) {
        implementationContent = generateNamespaceWrapper(implementationContent, options.namespacePrefix);
    }

    stream << implementationContent;

    emit fileGenerated(outputPath);
    return true;
}

bool CodeGenerator::generateMetadataFile(const CodeGenerationOptions& options, const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to create metadata file:" << outputPath;
        return false;
    }

    QTextStream stream(&file);
    stream << generatePluginMetadata(options);

    emit fileGenerated(outputPath);
    return true;
}

bool CodeGenerator::generateCMakeFile(const CodeGenerationOptions& options, const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to create CMake file:" << outputPath;
        return false;
    }

    QTextStream stream(&file);

    QString pluginName = formatFileName(options.pluginName);
    QString className = formatClassName(options.pluginName);

    stream << "# CMakeLists.txt for " << options.pluginName << " Plugin\n";
    stream << "# Generated on " << QDateTime::currentDateTime().toString() << "\n\n";

    stream << "cmake_minimum_required(VERSION 3.16)\n";
    stream << "project(" << pluginName << ")\n\n";

    stream << "set(CMAKE_CXX_STANDARD " << options.cppStandard << ")\n";
    stream << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n";

    stream << "find_package(Qt6 REQUIRED COMPONENTS Core Widgets)\n\n";

    stream << "set(SOURCES\n";
    stream << "    " << pluginName << ".cpp\n";
    stream << ")\n\n";

    stream << "set(HEADERS\n";
    stream << "    " << pluginName << ".h\n";
    stream << ")\n\n";

    stream << "add_library(" << pluginName << " SHARED ${SOURCES} ${HEADERS})\n\n";

    stream << "target_link_libraries(" << pluginName << "\n";
    stream << "    Qt6::Core\n";
    stream << "    Qt6::Widgets\n";
    stream << ")\n\n";

    stream << "set_target_properties(" << pluginName << " PROPERTIES\n";
    stream << "    OUTPUT_NAME \"" << pluginName << "\"\n";
    stream << "    VERSION " << options.pluginVersion << "\n";
    stream << ")\n";

    emit fileGenerated(outputPath);
    return true;
}

bool CodeGenerator::generateTestFile(const CodeGenerationOptions& options, const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to create test file:" << outputPath;
        return false;
    }

    QTextStream stream(&file);

    stream << generateLicenseHeader(options) << "\n";
    stream << "#include <QtTest/QtTest>\n";
    stream << "#include \"" << formatFileName(options.pluginName) << ".h\"\n\n";

    QString className = formatClassName(options.pluginName);
    stream << "class Test" << className << " : public QObject\n";
    stream << "{\n";
    stream << "    Q_OBJECT\n\n";
    stream << "private slots:\n";
    stream << "    void initTestCase();\n";
    stream << "    void cleanupTestCase();\n";
    stream << "    void testPluginCreation();\n";
    stream << "    void testPluginInitialization();\n";
    stream << "    void testPluginConfiguration();\n";
    stream << "};\n\n";

    // Generate test implementations
    stream << "void Test" << className << "::initTestCase()\n";
    stream << "{\n";
    stream << "    // Test setup\n";
    stream << "}\n\n";

    stream << "void Test" << className << "::cleanupTestCase()\n";
    stream << "{\n";
    stream << "    // Test cleanup\n";
    stream << "}\n\n";

    stream << "void Test" << className << "::testPluginCreation()\n";
    stream << "{\n";
    stream << "    " << className << " plugin;\n";
    stream << "    QVERIFY(!plugin.name().isEmpty());\n";
    stream << "    QVERIFY(!plugin.description().isEmpty());\n";
    stream << "}\n\n";

    stream << "void Test" << className << "::testPluginInitialization()\n";
    stream << "{\n";
    stream << "    " << className << " plugin;\n";
    stream << "    QVERIFY(plugin.initialize());\n";
    stream << "    QVERIFY(plugin.isInitialized());\n";
    stream << "}\n\n";

    stream << "void Test" << className << "::testPluginConfiguration()\n";
    stream << "{\n";
    stream << "    " << className << " plugin;\n";
    stream << "    QJsonObject config = plugin.defaultConfiguration();\n";
    stream << "    QVERIFY(plugin.configure(config));\n";
    stream << "}\n\n";

    stream << "QTEST_MAIN(Test" << className << ")\n";
    stream << "#include \"test_" << formatFileName(options.pluginName) << ".moc\"\n";

    emit fileGenerated(outputPath);
    return true;
}

bool CodeGenerator::generateDocumentationFile(const CodeGenerationOptions& options, const QString& outputPath) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCWarning(templateGenerator) << "Failed to create documentation file:" << outputPath;
        return false;
    }

    QTextStream stream(&file);

    stream << "# " << options.pluginName << " Plugin\n\n";
    stream << options.pluginDescription << "\n\n";

    stream << "## Author\n";
    stream << "- **Name:** " << options.authorName << "\n";
    if (!options.authorEmail.isEmpty()) {
        stream << "- **Email:** " << options.authorEmail << "\n";
    }
    if (!options.organizationName.isEmpty()) {
        stream << "- **Organization:** " << options.organizationName << "\n";
    }
    stream << "\n";

    stream << "## Version\n";
    stream << options.pluginVersion << "\n\n";

    stream << "## Requirements\n";
    stream << "- Qt " << options.qtVersion << " or later\n";
    stream << "- C++" << options.cppStandard << " compatible compiler\n\n";

    stream << "## Features\n";
    for (const QString& feature : options.features) {
        stream << "- " << feature << "\n";
    }
    stream << "\n";

    stream << "## Interfaces\n";
    for (const QString& interface : options.interfaces) {
        stream << "- " << interface << "\n";
    }
    stream << "\n";

    stream << "## Building\n";
    if (options.generateCMakeFiles) {
        stream << "```bash\n";
        stream << "mkdir build\n";
        stream << "cd build\n";
        stream << "cmake ..\n";
        stream << "make\n";
        stream << "```\n\n";
    }

    stream << "## Usage\n";
    stream << "1. Build the plugin using the instructions above\n";
    stream << "2. Copy the generated library to your application's plugin directory\n";
    stream << "3. The plugin will be automatically discovered and loaded\n\n";

    stream << "## Configuration\n";
    stream << "The plugin supports configuration through JSON metadata. ";
    stream << "See the generated .json file for available options.\n\n";

    if (options.generateTests) {
        stream << "## Testing\n";
        stream << "Run the included tests with:\n";
        stream << "```bash\n";
        stream << "cd build\n";
        stream << "ctest\n";
        stream << "```\n\n";
    }

    stream << "## Generated Files\n";
    stream << "- `" << formatFileName(options.pluginName) << ".h` - Plugin header\n";
    stream << "- `" << formatFileName(options.pluginName) << ".cpp` - Plugin implementation\n";
    stream << "- `" << formatFileName(options.pluginName) << ".json` - Plugin metadata\n";
    if (options.generateCMakeFiles) {
        stream << "- `CMakeLists.txt` - Build configuration\n";
    }
    if (options.generateTests) {
        stream << "- `test_" << formatFileName(options.pluginName) << ".cpp` - Unit tests\n";
    }
    stream << "\n";

    stream << "---\n";
    stream << "*Generated on " << QDateTime::currentDateTime().toString() << "*\n";

    emit fileGenerated(outputPath);
    return true;
}

QString CodeGenerator::generateClassDeclaration(const CodeGenerationOptions& options) const {
    QString className = formatClassName(options.pluginName);
    QString result;

    result += "class " + className + " : public QObject";

    // Add interface inheritance
    QString inheritance = generateInterfaceInheritance(options.interfaces);
    if (!inheritance.isEmpty()) {
        result += ", " + inheritance;
    }

    result += "\n{\n";
    result += "    Q_OBJECT\n";

    // Add plugin metadata
    result += "    Q_PLUGIN_METADATA(IID \"com.example.IPlugin/2.0\" FILE \""
              + formatFileName(options.pluginName) + ".json\")\n";

    // Add interface declarations
    result += "    Q_INTERFACES(IPlugin";
    for (const QString& interface : options.interfaces) {
        if (interface != "IPlugin") {
            result += " " + interface;
        }
    }
    result += ")\n\n";

    result += "public:\n";
    result += "    explicit " + className + "(QObject* parent = nullptr);\n";
    result += "    ~" + className + "() override;\n\n";

    // Add interface method declarations
    result += generateInterfaceMethods(options.interfaces, options.complexity);

    result += "\nprivate:\n";
    result += "    bool m_initialized = false;\n";
    result += "    PluginStatus m_status = PluginStatus::Unknown;\n";
    result += "    QJsonObject m_configuration;\n";

    result += "};\n";

    return result;
}

QString CodeGenerator::generateInterfaceImplementation(const CodeGenerationOptions& options) const {
    Q_UNUSED(options)
    // This would generate the implementation based on selected interfaces
    // For now, return basic implementation
    return "// Interface implementations will be generated here\n";
}

QString CodeGenerator::generateConstructorImplementation(const CodeGenerationOptions& options) const {
    QString className = formatClassName(options.pluginName);
    QString result;

    result += className + "::" + className + "(QObject* parent)\n";
    result += "    : QObject(parent)\n";
    result += "{\n";
    result += "    // Initialize plugin\n";
    result += "}\n\n";

    result += className + "::~" + className + "()\n";
    result += "{\n";
    result += "    cleanup();\n";
    result += "}\n";

    return result;
}

QString CodeGenerator::generateMethodImplementations(const CodeGenerationOptions& options) const {
    QString className = formatClassName(options.pluginName);
    QString result;

    // Basic IPlugin methods
    result += "QString " + className + "::name() const\n";
    result += "{\n";
    result += "    return \"" + options.pluginName + "\";\n";
    result += "}\n\n";

    result += "QString " + className + "::description() const\n";
    result += "{\n";
    result += "    return \"" + options.pluginDescription + "\";\n";
    result += "}\n\n";

    result += "QVersionNumber " + className + "::version() const\n";
    result += "{\n";
    result += "    return QVersionNumber::fromString(\"" + options.pluginVersion + "\");\n";
    result += "}\n\n";

    result += "QString " + className + "::author() const\n";
    result += "{\n";
    result += "    return \"" + options.authorName + "\";\n";
    result += "}\n\n";

    result += "QUuid " + className + "::uuid() const\n";
    result += "{\n";
    result += "    return QUuid(\"" + QUuid::createUuid().toString() + "\");\n";
    result += "}\n\n";

    result += "bool " + className + "::initialize()\n";
    result += "{\n";
    result += "    if (m_initialized) {\n";
    result += "        return true;\n";
    result += "    }\n\n";
    result += "    // Initialize plugin resources and connections\n";
    result += "    m_initialized = true;\n";
    result += "    m_status = PluginStatus::Running;\n";
    result += "    return true;\n";
    result += "}\n\n";

    result += "void " + className + "::cleanup()\n";
    result += "{\n";
    result += "    if (!m_initialized) {\n";
    result += "        return;\n";
    result += "    }\n\n";
    result += "    // Cleanup plugin resources\n";
    result += "    m_initialized = false;\n";
    result += "    m_status = PluginStatus::Stopped;\n";
    result += "}\n\n";

    result += "QJsonObject " + className + "::defaultConfiguration() const\n";
    result += "{\n";
    result += "    QJsonObject config;\n";
    result += "    config[\"enabled\"] = true;\n";
    result += "    config[\"version\"] = \"" + options.pluginVersion + "\";\n";
    result += "    return config;\n";
    result += "}\n\n";

    result += "bool " + className + "::configure(const QJsonObject& config)\n";
    result += "{\n";
    result += "    m_configuration = config;\n";
    result += "    return true;\n";
    result += "}\n\n";

    result += "QJsonObject " + className + "::currentConfiguration() const\n";
    result += "{\n";
    result += "    return m_configuration;\n";
    result += "}\n";

    return result;
}

QString CodeGenerator::generateSignalSlotConnections(const CodeGenerationOptions& options) const {
    Q_UNUSED(options)
    // Generate signal-slot connections based on plugin type
    return "// Signal-slot connections will be generated here\n";
}

QString CodeGenerator::generateIncludeGuard(const QString& className) const {
    QString guard = className.toUpper() + "_H";
    QString result;
    result += "#ifndef " + guard + "\n";
    result += "#define " + guard + "\n\n";
    return result;
}

QString CodeGenerator::generateNamespaceWrapper(const QString& content, const QString& namespaceName) const {
    QString result;
    result += "namespace " + namespaceName + " {\n\n";

    // Indent the content
    QStringList lines = content.split('\n');
    for (const QString& line : lines) {
        if (!line.trimmed().isEmpty()) {
            result += "    " + line + "\n";
        } else {
            result += "\n";
        }
    }

    result += "\n} // namespace " + namespaceName + "\n";
    return result;
}

QString CodeGenerator::generateLicenseHeader(const CodeGenerationOptions& options) const {
    QString result;
    result += "/*\n";
    result += " * " + options.pluginName + " Plugin\n";
    result += " * " + options.pluginDescription + "\n";
    result += " *\n";
    result += " * Author: " + options.authorName;
    if (!options.authorEmail.isEmpty()) {
        result += " <" + options.authorEmail + ">";
    }
    result += "\n";
    if (!options.organizationName.isEmpty()) {
        result += " * Organization: " + options.organizationName + "\n";
    }
    result += " * Version: " + options.pluginVersion + "\n";
    result += " * Generated: " + QDateTime::currentDateTime().toString() + "\n";
    result += " *\n";
    result += " * This file was automatically generated by the Plugin Template Generator.\n";
    result += " * Modify as needed for your specific requirements.\n";
    result += " */\n";
    return result;
}

QString CodeGenerator::generatePluginMetadata(const CodeGenerationOptions& options) const {
    QJsonObject metadata;
    metadata["name"] = options.pluginName;
    metadata["description"] = options.pluginDescription;
    metadata["version"] = options.pluginVersion;
    metadata["author"] = options.authorName;
    if (!options.authorEmail.isEmpty()) {
        metadata["email"] = options.authorEmail;
    }
    if (!options.organizationName.isEmpty()) {
        metadata["organization"] = options.organizationName;
    }
    metadata["uuid"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    metadata["type"] = static_cast<int>(options.templateType);
    metadata["qtVersion"] = options.qtVersion;
    metadata["cppStandard"] = options.cppStandard;

    // Add interfaces
    QJsonArray interfacesArray;
    for (const QString& interface : options.interfaces) {
        interfacesArray.append(interface);
    }
    metadata["interfaces"] = interfacesArray;

    // Add features
    QJsonArray featuresArray;
    for (const QString& feature : options.features) {
        featuresArray.append(feature);
    }
    metadata["features"] = featuresArray;

    // Add generation info
    QJsonObject generationInfo;
    generationInfo["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    generationInfo["generator"] = "Plugin Template Generator";
    generationInfo["version"] = "1.0.0";
    metadata["generation"] = generationInfo;

    QJsonDocument doc(metadata);
    return doc.toJson(QJsonDocument::Indented);
}

QString CodeGenerator::formatClassName(const QString& pluginName) const {
    QString className = pluginName;

    // Ensure first character is uppercase
    if (!className.isEmpty()) {
        className[0] = className[0].toUpper();
    }

    // Remove invalid characters and ensure camelCase
    className.remove(QRegularExpression("[^A-Za-z0-9_]"));

    return className;
}

QString CodeGenerator::formatFileName(const QString& pluginName) const {
    QString fileName = pluginName;

    // Convert to lowercase and replace spaces/invalid chars with underscores
    fileName = fileName.toLower();
    fileName.replace(QRegularExpression("[^a-z0-9_]"), "_");

    // Remove consecutive underscores
    fileName.replace(QRegularExpression("_+"), "_");

    // Remove leading/trailing underscores
    fileName.remove(QRegularExpression("^_+|_+$"));

    return fileName;
}

QString CodeGenerator::formatVariableName(const QString& name) const {
    QString varName = name;

    // Convert to camelCase
    if (!varName.isEmpty()) {
        varName[0] = varName[0].toLower();
    }

    // Remove invalid characters
    varName.remove(QRegularExpression("[^A-Za-z0-9_]"));

    return varName;
}

QString CodeGenerator::generateInterfaceIncludes(const QStringList& interfaces) const {
    QString result;
    result += "#include <QObject>\n";
    result += "#include <QJsonObject>\n";
    result += "#include <QUuid>\n";
    result += "#include <QVersionNumber>\n";
    result += "#include \"../core/PluginInterface.h\"\n";

    for (const QString& interface : interfaces) {
        if (interface != "IPlugin") {
            result += "#include \"../core/AdvancedInterfaces.h\"\n";
            break;
        }
    }

    return result;
}

QString CodeGenerator::generateInterfaceInheritance(const QStringList& interfaces) const {
    QStringList inheritance;

    for (const QString& interface : interfaces) {
        if (interface != "IPlugin") {  // IPlugin is always included
            inheritance.append("public " + interface);
        }
    }

    return inheritance.join(", ");
}

QString CodeGenerator::generateInterfaceMethods(const QStringList& interfaces, TemplateComplexity complexity) const {
    QString result;

    // Always include basic IPlugin methods
    result += "    // IPlugin interface\n";
    result += "    QString name() const override;\n";
    result += "    QString description() const override;\n";
    result += "    QVersionNumber version() const override;\n";
    result += "    QString author() const override;\n";
    result += "    QUuid uuid() const override;\n";
    result += "    bool initialize() override;\n";
    result += "    void cleanup() override;\n";
    result += "    bool isInitialized() const override { return m_initialized; }\n";
    result += "    PluginStatus status() const override { return m_status; }\n";
    result += "    QJsonObject defaultConfiguration() const override;\n";
    result += "    bool configure(const QJsonObject& config) override;\n";
    result += "    QJsonObject currentConfiguration() const override;\n";

    // Add interface-specific methods based on complexity
    if (complexity != TemplateComplexity::Basic) {
        for (const QString& interface : interfaces) {
            if (interface == "IUIPlugin") {
                result += "\n    // IUIPlugin interface\n";
                result += "    std::unique_ptr<QWidget> createWidget(QWidget* parent = nullptr) override;\n";
                result += "    QWidget* createConfigurationWidget(QWidget* parent = nullptr) override;\n";
            } else if (interface == "IServicePlugin") {
                result += "\n    // IServicePlugin interface\n";
                result += "    bool startService() override;\n";
                result += "    bool stopService() override;\n";
                result += "    bool isServiceRunning() const override;\n";
                result += "    QJsonObject serviceStatus() const override;\n";
            }
            // Add more interface methods as needed
        }
    }

    return result;
}

void CodeGenerator::initializeCodeTemplates() {
    // Initialize code templates for different plugin types
    // This could be loaded from external template files

    m_codeTemplates["ui_plugin_widget"] = R"(
std::unique_ptr<QWidget> {{CLASS_NAME}}::createWidget(QWidget* parent) {
    auto widget = std::make_unique<QWidget>(parent);
    // TODO: Implement widget creation
    return widget;
}
)";

    m_codeTemplates["service_plugin_start"] = R"(
bool {{CLASS_NAME}}::startService() {
    // TODO: Implement service startup
    return true;
}
)";

    // Add more templates as needed
}
