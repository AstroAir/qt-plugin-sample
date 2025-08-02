// PluginValidator.cpp - Implementation of Plugin Validation System
#include "PluginValidator.h"
#include "PluginCodeAnalyzer.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QTextStream>
#include <QJsonArray>
#include <QUuid>
#include <QVersionNumber>
#include <QHeaderView>
#include <QSplitter>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QDialogButtonBox>

Q_LOGGING_CATEGORY(pluginValidator, "plugin.validator")

// PluginValidator Implementation
PluginValidator::PluginValidator(QObject* parent)
    : QObject(parent)
    , m_codeAnalyzer(std::make_unique<PluginCodeAnalyzer>(this))
    , m_metadataValidator(std::make_unique<PluginMetadataValidator>(this))
    , m_interfaceChecker(std::make_unique<PluginInterfaceChecker>(this))
    , m_fileWatcher(std::make_unique<QFileSystemWatcher>(this))
    , m_realTimeTimer(std::make_unique<QTimer>(this))
{
    initializeValidators();
    
    // Setup real-time validation timer
    m_realTimeTimer->setSingleShot(true);
    m_realTimeTimer->setInterval(500); // 500ms delay for real-time validation
    connect(m_realTimeTimer.get(), &QTimer::timeout, this, &PluginValidator::onRealTimeValidationTimer);
    
    // Connect file watcher
    connect(m_fileWatcher.get(), &QFileSystemWatcher::fileChanged, this, &PluginValidator::onFileChanged);
    connect(m_fileWatcher.get(), &QFileSystemWatcher::directoryChanged, this, &PluginValidator::onDirectoryChanged);
    
    // Connect analyzer signals
    connect(m_codeAnalyzer.get(), &PluginCodeAnalyzer::validationProgress,
            this, [this](int percentage) {
                emit validationProgress(percentage, "Analyzing code...");
            });
    
    connect(m_metadataValidator.get(), &PluginMetadataValidator::validationProgress,
            this, [this](int percentage) {
                emit validationProgress(percentage, "Validating metadata...");
            });
    
    connect(m_interfaceChecker.get(), &PluginInterfaceChecker::checkProgress,
            this, [this](int percentage) {
                emit validationProgress(percentage, "Checking interfaces...");
            });
}

PluginValidator::~PluginValidator() = default;

void PluginValidator::setConfiguration(const ValidationConfig& config) {
    m_config = config;
    
    // Update analyzers with new configuration
    m_codeAnalyzer->setIncludePaths(config.includePaths);
    m_codeAnalyzer->setCppStandard(config.cppStandard);
    m_codeAnalyzer->setQtVersion(config.qtVersion);
    
    // Enable/disable real-time validation
    enableRealTimeValidation(config.enableRealTimeValidation);
}

ValidationConfig PluginValidator::configuration() const {
    return m_config;
}

void PluginValidator::validatePlugin(const QString& pluginPath) {
    emit validationStarted(pluginPath);
    clearResults();
    
    QFileInfo pathInfo(pluginPath);
    if (!pathInfo.exists()) {
        ValidationResult result(ValidationSeverity::Error, ValidationIssueType::SyntaxError,
                              "Plugin path does not exist", pluginPath);
        m_results.append(result);
        emit validationCompleted(pluginPath, false);
        return;
    }
    
    QStringList filesToValidate;
    
    if (pathInfo.isFile()) {
        filesToValidate.append(pluginPath);
    } else if (pathInfo.isDir()) {
        // Find all relevant files in the directory
        QDir dir(pluginPath);
        QStringList filters;
        filters << "*.h" << "*.hpp" << "*.cpp" << "*.cxx" << "*.cc" << "*.json";
        
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);
        for (const QFileInfo& fileInfo : files) {
            if (shouldValidateFile(fileInfo.absoluteFilePath())) {
                filesToValidate.append(fileInfo.absoluteFilePath());
            }
        }
    }
    
    if (filesToValidate.isEmpty()) {
        ValidationResult result(ValidationSeverity::Warning, ValidationIssueType::SyntaxError,
                              "No files found to validate", pluginPath);
        m_results.append(result);
        emit validationCompleted(pluginPath, true);
        return;
    }
    
    // Validate each file
    int totalFiles = filesToValidate.size();
    for (int i = 0; i < totalFiles; ++i) {
        const QString& filePath = filesToValidate[i];
        emit validationProgress((i * 100) / totalFiles, filePath);
        
        validateFile(filePath);
    }
    
    emit validationProgress(100, "Validation completed");
    emit validationCompleted(pluginPath, !hasErrors());
    emit validationResultsUpdated();
}

