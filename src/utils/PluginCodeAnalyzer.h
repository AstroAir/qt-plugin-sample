// PluginCodeAnalyzer.h - Advanced Code Analysis and Quality Assessment Tools
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QProcess>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QRegularExpression>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <memory>

// Forward declarations
class PluginCodeAnalyzer;
class StaticAnalyzer;
class QualityMetrics;
class RefactoringEngine;
class CodeComplexityAnalyzer;
class SecurityAnalyzer;
class AnalysisWidget;
class ValidationResult;

// Analysis types
enum class AnalysisType {
    Syntax,         // Syntax checking
    Semantic,       // Semantic analysis
    Style,          // Code style checking
    Complexity,     // Complexity analysis
    Security,       // Security vulnerability analysis
    Performance,    // Performance analysis
    Documentation,  // Documentation analysis
    Dependencies,   // Dependency analysis
    Testing,        // Test coverage analysis
    Maintainability // Maintainability analysis
};

// Issue severity levels
enum class IssueSeverity {
    Info,           // Informational
    Warning,        // Warning
    Error,          // Error
    Critical,       // Critical issue
    Suggestion      // Improvement suggestion
};

// Code analysis metrics (separate from performance metrics)
enum class CodeMetricType {
    LinesOfCode,            // Total lines of code
    CyclomaticComplexity,   // Cyclomatic complexity
    CognitiveComplexity,    // Cognitive complexity
    NestingDepth,           // Maximum nesting depth
    FunctionLength,         // Function length
    ClassSize,              // Class size
    ParameterCount,         // Parameter count
    Coupling,               // Coupling between objects
    Cohesion,               // Class cohesion
    Duplication,            // Code duplication
    TestCoverage,           // Test coverage percentage
    TechnicalDebt,          // Technical debt estimation
    Maintainability,        // Maintainability index
    Reliability,            // Reliability rating
    Security                // Security rating
};

// Analysis issue
struct AnalysisIssue {
    QString id;
    QString pluginId;
    QString filePath;
    int lineNumber;
    int columnNumber;
    AnalysisType analysisType;
    IssueSeverity severity;
    QString title;
    QString description;
    QString rule;
    QString category;
    QStringList tags;
    QString suggestion;
    QString codeSnippet;
    QDateTime detectedTime;
    bool isFixed;
    QDateTime fixedTime;
    QJsonObject metadata;
    
    AnalysisIssue() = default;
    AnalysisIssue(const QString& file, int line, IssueSeverity sev, const QString& desc)
        : filePath(file), lineNumber(line), severity(sev), description(desc),
          detectedTime(QDateTime::currentDateTime()), isFixed(false) {
        id = generateIssueId();
    }
    
    QString getSeverityString() const;
    QString getLocationString() const;
    QString getAnalysisTypeString() const;
    bool isError() const { return severity == IssueSeverity::Error || severity == IssueSeverity::Critical; }
    
private:
    QString generateIssueId() const;
};

// Code metric result
struct MetricResult {
    QString pluginId;
    QString filePath;
    QString function;
    QString className;
    CodeMetricType type;
    double value;
    double threshold;
    QString unit;
    QString description;
    QDateTime measuredTime;
    QJsonObject metadata;
    
    MetricResult() = default;
    MetricResult(CodeMetricType t, double v, const QString& desc = "")
        : type(t), value(v), description(desc), measuredTime(QDateTime::currentDateTime()) {}
    
    bool exceedsThreshold() const;
    QString getMetricTypeString() const;
    QString getFormattedValue() const;
    QString getRating() const;
};

// Analysis configuration
struct AnalysisConfig {
    QSet<AnalysisType> enabledAnalyses;
    QMap<CodeMetricType, double> metricThresholds;
    QStringList includePaths;
    QStringList excludePaths;
    QStringList fileExtensions;
    bool enableAutoFix = false;
    bool enableSuggestions = true;
    bool enableMetrics = true;
    bool enableSecurity = true;
    bool enablePerformance = true;
    QString outputFormat = "json";
    QString outputDirectory;
    int maxIssues = 1000;
    bool stopOnError = false;
    QJsonObject customRules;
    
