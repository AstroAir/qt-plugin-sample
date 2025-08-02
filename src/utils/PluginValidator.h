// PluginValidator.h - Real-time Plugin Code and Metadata Validation System
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QFileInfo>
#include <QDir>
#include <memory>

// Forward declarations
class PluginCodeAnalyzer;
class PluginMetadataValidator;
class PluginInterfaceChecker;
class ValidationResultsWidget;
class CodeHighlighter;

// Validation severity levels
enum class ValidationSeverity {
    Info,
    Warning,
    Error,
    Critical
};

// Validation issue types
enum class ValidationIssueType {
    SyntaxError,
    InterfaceCompliance,
    MetadataError,
    DependencyIssue,
    SecurityConcern,
    PerformanceWarning,
    BestPracticeViolation,
    DocumentationMissing
};

// Validation result structure
struct ValidationResult {
    ValidationSeverity severity;
    ValidationIssueType type;
    QString message;
    QString file;
    int line = -1;
    int column = -1;
    QString suggestion;
    QString code;
    
    ValidationResult() = default;
    ValidationResult(ValidationSeverity sev, ValidationIssueType t, const QString& msg, 
                    const QString& f = "", int l = -1, int c = -1)
        : severity(sev), type(t), message(msg), file(f), line(l), column(c) {}
};

// Plugin validation configuration
struct ValidationConfig {
    bool enableRealTimeValidation = true;
    bool validateSyntax = true;
    bool validateInterfaces = true;
    bool validateMetadata = true;
    bool validateDependencies = true;
    bool checkSecurity = true;
    bool checkPerformance = true;
    bool checkBestPractices = true;
    bool checkDocumentation = true;
    QStringList includePaths;
    QStringList excludePatterns;
    QString cppStandard = "20";
    QString qtVersion = "6.5";
};

// Main plugin validator class
class PluginValidator : public QObject {
    Q_OBJECT

public:
    explicit PluginValidator(QObject* parent = nullptr);
    ~PluginValidator() override;

    // Configuration
    void setConfiguration(const ValidationConfig& config);
    ValidationConfig configuration() const;
    
    // Validation operations
    void validatePlugin(const QString& pluginPath);
    void validateFile(const QString& filePath);
    void validateCode(const QString& code, const QString& fileName = "");
    void validateMetadata(const QJsonObject& metadata, const QString& filePath = "");
    
    // Real-time validation
    void enableRealTimeValidation(bool enable);
    void addWatchedDirectory(const QString& directory);
    void removeWatchedDirectory(const QString& directory);
    void clearWatchedDirectories();
    
    // Results
    QList<ValidationResult> getResults() const;
    QList<ValidationResult> getResultsForFile(const QString& filePath) const;
    void clearResults();
    
    // Statistics
    int getErrorCount() const;
    int getWarningCount() const;
    int getInfoCount() const;
    bool hasErrors() const;
    bool hasWarnings() const;

signals:
    void validationStarted(const QString& target);
    void validationProgress(int percentage, const QString& currentFile);
    void validationCompleted(const QString& target, bool success);
    void validationResultsUpdated();
    void fileValidated(const QString& filePath, const QList<ValidationResult>& results);
    void realTimeValidationTriggered(const QString& filePath);

private slots:
    void onFileChanged(const QString& filePath);
    void onDirectoryChanged(const QString& directoryPath);
    void onRealTimeValidationTimer();

private:
    ValidationConfig m_config;
    QList<ValidationResult> m_results;
    std::unique_ptr<PluginCodeAnalyzer> m_codeAnalyzer;
    std::unique_ptr<PluginMetadataValidator> m_metadataValidator;
    std::unique_ptr<PluginInterfaceChecker> m_interfaceChecker;
    std::unique_ptr<QFileSystemWatcher> m_fileWatcher;
    std::unique_ptr<QTimer> m_realTimeTimer;
    QSet<QString> m_watchedDirectories;
    QSet<QString> m_pendingFiles;
    
    void initializeValidators();
    void processValidationQueue();
    void mergeResults(const QList<ValidationResult>& newResults);
    QString getFileType(const QString& filePath) const;
    bool shouldValidateFile(const QString& filePath) const;
};

// PluginCodeAnalyzer is defined in PluginCodeAnalyzer.h

// Plugin metadata validator for JSON metadata validation
class PluginMetadataValidator : public QObject {
    Q_OBJECT

public:
    explicit PluginMetadataValidator(QObject* parent = nullptr);
    ~PluginMetadataValidator() override;

    // Metadata validation
    QList<ValidationResult> validateMetadata(const QJsonObject& metadata, const QString& filePath = "");
    QList<ValidationResult> validateMetadataFile(const QString& filePath);
    
    // Schema validation
    void setRequiredFields(const QStringList& fields);
    void setOptionalFields(const QStringList& fields);
    void setFieldValidators(const QMap<QString, std::function<bool(const QJsonValue&)>>& validators);

signals:
    void validationProgress(int percentage);
    void validationCompleted(const QList<ValidationResult>& results);

private:
    QStringList m_requiredFields;
    QStringList m_optionalFields;
    QMap<QString, std::function<bool(const QJsonValue&)>> m_fieldValidators;
    