void PluginValidator::validateFile(const QString& filePath) {
    if (!shouldValidateFile(filePath)) {
        return;
    }
    
    QList<ValidationResult> fileResults;
    QString fileType = getFileType(filePath);
    
    if (fileType == "cpp" || fileType == "h") {
        // Validate C++ code
        if (m_config.validateSyntax || m_config.validateInterfaces || 
            m_config.checkSecurity || m_config.checkPerformance || 
            m_config.checkBestPractices || m_config.checkDocumentation) {
            
            QList<ValidationResult> codeResults = m_codeAnalyzer->analyzeFileForValidator(filePath);
            fileResults.append(codeResults);
        }
        
        // Check interface compliance
        if (m_config.validateInterfaces) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString code = QTextStream(&file).readAll();
                QList<ValidationResult> interfaceResults = m_interfaceChecker->checkInterfaceCompliance(code, filePath);
                fileResults.append(interfaceResults);
            }
        }
    } else if (fileType == "json") {
        // Validate JSON metadata
        if (m_config.validateMetadata) {
            QList<ValidationResult> metadataResults = m_metadataValidator->validateMetadataFile(filePath);
            fileResults.append(metadataResults);
        }
    }
    
    // Merge results
    mergeResults(fileResults);
    emit fileValidated(filePath, fileResults);
}

void PluginValidator::validateCode(const QString& code, const QString& fileName) {
    QList<ValidationResult> results;
    
    if (m_config.validateSyntax || m_config.validateInterfaces || 
        m_config.checkSecurity || m_config.checkPerformance || 
        m_config.checkBestPractices || m_config.checkDocumentation) {
        
        QList<ValidationResult> codeResults = m_codeAnalyzer->analyzeCodeForValidator(code, fileName);
        results.append(codeResults);
    }
    
    if (m_config.validateInterfaces) {
        QList<ValidationResult> interfaceResults = m_interfaceChecker->checkInterfaceCompliance(code, fileName);
        results.append(interfaceResults);
    }
    
    mergeResults(results);
    emit fileValidated(fileName, results);
    emit validationResultsUpdated();
}

void PluginValidator::validateMetadata(const QJsonObject& metadata, const QString& filePath) {
    if (!m_config.validateMetadata) {
        return;
    }
    
    QList<ValidationResult> results = m_metadataValidator->validateMetadata(metadata, filePath);
    mergeResults(results);
    emit fileValidated(filePath, results);
    emit validationResultsUpdated();
}

void PluginValidator::enableRealTimeValidation(bool enable) {
    m_config.enableRealTimeValidation = enable;
    
    if (!enable) {
        m_realTimeTimer->stop();
        m_pendingFiles.clear();
    }
}

void PluginValidator::addWatchedDirectory(const QString& directory) {
    if (m_watchedDirectories.contains(directory)) {
        return;
    }
    
    m_watchedDirectories.insert(directory);
    m_fileWatcher->addPath(directory);
    
    // Also watch all files in the directory
    QDir dir(directory);
    QStringList filters;
    filters << "*.h" << "*.hpp" << "*.cpp" << "*.cxx" << "*.cc" << "*.json";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable);
    for (const QFileInfo& fileInfo : files) {
        m_fileWatcher->addPath(fileInfo.absoluteFilePath());
    }
    
    qCInfo(pluginValidator) << "Added watched directory:" << directory;
}

void PluginValidator::removeWatchedDirectory(const QString& directory) {
    if (!m_watchedDirectories.contains(directory)) {
        return;
    }
    
    m_watchedDirectories.remove(directory);
    m_fileWatcher->removePath(directory);
    
    // Remove all files in the directory from watching
    QDir dir(directory);
    QStringList filters;
    filters << "*.h" << "*.hpp" << "*.cpp" << "*.cxx" << "*.cc" << "*.json";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable);
    for (const QFileInfo& fileInfo : files) {
        m_fileWatcher->removePath(fileInfo.absoluteFilePath());
    }
    
    qCInfo(pluginValidator) << "Removed watched directory:" << directory;
}

void PluginValidator::clearWatchedDirectories() {
    for (const QString& directory : m_watchedDirectories) {
        m_fileWatcher->removePath(directory);
    }
    m_watchedDirectories.clear();
    
    // Clear all watched files
    QStringList watchedFiles = m_fileWatcher->files();
    for (const QString& file : watchedFiles) {
        m_fileWatcher->removePath(file);
    }
    
    qCInfo(pluginValidator) << "Cleared all watched directories";
}