    AnalysisConfig() {
        // Default enabled analyses
        enabledAnalyses << AnalysisType::Syntax << AnalysisType::Style << AnalysisType::Complexity;
        
        // Default thresholds
        metricThresholds[CodeMetricType::CyclomaticComplexity] = 10.0;
        metricThresholds[CodeMetricType::FunctionLength] = 50.0;
        metricThresholds[CodeMetricType::ParameterCount] = 5.0;
        metricThresholds[CodeMetricType::NestingDepth] = 4.0;
        
        // Default file extensions
        fileExtensions << "cpp" << "h" << "hpp" << "c" << "cc" << "cxx" << "qml" << "js";
    }
};

// Analysis report
struct AnalysisReport {
    QString reportId;
    QString pluginId;
    QDateTime analysisTime;
    QDateTime completionTime;
    int totalFiles;
    int analyzedFiles;
    QList<AnalysisIssue> issues;
    QList<MetricResult> metrics;
    QMap<IssueSeverity, int> issueCounts;
    QMap<AnalysisType, int> analysisTypeCounts;
    double overallScore;
    QString grade; // A, B, C, D, F
    QStringList recommendations;
    QJsonObject summary;
    
    AnalysisReport() = default;
    AnalysisReport(const QString& plugId)
        : pluginId(plugId), analysisTime(QDateTime::currentDateTime()),
          totalFiles(0), analyzedFiles(0), overallScore(0.0) {
        reportId = generateReportId();
    }
    
    int getTotalIssues() const;
    int getErrorCount() const;
    int getWarningCount() const;
    double getQualityScore() const;
    QString getSummary() const;
    
private:
    QString generateReportId() const;
};

// Main code analyzer
class PluginCodeAnalyzer : public QObject {
    Q_OBJECT

public:
    explicit PluginCodeAnalyzer(QObject* parent = nullptr);
    ~PluginCodeAnalyzer() override;

    // Analysis operations
    QString analyzePlugin(const QString& pluginId, const QString& sourcePath, const AnalysisConfig& config = AnalysisConfig());
    QString analyzeFile(const QString& filePath, const AnalysisConfig& config = AnalysisConfig());
    QString analyzeCode(const QString& code, const QString& language, const AnalysisConfig& config = AnalysisConfig());
    void cancelAnalysis(const QString& analysisId);
    bool isAnalyzing(const QString& analysisId) const;
    
    // Results management
    AnalysisReport getAnalysisReport(const QString& analysisId) const;
    QList<AnalysisIssue> getIssues(const QString& analysisId) const;
    QList<MetricResult> getMetrics(const QString& analysisId) const;
    QList<AnalysisIssue> getIssuesForFile(const QString& filePath) const;
    void markIssueFixed(const QString& issueId);
    void dismissIssue(const QString& issueId);
    
    // Configuration
    void setAnalysisConfig(const AnalysisConfig& config);
    AnalysisConfig analysisConfig() const;
    void setMetricThreshold(CodeMetricType type, double threshold);
    double getMetricThreshold(CodeMetricType type) const;
    void enableAnalysisType(AnalysisType type, bool enable);
    bool isAnalysisTypeEnabled(AnalysisType type) const;
    
    // Custom rules
    void addCustomRule(const QString& ruleId, const QJsonObject& rule);
    void removeCustomRule(const QString& ruleId);
    QJsonObject getCustomRule(const QString& ruleId) const;
    QStringList getCustomRules() const;
    
    // Analysis history
    QStringList getAnalysisHistory(const QString& pluginId = "") const;
    void clearAnalysisHistory(const QString& pluginId = "");
    AnalysisReport getHistoricalReport(const QString& reportId) const;
    
    // Export and reporting
    void exportReport(const QString& analysisId, const QString& filePath, const QString& format = "html");
    void generateTrendReport(const QString& pluginId, const QString& filePath);
    void generateComparisonReport(const QStringList& analysisIds, const QString& filePath);
    
    // Integration
    void setExternalAnalyzer(const QString& name, const QString& command);
    void removeExternalAnalyzer(const QString& name);
    QStringList getExternalAnalyzers() const;