    // Validation methods
    QList<ValidationResult> checkRequiredFields(const QJsonObject& metadata);
    QList<ValidationResult> checkFieldTypes(const QJsonObject& metadata);
    QList<ValidationResult> checkFieldValues(const QJsonObject& metadata);
    QList<ValidationResult> checkVersionFormat(const QString& version);
    QList<ValidationResult> checkUuidFormat(const QString& uuid);
    QList<ValidationResult> checkDependencies(const QJsonArray& dependencies);
    
    void initializeDefaultValidators();
};

// Plugin interface checker for interface compliance validation
class PluginInterfaceChecker : public QObject {
    Q_OBJECT

public:
    explicit PluginInterfaceChecker(QObject* parent = nullptr);
    ~PluginInterfaceChecker() override;

    // Interface checking
    QList<ValidationResult> checkInterfaceCompliance(const QString& code, const QString& fileName = "");
    QList<ValidationResult> checkInterfaceImplementation(const QString& code, const QStringList& interfaces);
    
    // Interface definitions
    void addInterfaceDefinition(const QString& interfaceName, const QStringList& requiredMethods);
    void removeInterfaceDefinition(const QString& interfaceName);
    void clearInterfaceDefinitions();

signals:
    void checkProgress(int percentage);
    void checkCompleted(const QList<ValidationResult>& results);

private:
    QMap<QString, QStringList> m_interfaceDefinitions;
    
    // Checking methods
    QList<ValidationResult> checkMethodImplementation(const QString& code, const QString& interfaceName, const QStringList& methods);
    QList<ValidationResult> checkMethodSignatures(const QString& code, const QString& interfaceName);
    QList<ValidationResult> checkVirtualOverrides(const QString& code);
    
    // Helper methods
    QStringList extractImplementedMethods(const QString& code, const QString& interfaceName);
    bool isMethodImplemented(const QString& code, const QString& methodSignature);
    QString extractMethodSignature(const QString& methodDeclaration);
    
    void initializeBuiltInInterfaces();
};

// Validation results widget for displaying validation results
class ValidationResultsWidget : public QWidget {
    Q_OBJECT

public:
    explicit ValidationResultsWidget(QWidget* parent = nullptr);
    ~ValidationResultsWidget() override;

    // Results management
    void setResults(const QList<ValidationResult>& results);
    void addResults(const QList<ValidationResult>& results);
    void clearResults();
    
    // Filtering
    void setFilterSeverity(ValidationSeverity severity, bool show);
    void setFilterType(ValidationIssueType type, bool show);
    void setFilterFile(const QString& filePath);
    void clearFilters();
    
    // Statistics
    void updateStatistics();

signals:
    void resultSelected(const ValidationResult& result);
    void resultDoubleClicked(const ValidationResult& result);
    void filterChanged();

private slots:
    void onItemSelectionChanged();
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);
    void onFilterButtonClicked();

private:
    QTreeWidget* m_resultsTree;
    QLabel* m_statisticsLabel;
    QPushButton* m_filterButton;
    QList<ValidationResult> m_allResults;
    QList<ValidationResult> m_filteredResults;
    QMap<ValidationSeverity, bool> m_severityFilters;
    QMap<ValidationIssueType, bool> m_typeFilters;
    QString m_fileFilter;
    
    void setupUI();
    void populateResults();
    void applyFilters();
    QTreeWidgetItem* createResultItem(const ValidationResult& result);
    QIcon getIconForSeverity(ValidationSeverity severity);
    QString getSeverityText(ValidationSeverity severity);
    QString getTypeText(ValidationIssueType type);
};

// Code highlighter for syntax highlighting in validation views
class CodeHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit CodeHighlighter(QTextDocument* parent = nullptr);
    ~CodeHighlighter() override;

    // Highlighting configuration
    void setValidationResults(const QList<ValidationResult>& results);
    void clearValidationResults();
    
    // Highlight rules
    void addHighlightRule(const QRegularExpression& pattern, const QTextCharFormat& format);
    void removeHighlightRule(const QRegularExpression& pattern);
    void clearHighlightRules();

protected:
    void highlightBlock(const QString& text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    QList<HighlightRule> m_highlightRules;
    QList<ValidationResult> m_validationResults;
    QTextCharFormat m_errorFormat;
    QTextCharFormat m_warningFormat;
    QTextCharFormat m_infoFormat;
    
    void initializeFormats();
    void highlightValidationIssues(const QString& text);
};

// Plugin validation dialog for interactive validation
class PluginValidationDialog : public QDialog {
    Q_OBJECT

public:
    explicit PluginValidationDialog(QWidget* parent = nullptr);
    ~PluginValidationDialog() override;

    // Validation operations
    void validatePlugin(const QString& pluginPath);
    void validateFiles(const QStringList& filePaths);
    
    // Configuration
    void setValidationConfig(const ValidationConfig& config);
    ValidationConfig getValidationConfig() const;

public slots:
    void accept() override;
    void reject() override;

private slots:
    void onValidateClicked();
    void onConfigureClicked();
    void onResultSelected(const ValidationResult& result);
    void onValidationCompleted(const QString& target, bool success);

private:
    PluginValidator* m_validator;
    ValidationResultsWidget* m_resultsWidget;
    QPlainTextEdit* m_codePreview;
    CodeHighlighter* m_highlighter;
    QPushButton* m_validateButton;
    QPushButton* m_configureButton;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    
    void setupUI();
    void updateCodePreview(const ValidationResult& result);
    void showConfigurationDialog();
};
