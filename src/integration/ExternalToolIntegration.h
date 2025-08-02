// ExternalToolIntegration.h - Integration with External Development Tools and IDEs
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
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QSplitter>
#include <QTabWidget>
#include <QTimer>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QFileSystemWatcher>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QRegularExpression>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <memory>

// Forward declarations
class ExternalToolManager;
class IDEIntegration;
class BuildSystemIntegration;
class VersionControlIntegration;
class DebuggerIntegration;
class ToolConfiguration;
class IntegrationWidget;

// Tool types
enum class ToolType {
    IDE,            // Integrated Development Environment
    Editor,         // Text/Code Editor
    Compiler,       // Compiler/Build Tool
    Debugger,       // Debugger
    Profiler,       // Performance Profiler
    Analyzer,       // Static Code Analyzer
    VersionControl, // Version Control System
    BuildSystem,    // Build System (CMake, Make, etc.)
    PackageManager, // Package Manager
    Documentation,  // Documentation Generator
    Testing,        // Testing Framework
    Deployment,     // Deployment Tool
    Custom          // Custom Tool
};

// Integration protocols
enum class IntegrationProtocol {
    CommandLine,    // Command line interface
    API,            // REST/HTTP API
    Plugin,         // Plugin/Extension
    LSP,            // Language Server Protocol
    DAP,            // Debug Adapter Protocol
    FileSystem,     // File system monitoring
    Socket,         // Socket communication
    Pipe,           // Named pipe
    SharedMemory,   // Shared memory
    Registry,       // Windows registry
    Custom          // Custom protocol
};

// Tool status
enum class ToolStatus {
    Unknown,        // Status unknown
    Available,      // Tool is available
    Running,        // Tool is running
    Busy,           // Tool is busy
    Error,          // Tool has error
    NotInstalled,   // Tool not installed
    Incompatible,   // Tool version incompatible
    Disabled        // Tool disabled
};

// External tool configuration
struct ExternalTool {
    QString toolId;
    QString name;
    QString description;
    ToolType type;
    QString version;
    QString executablePath;
    QStringList arguments;
    QString workingDirectory;
    QMap<QString, QString> environment;
    IntegrationProtocol protocol;
    QString configurationFile;
    QJsonObject settings;
    bool isEnabled;
    bool autoDetect;
    QDateTime lastUsed;
    ToolStatus status;
    QString statusMessage;
    QJsonObject metadata;
    
    ExternalTool() = default;
    ExternalTool(const QString& id, const QString& n, ToolType t)
        : toolId(id), name(n), type(t), protocol(IntegrationProtocol::CommandLine),
          isEnabled(true), autoDetect(true), status(ToolStatus::Unknown) {}
    
    QString getTypeString() const;
    QString getProtocolString() const;
    QString getStatusString() const;
    bool isAvailable() const;
    QString getFullCommand() const;
};

// Tool operation result
struct ToolOperationResult {
    QString operationId;
    QString toolId;
    QString operation;
    bool success;
    int exitCode;
    QString output;
    QString errorOutput;
    QDateTime startTime;
    QDateTime endTime;
    int duration; // milliseconds
    QJsonObject metadata;
    
    ToolOperationResult() = default;
    ToolOperationResult(const QString& tool, const QString& op)
        : toolId(tool), operation(op), success(false), exitCode(-1),
          startTime(QDateTime::currentDateTime()) {
        operationId = generateOperationId();
    }
    
    void complete(bool result, int code = 0, const QString& out = "", const QString& err = "");
    int getDuration() const;
    QString getSummary() const;
    
private:
    QString generateOperationId() const;
};

// IDE project information
struct IDEProject {
    QString projectId;
    QString name;
    QString path;
    QString ideType; // Visual Studio, Qt Creator, CLion, etc.
    QString projectFile;
    QStringList sourceFiles;
    QStringList headerFiles;
    QStringList resourceFiles;
    QStringList configurations; // Debug, Release, etc.
    QString activeConfiguration;
    QJsonObject buildSettings;
    QJsonObject debugSettings;
    QDateTime lastModified;
    QJsonObject metadata;
    