    // Legacy compatibility methods for PluginValidator
    QList<ValidationResult> analyzeCodeForValidator(const QString& code, const QString& fileName);
    QList<ValidationResult> analyzeFileForValidator(const QString& filePath);
    void setIncludePaths(const QStringList& paths);
    void setCppStandard(const QString& standard);
    void setQtVersion(const QString& version);

signals:
    // Legacy compatibility signal for PluginValidator
    void validationProgress(int percentage);
    void analysisStarted(const QString& analysisId, const QString& pluginId);
    void analysisProgress(const QString& analysisId, int percentage, const QString& currentFile);
    void analysisCompleted(const QString& analysisId, const AnalysisReport& report);
    void analysisFailed(const QString& analysisId, const QString& error);
    void issueFound(const QString& analysisId, const AnalysisIssue& issue);
    void metricCalculated(const QString& analysisId, const MetricResult& metric);
    void issueFixed(const QString& issueId);
    void issueDismissed(const QString& issueId);

public slots:
    void showAnalysisWidget();
    void showAnalysisWidget(const QString& pluginId);
    void runQuickAnalysis(const QString& pluginId);

private slots:
    void onAnalysisThreadFinished();
    void onExternalAnalyzerFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    struct CodeAnalyzerPrivate;
    std::unique_ptr<CodeAnalyzerPrivate> d;
    
    void initializeAnalyzer();
    void loadConfiguration();
    void saveConfiguration();
    void setupExternalAnalyzers();
    QString generateAnalysisId() const;
    void runAnalysisInThread(const QString& analysisId, const QString& sourcePath, const AnalysisConfig& config);
    void processAnalysisResults(const QString& analysisId, const QJsonObject& results);
    void calculateOverallScore(AnalysisReport& report);
    void generateRecommendations(AnalysisReport& report);
};

// Static analyzer for syntax and semantic analysis
class StaticAnalyzer : public QObject {
    Q_OBJECT

public:
    explicit StaticAnalyzer(QObject* parent = nullptr);
    ~StaticAnalyzer() override;

    // Analysis operations
    QList<AnalysisIssue> analyzeSyntax(const QString& filePath, const QString& code = "");
    QList<AnalysisIssue> analyzeSemantics(const QString& filePath, const QString& code = "");
    QList<AnalysisIssue> analyzeStyle(const QString& filePath, const QString& code = "");
    QList<AnalysisIssue> analyzeFile(const QString& filePath, const QSet<AnalysisType>& types);
    
    // Rule management
    void loadRules(const QString& rulesFile);
    void addRule(const QString& ruleId, const QJsonObject& rule);
    void removeRule(const QString& ruleId);
    QJsonObject getRule(const QString& ruleId) const;
    QStringList getRules() const;
    
    // Configuration
    void setStrictMode(bool strict);
    bool isStrictMode() const;
    void setLanguageStandard(const QString& standard);
    QString languageStandard() const;
    void setIncludePaths(const QStringList& paths);
    QStringList includePaths() const;

signals:
    void issueDetected(const AnalysisIssue& issue);
    void analysisCompleted(const QString& filePath, const QList<AnalysisIssue>& issues);

private:
    QMap<QString, QJsonObject> m_rules;
    QStringList m_includePaths;
    QString m_languageStandard;
    bool m_strictMode;
    
    QList<AnalysisIssue> parseCompilerOutput(const QString& output, const QString& filePath);
    QList<AnalysisIssue> runStyleChecker(const QString& filePath, const QString& code);
    QList<AnalysisIssue> checkNamingConventions(const QString& filePath, const QString& code);
    QList<AnalysisIssue> checkCodeStructure(const QString& filePath, const QString& code);
    AnalysisIssue createIssue(const QString& filePath, int line, IssueSeverity severity, const QString& message, const QString& rule = "");
};

// Quality metrics calculator
class QualityMetrics : public QObject {
    Q_OBJECT

public:
    explicit QualityMetrics(QObject* parent = nullptr);
    ~QualityMetrics() override;

    // Metric calculations
    QList<MetricResult> calculateMetrics(const QString& filePath, const QString& code = "");
    MetricResult calculateMetric(CodeMetricType type, const QString& filePath, const QString& code = "");
    double calculateCyclomaticComplexity(const QString& code);
    double calculateCognitiveComplexity(const QString& code);
    int calculateNestingDepth(const QString& code);
    int countLinesOfCode(const QString& code);
    double calculateDuplication(const QStringList& files);
    