QList<ValidationResult> PluginValidator::getResults() const {
    return m_results;
}

QList<ValidationResult> PluginValidator::getResultsForFile(const QString& filePath) const {
    QList<ValidationResult> fileResults;
    for (const ValidationResult& result : m_results) {
        if (result.file == filePath) {
            fileResults.append(result);
        }
    }
    return fileResults;
}

void PluginValidator::clearResults() {
    m_results.clear();
    emit validationResultsUpdated();
}

int PluginValidator::getErrorCount() const {
    int count = 0;
    for (const ValidationResult& result : m_results) {
        if (result.severity == ValidationSeverity::Error || result.severity == ValidationSeverity::Critical) {
            count++;
        }
    }
    return count;
}

int PluginValidator::getWarningCount() const {
    int count = 0;
    for (const ValidationResult& result : m_results) {
        if (result.severity == ValidationSeverity::Warning) {
            count++;
        }
    }
    return count;
}

int PluginValidator::getInfoCount() const {
    int count = 0;
    for (const ValidationResult& result : m_results) {
        if (result.severity == ValidationSeverity::Info) {
            count++;
        }
    }
    return count;
}

bool PluginValidator::hasErrors() const {
    return getErrorCount() > 0;
}

bool PluginValidator::hasWarnings() const {
    return getWarningCount() > 0;
}

void PluginValidator::onFileChanged(const QString& filePath) {
    if (!m_config.enableRealTimeValidation) {
        return;
    }
    
    m_pendingFiles.insert(filePath);
    m_realTimeTimer->start(); // Restart timer
    
    emit realTimeValidationTriggered(filePath);
}

void PluginValidator::onDirectoryChanged(const QString& directoryPath) {
    if (!m_config.enableRealTimeValidation) {
        return;
    }
    
    // Check for new files in the directory
    QDir dir(directoryPath);
    QStringList filters;
    filters << "*.h" << "*.hpp" << "*.cpp" << "*.cxx" << "*.cc" << "*.json";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable);
    for (const QFileInfo& fileInfo : files) {
        QString filePath = fileInfo.absoluteFilePath();
        if (!m_fileWatcher->files().contains(filePath)) {
            m_fileWatcher->addPath(filePath);
            m_pendingFiles.insert(filePath);
        }
    }
    
    if (!m_pendingFiles.isEmpty()) {
        m_realTimeTimer->start();
    }
}

void PluginValidator::onRealTimeValidationTimer() {
    if (m_pendingFiles.isEmpty()) {
        return;
    }
    
    processValidationQueue();
}

void PluginValidator::initializeValidators() {
    // Initialize default configuration
    m_config.enableRealTimeValidation = true;
    m_config.validateSyntax = true;
    m_config.validateInterfaces = true;
    m_config.validateMetadata = true;
    m_config.validateDependencies = true;
    m_config.checkSecurity = true;
    m_config.checkPerformance = true;
    m_config.checkBestPractices = true;
    m_config.checkDocumentation = true;
    m_config.cppStandard = "20";
    m_config.qtVersion = "6.5";
}

void PluginValidator::processValidationQueue() {
    QSet<QString> filesToProcess = m_pendingFiles;
    m_pendingFiles.clear();
    
    for (const QString& filePath : filesToProcess) {
        if (shouldValidateFile(filePath)) {
            // Remove old results for this file
            m_results.erase(std::remove_if(m_results.begin(), m_results.end(),
                [&filePath](const ValidationResult& result) {
                    return result.file == filePath;
                }), m_results.end());
            
            // Validate the file
            validateFile(filePath);
        }
    }
    
    emit validationResultsUpdated();
}

void PluginValidator::mergeResults(const QList<ValidationResult>& newResults) {
    m_results.append(newResults);
}

QString PluginValidator::getFileType(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "cpp" || suffix == "cxx" || suffix == "cc") {
        return "cpp";
    } else if (suffix == "h" || suffix == "hpp") {
        return "h";
    } else if (suffix == "json") {
        return "json";
    }
    
    return "unknown";
}

bool PluginValidator::shouldValidateFile(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    
    // Check exclude patterns
    for (const QString& pattern : m_config.excludePatterns) {
        QRegularExpression regex(pattern);
        if (regex.match(fileName).hasMatch()) {
            return false;
        }
    }
    
    // Check file type
    QString fileType = getFileType(filePath);
    return (fileType == "cpp" || fileType == "h" || fileType == "json");
}

// Note: PluginCodeAnalyzer is implemented in a separate file
// Note: MOC file will be generated automatically by CMake