    IDEProject() = default;
    IDEProject(const QString& id, const QString& n, const QString& p)
        : projectId(id), name(n), path(p), lastModified(QDateTime::currentDateTime()) {}
    
    bool isValid() const;
    QStringList getAllFiles() const;
    QString getProjectDirectory() const;
};

// Main external tool manager
class ExternalToolManager : public QObject {
    Q_OBJECT

public:
    explicit ExternalToolManager(QObject* parent = nullptr);
    ~ExternalToolManager() override;

    // Tool management
    void registerTool(const ExternalTool& tool);
    void unregisterTool(const QString& toolId);
    void updateTool(const ExternalTool& tool);
    ExternalTool getTool(const QString& toolId) const;
    QList<ExternalTool> getTools(ToolType type = ToolType::Custom) const;
    QStringList getToolIds(ToolType type = ToolType::Custom) const;
    
    // Tool detection
    void autoDetectTools();
    void detectTool(const QString& toolId);
    QStringList detectInstalledTools(ToolType type) const;
    QString findToolExecutable(const QString& toolName) const;
    bool isToolInstalled(const QString& toolId) const;
    
    // Tool operations
    QString executeTool(const QString& toolId, const QStringList& arguments = QStringList(), const QString& workingDir = "");
    QString executeToolAsync(const QString& toolId, const QStringList& arguments = QStringList(), const QString& workingDir = "");
    void cancelOperation(const QString& operationId);
    ToolOperationResult getOperationResult(const QString& operationId) const;
    QList<ToolOperationResult> getOperationHistory(const QString& toolId = "") const;
    
    // Tool status
    void updateToolStatus(const QString& toolId);
    ToolStatus getToolStatus(const QString& toolId) const;
    QString getToolStatusMessage(const QString& toolId) const;
    void enableTool(const QString& toolId, bool enable);
    bool isToolEnabled(const QString& toolId) const;
    
    // Integration management
    void enableIntegration(const QString& toolId, bool enable);
    bool isIntegrationEnabled(const QString& toolId) const;
    void configureIntegration(const QString& toolId, const QJsonObject& config);
    QJsonObject getIntegrationConfig(const QString& toolId) const;
    
    // Project integration
    void setActiveProject(const QString& projectPath);
    QString activeProject() const;
    void syncWithProject(const QString& toolId);
    void importProjectFromTool(const QString& toolId, const QString& projectPath);
    void exportProjectToTool(const QString& toolId, const QString& projectPath);
    
    // Configuration
    void setToolsDirectory(const QString& directory);
    QString toolsDirectory() const;
    void setConfigurationDirectory(const QString& directory);
    QString configurationDirectory() const;
    void saveConfiguration();
    void loadConfiguration();

signals:
    void toolRegistered(const QString& toolId);
    void toolUnregistered(const QString& toolId);
    void toolStatusChanged(const QString& toolId, ToolStatus status);
    void toolDetected(const QString& toolId, const QString& path);
    void operationStarted(const QString& operationId, const QString& toolId);
    void operationCompleted(const QString& operationId, bool success);
    void operationFailed(const QString& operationId, const QString& error);
    void integrationEnabled(const QString& toolId);
    void integrationDisabled(const QString& toolId);
    void projectChanged(const QString& projectPath);

public slots:
    void refreshToolStatus();
    void showIntegrationWidget();

private slots:
    void onToolProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onToolProcessError(QProcess::ProcessError error);
    void onFileSystemChanged(const QString& path);

private:
    struct ExternalToolManagerPrivate;
    std::unique_ptr<ExternalToolManagerPrivate> d;
    
    void initializeManager();
    void loadDefaultTools();
    void setupFileSystemWatcher();
    void createDefaultConfigurations();
    QString generateOperationId() const;
    void logOperation(const ToolOperationResult& result);
    void updateToolFromProcess(const QString& toolId, QProcess* process);
};

// IDE integration for popular IDEs
class IDEIntegration : public QObject {
    Q_OBJECT

public:
    explicit IDEIntegration(QObject* parent = nullptr);
    ~IDEIntegration() override;

    // IDE support
    void addIDESupport(const QString& ideType, const QString& executablePath);
    void removeIDESupport(const QString& ideType);
    QStringList getSupportedIDEs() const;
    bool isIDESupported(const QString& ideType) const;
    QString getIDEExecutable(const QString& ideType) const;
    