    // Aggregated metrics
    QMap<CodeMetricType, double> calculateProjectMetrics(const QStringList& files);
    double calculateMaintainabilityIndex(const QString& filePath);
    QString calculateQualityGrade(const QList<MetricResult>& metrics);
    double calculateTechnicalDebt(const QList<MetricResult>& metrics);
    
    // Thresholds
    void setThreshold(CodeMetricType type, double threshold);
    double getThreshold(CodeMetricType type) const;
    bool exceedsThreshold(const MetricResult& metric) const;
    
    // Trend analysis
    void recordMetrics(const QString& pluginId, const QList<MetricResult>& metrics);
    QList<MetricResult> getMetricHistory(const QString& pluginId, CodeMetricType type) const;
    double calculateTrend(const QString& pluginId, CodeMetricType type) const;

signals:
    void metricCalculated(const MetricResult& metric);
    void metricsCompleted(const QString& filePath, const QList<MetricResult>& metrics);
    void thresholdExceeded(const MetricResult& metric);

private:
    QMap<CodeMetricType, double> m_thresholds;
    QMap<QString, QMap<CodeMetricType, QList<MetricResult>>> m_metricHistory;
    
    int countFunctions(const QString& code);
    int countClasses(const QString& code);
    int countParameters(const QString& functionSignature);
    QStringList extractFunctions(const QString& code);
    QStringList extractClasses(const QString& code);
    double calculateHalsteadComplexity(const QString& code);
    int calculateCouplingBetweenObjects(const QString& code);
    double calculateLackOfCohesion(const QString& code);
};

// Refactoring engine for code improvements
class RefactoringEngine : public QObject {
    Q_OBJECT

public:
    explicit RefactoringEngine(QObject* parent = nullptr);
    ~RefactoringEngine() override;

    // Refactoring operations
    QString extractMethod(const QString& code, int startLine, int endLine, const QString& methodName);
    QString renameVariable(const QString& code, const QString& oldName, const QString& newName);
    QString renameFunction(const QString& code, const QString& oldName, const QString& newName);
    QString inlineVariable(const QString& code, const QString& variableName);
    QString moveMethod(const QString& code, const QString& methodName, const QString& targetClass);
    QString extractClass(const QString& code, const QStringList& members, const QString& className);
    
    // Automatic fixes
    QString fixIssue(const QString& code, const AnalysisIssue& issue);
    QStringList getAvailableFixes(const AnalysisIssue& issue);
    QString applyQuickFix(const QString& code, const AnalysisIssue& issue, const QString& fixType);
    
    // Code generation
    QString generateGetter(const QString& memberName, const QString& type);
    QString generateSetter(const QString& memberName, const QString& type);
    QString generateConstructor(const QString& className, const QStringList& parameters);
    QString generateDestructor(const QString& className);
    QString generateToString(const QString& className, const QStringList& members);
    
    // Refactoring suggestions
    QStringList suggestRefactorings(const QString& code, const QList<AnalysisIssue>& issues);
    QStringList suggestImprovements(const QString& code, const QList<MetricResult>& metrics);
    QString generateRefactoringPlan(const QString& code, const QList<AnalysisIssue>& issues);

signals:
    void refactoringCompleted(const QString& originalCode, const QString& refactoredCode);
    void refactoringFailed(const QString& error);
    void suggestionGenerated(const QString& suggestion);

private:
    struct RefactoringContext {
        QString code;
        QStringList lines;
        QMap<QString, QStringList> functions;
        QMap<QString, QStringList> classes;
        QMap<QString, QString> variables;
    };
    
    RefactoringContext analyzeCode(const QString& code);
    QString reconstructCode(const RefactoringContext& context);
    bool isValidIdentifier(const QString& name);
    QStringList findReferences(const QString& code, const QString& identifier);
    QString generateUniqueIdentifier(const QString& base, const QStringList& existing);
};

// Security analyzer for vulnerability detection
class SecurityAnalyzer : public QObject {
    Q_OBJECT

public:
    explicit SecurityAnalyzer(QObject* parent = nullptr);
    ~SecurityAnalyzer() override;

    // Security analysis
    QList<AnalysisIssue> analyzeSecurityVulnerabilities(const QString& filePath, const QString& code = "");
    QList<AnalysisIssue> checkBufferOverflows(const QString& code);
    QList<AnalysisIssue> checkSqlInjection(const QString& code);
    QList<AnalysisIssue> checkCrossSiteScripting(const QString& code);
    QList<AnalysisIssue> checkInsecureCrypto(const QString& code);
    QList<AnalysisIssue> checkHardcodedSecrets(const QString& code);
    
    // Security rules
    void loadSecurityRules(const QString& rulesFile);
    void addSecurityRule(const QString& ruleId, const QJsonObject& rule);
    void removeSecurityRule(const QString& ruleId);
    QStringList getSecurityRules() const;
    
    // Vulnerability database
    void updateVulnerabilityDatabase();
    QStringList getKnownVulnerabilities(const QString& library) const;
    QString getVulnerabilityInfo(const QString& cveId) const;

signals:
    void vulnerabilityDetected(const AnalysisIssue& vulnerability);
    void securityAnalysisCompleted(const QString& filePath, const QList<AnalysisIssue>& vulnerabilities);
    void vulnerabilityDatabaseUpdated();

private:
    QMap<QString, QJsonObject> m_securityRules;
    QMap<QString, QStringList> m_vulnerabilityDatabase;
    
    QList<AnalysisIssue> runSecurityRule(const QString& ruleId, const QString& code, const QString& filePath);
    bool matchesPattern(const QString& code, const QString& pattern);
    AnalysisIssue createSecurityIssue(const QString& filePath, int line, const QString& vulnerability, const QString& description);
};

// Analysis widget for displaying results
class AnalysisWidget : public QWidget {
    Q_OBJECT

public:
    explicit AnalysisWidget(PluginCodeAnalyzer* analyzer, QWidget* parent = nullptr);
    ~AnalysisWidget() override;

    // Display management
    void showAnalysisReport(const QString& analysisId);
    void refreshResults();
    void clearResults();
    void setCurrentPlugin(const QString& pluginId);
    QString currentPlugin() const;
    
    // Filtering and sorting
    void setIssueFilter(IssueSeverity minSeverity);
    void setAnalysisTypeFilter(const QSet<AnalysisType>& types);
    void setFileFilter(const QString& filePattern);
    void sortIssues(const QString& column, Qt::SortOrder order);

signals:
    void issueSelected(const AnalysisIssue& issue);
    void issueDoubleClicked(const AnalysisIssue& issue);
    void fixIssueRequested(const QString& issueId);
    void dismissIssueRequested(const QString& issueId);
    void analysisRequested(const QString& pluginId);
    void navigateToCode(const QString& filePath, int lineNumber);

private slots:
    void onIssueItemClicked();
    void onIssueItemDoubleClicked();
    void onFixButtonClicked();
    void onDismissButtonClicked();
    void onAnalyzeButtonClicked();
    void onRefreshButtonClicked();
    void onFilterChanged();

private:
    PluginCodeAnalyzer* m_analyzer;
    QString m_currentPlugin;
    QString m_currentAnalysisId;
    
    // UI components
    QTabWidget* m_tabWidget;
    QTreeWidget* m_issuesTree;
    QTableWidget* m_metricsTable;
    QTextEdit* m_reportView;
    QWidget* m_summaryWidget;
    QComboBox* m_severityFilter;
    QComboBox* m_typeFilter;
    QLineEdit* m_fileFilter;
    
    void setupUI();
    void setupIssuesTab();
    void setupMetricsTab();
    void setupReportTab();
    void setupSummaryTab();
    void populateIssuesTree(const QList<AnalysisIssue>& issues);
    void populateMetricsTable(const QList<MetricResult>& metrics);
    void updateSummary(const AnalysisReport& report);
    void updateReportView(const AnalysisReport& report);
    QTreeWidgetItem* createIssueItem(const AnalysisIssue& issue);
    void addMetricRow(const MetricResult& metric);
    QString formatIssueTooltip(const AnalysisIssue& issue);
    QString generateHtmlReport(const AnalysisReport& report);
};