    // Project operations
    IDEProject loadProject(const QString& projectPath, const QString& ideType = "");
    void saveProject(const IDEProject& project);
    void openProjectInIDE(const QString& projectPath, const QString& ideType = "");
    void createProjectInIDE(const QString& projectName, const QString& projectPath, const QString& ideType);
    
    // File operations
    void openFileInIDE(const QString& filePath, int lineNumber = -1, const QString& ideType = "");
    void createFileInIDE(const QString& filePath, const QString& templateType = "", const QString& ideType = "");
    void addFileToProject(const QString& projectPath, const QString& filePath);
    void removeFileFromProject(const QString& projectPath, const QString& filePath);
    
    // Build operations
    void buildProject(const QString& projectPath, const QString& configuration = "");
    void cleanProject(const QString& projectPath);
    void rebuildProject(const QString& projectPath, const QString& configuration = "");
    void runProject(const QString& projectPath, const QString& configuration = "");
    
    // Debug operations
    void startDebugging(const QString& projectPath, const QString& configuration = "");
    void attachDebugger(int processId);
    void setBreakpoint(const QString& filePath, int lineNumber);
    void removeBreakpoint(const QString& filePath, int lineNumber);
    
    // Configuration
    void setIDEConfiguration(const QString& ideType, const QJsonObject& config);
    QJsonObject getIDEConfiguration(const QString& ideType) const;
    void syncIDESettings(const QString& ideType);

signals:
    void ideDetected(const QString& ideType, const QString& path);
    void projectLoaded(const IDEProject& project);
    void projectOpened(const QString& projectPath, const QString& ideType);
    void fileOpened(const QString& filePath, const QString& ideType);
    void buildStarted(const QString& projectPath);
    void buildCompleted(const QString& projectPath, bool success);
    void debuggingStarted(const QString& projectPath);
    void debuggingStopped(const QString& projectPath);

private:
    QMap<QString, QString> m_ideExecutables;
    QMap<QString, QJsonObject> m_ideConfigurations;
    QMap<QString, IDEProject> m_loadedProjects;
    
    IDEProject parseVisualStudioProject(const QString& projectPath);
    IDEProject parseQtCreatorProject(const QString& projectPath);
    IDEProject parseCLionProject(const QString& projectPath);
    IDEProject parseCodeBlocksProject(const QString& projectPath);
    void saveVisualStudioProject(const IDEProject& project);
    void saveQtCreatorProject(const IDEProject& project);
    QString detectIDEType(const QString& projectPath);
    QStringList findIDEExecutables();
};

// Build system integration
class BuildSystemIntegration : public QObject {
    Q_OBJECT

public:
    explicit BuildSystemIntegration(QObject* parent = nullptr);
    ~BuildSystemIntegration() override;

    // Build system support
    void addBuildSystemSupport(const QString& buildSystem, const QString& executablePath);
    void removeBuildSystemSupport(const QString& buildSystem);
    QStringList getSupportedBuildSystems() const;
    bool isBuildSystemSupported(const QString& buildSystem) const;
    
    // Project configuration
    void generateBuildFiles(const QString& projectPath, const QString& buildSystem);
    void configureBuild(const QString& projectPath, const QJsonObject& configuration);
    QJsonObject getBuildConfiguration(const QString& projectPath) const;
    void setBuildDirectory(const QString& projectPath, const QString& buildDirectory);
    QString getBuildDirectory(const QString& projectPath) const;
    
    // Build operations
    void buildProject(const QString& projectPath, const QString& target = "");
    void cleanProject(const QString& projectPath);
    void installProject(const QString& projectPath);
    void testProject(const QString& projectPath);
    void packageProject(const QString& projectPath);
    
    // Target management
    QStringList getAvailableTargets(const QString& projectPath) const;
    void addTarget(const QString& projectPath, const QString& targetName, const QJsonObject& targetConfig);
    void removeTarget(const QString& projectPath, const QString& targetName);
    void setActiveTarget(const QString& projectPath, const QString& targetName);
    QString getActiveTarget(const QString& projectPath) const;
    
    // Dependency management
    void addDependency(const QString& projectPath, const QString& dependency);
    void removeDependency(const QString& projectPath, const QString& dependency);
    QStringList getDependencies(const QString& projectPath) const;
    void updateDependencies(const QString& projectPath);

signals:
    void buildSystemDetected(const QString& buildSystem, const QString& path);
    void buildFilesGenerated(const QString& projectPath, const QString& buildSystem);
    void buildStarted(const QString& projectPath, const QString& target);
    void buildCompleted(const QString& projectPath, bool success, const QString& output);
    void buildFailed(const QString& projectPath, const QString& error);
    void targetAdded(const QString& projectPath, const QString& targetName);
    void targetRemoved(const QString& projectPath, const QString& targetName);
    void dependencyAdded(const QString& projectPath, const QString& dependency);

private:
    QMap<QString, QString> m_buildSystemExecutables;
    QMap<QString, QJsonObject> m_projectConfigurations;
    QMap<QString, QString> m_buildDirectories;
    QMap<QString, QString> m_activeTargets;
    
    void generateCMakeFiles(const QString& projectPath, const QJsonObject& config);
    void generateMakefiles(const QString& projectPath, const QJsonObject& config);
    void generateQMakeFiles(const QString& projectPath, const QJsonObject& config);
    QString detectBuildSystem(const QString& projectPath);
    QStringList findBuildSystemExecutables();
};

// Version control integration
class VersionControlIntegration : public QObject {
    Q_OBJECT

public:
    explicit VersionControlIntegration(QObject* parent = nullptr);
    ~VersionControlIntegration() override;

    // VCS support
    void addVCSSupport(const QString& vcsType, const QString& executablePath);
    void removeVCSSupport(const QString& vcsType);
    QStringList getSupportedVCS() const;
    bool isVCSSupported(const QString& vcsType) const;
    
    // Repository operations
    void initializeRepository(const QString& projectPath, const QString& vcsType);
    void cloneRepository(const QString& url, const QString& localPath, const QString& vcsType = "");
    QString getRepositoryStatus(const QString& projectPath);
    QString getRepositoryInfo(const QString& projectPath);
    
    // File operations
    void addFiles(const QString& projectPath, const QStringList& files);
    void removeFiles(const QString& projectPath, const QStringList& files);
    void commitChanges(const QString& projectPath, const QString& message, const QStringList& files = QStringList());
    void revertChanges(const QString& projectPath, const QStringList& files = QStringList());
    
    // Branch operations
    QStringList getBranches(const QString& projectPath);
    QString getCurrentBranch(const QString& projectPath);
    void createBranch(const QString& projectPath, const QString& branchName);
    void switchBranch(const QString& projectPath, const QString& branchName);
    void mergeBranch(const QString& projectPath, const QString& branchName);
    void deleteBranch(const QString& projectPath, const QString& branchName);
    
    // Remote operations
    void pushChanges(const QString& projectPath, const QString& remote = "origin", const QString& branch = "");
    void pullChanges(const QString& projectPath, const QString& remote = "origin", const QString& branch = "");
    void fetchChanges(const QString& projectPath, const QString& remote = "origin");
    QStringList getRemotes(const QString& projectPath);
    void addRemote(const QString& projectPath, const QString& name, const QString& url);
    void removeRemote(const QString& projectPath, const QString& name);
    
    // History operations
    QStringList getCommitHistory(const QString& projectPath, int maxCommits = 100);
    QString getCommitDetails(const QString& projectPath, const QString& commitHash);
    QString getDiff(const QString& projectPath, const QString& file = "");
    QString getBlame(const QString& projectPath, const QString& file);

signals:
    void vcsDetected(const QString& vcsType, const QString& path);
    void repositoryInitialized(const QString& projectPath, const QString& vcsType);
    void repositoryCloned(const QString& url, const QString& localPath);
    void filesAdded(const QString& projectPath, const QStringList& files);
    void changesCommitted(const QString& projectPath, const QString& commitHash);
    void branchCreated(const QString& projectPath, const QString& branchName);
    void branchSwitched(const QString& projectPath, const QString& branchName);
    void changesPushed(const QString& projectPath, const QString& remote);
    void changesPulled(const QString& projectPath, const QString& remote);

private:
    QMap<QString, QString> m_vcsExecutables;
    QMap<QString, QString> m_repositoryTypes; // projectPath -> vcsType
    
    QString detectVCSType(const QString& projectPath);
    QStringList findVCSExecutables();
    QString executeVCSCommand(const QString& projectPath, const QString& vcsType, const QStringList& arguments);
};

// Integration widget for managing external tools
class IntegrationWidget : public QWidget {
    Q_OBJECT

public:
    explicit IntegrationWidget(ExternalToolManager* manager, QWidget* parent = nullptr);
    ~IntegrationWidget() override;

    // Display management
    void refreshTools();
    void refreshOperations();
    void refreshProjects();
    void showToolDetails(const QString& toolId);
    void showOperationDetails(const QString& operationId);
    
    // Tool management
    void addTool();
    void editTool(const QString& toolId);
    void removeTool(const QString& toolId);
    void detectTools();

signals:
    void toolSelected(const QString& toolId);
    void operationSelected(const QString& operationId);
    void projectSelected(const QString& projectPath);
    void toolExecutionRequested(const QString& toolId);
    void toolConfigurationRequested(const QString& toolId);
    void integrationToggleRequested(const QString& toolId, bool enable);

private slots:
    void onToolItemClicked();
    void onOperationItemClicked();
    void onProjectItemClicked();
    void onAddToolClicked();
    void onEditToolClicked();
    void onRemoveToolClicked();
    void onDetectToolsClicked();
    void onExecuteToolClicked();
    void onConfigureToolClicked();
    void onToggleIntegrationClicked();
    void onRefreshClicked();

private:
    ExternalToolManager* m_manager;
    
    // UI components
    QTabWidget* m_tabWidget;
    QTreeWidget* m_toolsTree;
    QTableWidget* m_operationsTable;
    QListWidget* m_projectsList;
    QTextEdit* m_detailsView;
    QSplitter* m_splitter;
    
    void setupUI();
    void setupToolsTab();
    void setupOperationsTab();
    void setupProjectsTab();
    void populateToolsTree();
    void populateOperationsTable();
    void populateProjectsList();
    void updateDetailsView(const QString& content);
    QTreeWidgetItem* createToolItem(const ExternalTool& tool);
    void addOperationRow(const ToolOperationResult& operation);
    QString formatToolDetails(const ExternalTool& tool);
    QString formatOperationDetails(const ToolOperationResult& operation);
};

// Tool configuration dialog
class ToolConfigurationDialog : public QDialog {
    Q_OBJECT

public:
    explicit ToolConfigurationDialog(const ExternalTool& tool = ExternalTool(), QWidget* parent = nullptr);
    ~ToolConfigurationDialog() override;

    // Tool configuration
    ExternalTool getTool() const;
    void setTool(const ExternalTool& tool);

public slots:
    void accept() override;
    void reject() override;

signals:
    void toolConfigured(const ExternalTool& tool);

private slots:
    void onBrowseExecutable();
    void onBrowseWorkingDirectory();
    void onBrowseConfigurationFile();
    void onTestTool();
    void onDetectTool();
    void onAddArgument();
    void onRemoveArgument();
    void onAddEnvironmentVariable();
    void onRemoveEnvironmentVariable();

private:
    ExternalTool m_tool;
    
    // UI components
    QLineEdit* m_nameEdit;
    QTextEdit* m_descriptionEdit;
    QComboBox* m_typeCombo;
    QLineEdit* m_executableEdit;
    QListWidget* m_argumentsList;
    QLineEdit* m_workingDirectoryEdit;
    QTableWidget* m_environmentTable;
    QComboBox* m_protocolCombo;
    QLineEdit* m_configurationFileEdit;
    QCheckBox* m_enabledCheck;
    QCheckBox* m_autoDetectCheck;
    
    void setupUI();
    void setupGeneralTab();
    void setupExecutionTab();
    void setupEnvironmentTab();
    void setupAdvancedTab();
    void updateUIFromTool();
    void updateToolFromUI();
    void validateTool();
    void populateArgumentsList();
    void populateEnvironmentTable();
};
