// PluginLiveDebugger.h - Live Debugging System for Plugin Development
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QTabWidget>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTimer>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <QStack>
#include <memory>
#include "utils/PluginDebugger.h"

// Forward declarations
class PluginLiveDebugger;
class DebugSession;
class BreakpointManager;
class VariableInspector;
class CallStackAnalyzer;
class DebugConsole;
class DebuggerWidget;

// Note: DebugState enum is defined in utils/PluginDebugger.h

// Note: BreakpointType enum is defined in utils/PluginDebugger.h

// Variable types
enum class VariableType {
    Primitive,      // int, float, bool, etc.
    String,         // String/text
    Array,          // Array/list
    Object,         // Object/struct/class
    Pointer,        // Pointer/reference
    Function,       // Function pointer
    Unknown         // Unknown type
};

// Note: DebugEventType enum is defined in utils/PluginDebugger.h

// Note: Breakpoint struct is defined in utils/PluginDebugger.h

// Variable information
struct Variable {
    QString name;
    QString value;
    QString type;
    VariableType variableType;
    QString scope; // local, global, parameter, member
    bool isReadOnly;
    bool hasChildren;
    QList<Variable> children;
    QString address;
    int size;
    QJsonObject metadata;
    
    Variable() = default;
    Variable(const QString& n, const QString& v, const QString& t)
        : name(n), value(v), type(t), variableType(VariableType::Unknown),
          isReadOnly(false), hasChildren(false), size(0) {}
    
    QString getDisplayValue() const;
    QString getTypeString() const;
    bool isExpandable() const;
};

// Note: StackFrame struct is defined in utils/PluginDebugger.h

// Note: DebugEvent struct is defined in utils/PluginDebugger.h

// Debug session configuration
struct DebugConfig {
    QString pluginId;
    QString executable;
    QStringList arguments;
    QString workingDirectory;
    QMap<QString, QString> environment;
    QString debuggerPath;
    QStringList debuggerArgs;
    int port = 0; // 0 for auto-assign
    bool attachToProcess = false;
    int processId = 0;
    bool enableLogging = true;
    QString logLevel = "info";
    bool breakOnStart = false;
    bool breakOnException = true;
    QStringList sourceDirectories;
    QStringList libraryPaths;
    int timeout = 30000; // 30 seconds
    
    DebugConfig() = default;
};

// Main live debugger
class PluginLiveDebugger : public QObject {
    Q_OBJECT

public:
    explicit PluginLiveDebugger(QObject* parent = nullptr);
    ~PluginLiveDebugger() override;

    // Debug session management
    QString startDebugSession(const QString& pluginId, const DebugConfig& config);
    void stopDebugSession(const QString& sessionId);
    void pauseDebugSession(const QString& sessionId);
    void resumeDebugSession(const QString& sessionId);
    bool isDebugging(const QString& pluginId) const;
    QStringList getActiveSessionIds() const;
    DebugSession* getDebugSession(const QString& sessionId) const;
    
    // Execution control
    void continueExecution(const QString& sessionId);
    void stepOver(const QString& sessionId);
    void stepInto(const QString& sessionId);
    void stepOut(const QString& sessionId);
    void runToCursor(const QString& sessionId, const QString& filePath, int lineNumber);
    void restart(const QString& sessionId);
    void terminate(const QString& sessionId);
    
    // Breakpoint management
    QString addBreakpoint(const QString& pluginId, const QString& filePath, int lineNumber, BreakpointType type = BreakpointType::Line);
    void removeBreakpoint(const QString& breakpointId);
    void enableBreakpoint(const QString& breakpointId, bool enable);
    void setBreakpointCondition(const QString& breakpointId, const QString& condition);
    Breakpoint getBreakpoint(const QString& breakpointId) const;
    QList<Breakpoint> getBreakpoints(const QString& pluginId = "") const;
    void clearAllBreakpoints(const QString& pluginId = "");
    
    // Variable inspection
    QList<Variable> getLocalVariables(const QString& sessionId) const;
    QList<Variable> getGlobalVariables(const QString& sessionId) const;
    Variable getVariable(const QString& sessionId, const QString& variableName) const;
    void setVariableValue(const QString& sessionId, const QString& variableName, const QString& value);
    QList<Variable> expandVariable(const QString& sessionId, const QString& variableName) const;
    QString evaluateExpression(const QString& sessionId, const QString& expression) const;
    
    // Call stack analysis
    QList<StackFrame> getCallStack(const QString& sessionId) const;
    void selectStackFrame(const QString& sessionId, int frameLevel);
    int getCurrentStackFrame(const QString& sessionId) const;
    
    // Debug output and logging
    QStringList getDebugOutput(const QString& sessionId) const;
    void sendDebugCommand(const QString& sessionId, const QString& command);
    void clearDebugOutput(const QString& sessionId);
    
    // Configuration
    void setDebugConfig(const QString& pluginId, const DebugConfig& config);
    DebugConfig getDebugConfig(const QString& pluginId) const;
    void setDefaultDebugger(const QString& debuggerPath);
    QString defaultDebugger() const;
    
    // Debug events
    QList<DebugEvent> getDebugEvents(const QString& sessionId = "") const;
    void clearDebugEvents(const QString& sessionId = "");

signals:
    void debugSessionStarted(const QString& sessionId, const QString& pluginId);
    void debugSessionStopped(const QString& sessionId);
    void debugStateChanged(const QString& sessionId, DebugState oldState, DebugState newState);
    void breakpointHit(const QString& sessionId, const QString& breakpointId);
    void breakpointAdded(const QString& breakpointId);
    void breakpointRemoved(const QString& breakpointId);
    void variableChanged(const QString& sessionId, const Variable& variable);
    void callStackChanged(const QString& sessionId);
    void debugOutputReceived(const QString& sessionId, const QString& output);
    void debugEventOccurred(const DebugEvent& event);
    void debugError(const QString& sessionId, const QString& error);

public slots:
    void showDebuggerWidget();
    void showDebuggerWidget(const QString& pluginId);

private slots:
    void onDebugProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDebugProcessError(QProcess::ProcessError error);
    void onDebuggerConnected();
    void onDebuggerDisconnected();
    void onDebuggerDataReceived();

private:
    struct LiveDebuggerPrivate;
    std::unique_ptr<LiveDebuggerPrivate> d;
    
    void initializeDebugger();
    void loadConfiguration();
    void saveConfiguration();
    void setupDebuggerCommunication();
    void processDebuggerMessage(const QString& sessionId, const QJsonObject& message);
    void handleBreakpointHit(const QString& sessionId, const QJsonObject& data);
    void handleVariableUpdate(const QString& sessionId, const QJsonObject& data);
    void handleCallStackUpdate(const QString& sessionId, const QJsonObject& data);
    QString generateSessionId() const;
    void logDebugEvent(const DebugEvent& event);
};

// Debug session for managing individual debugging sessions
class DebugSession : public QObject {
    Q_OBJECT

public:
    explicit DebugSession(const QString& sessionId, const QString& pluginId, const DebugConfig& config, QObject* parent = nullptr);
    ~DebugSession() override;

    // Session information
    QString sessionId() const;
    QString pluginId() const;
    DebugConfig configuration() const;
    DebugState state() const;
    QDateTime startTime() const;
    QDateTime endTime() const;
    
    // Process management
    bool start();
    void stop();
    void pause();
    void resume();
    bool isRunning() const;
    qint64 processId() const;
    
    // Communication
    void sendCommand(const QString& command);
    void sendCommand(const QJsonObject& command);
    QJsonObject receiveResponse(int timeout = 5000);
    bool isConnected() const;
    
    // State management
    void setState(DebugState newState);
    void setCurrentFrame(int frameLevel);
    int currentFrame() const;
    void setCurrentThread(const QString& threadId);
    QString currentThread() const;

signals:
    void stateChanged(DebugState oldState, DebugState newState);
    void processStarted();
    void processStopped();
    void commandSent(const QString& command);
    void responseReceived(const QJsonObject& response);
    void connectionEstablished();
    void connectionLost();

private slots:
    void onProcessStarted();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketDataReady();

private:
    QString m_sessionId;
    QString m_pluginId;
    DebugConfig m_config;
    DebugState m_state;
    QDateTime m_startTime;
    QDateTime m_endTime;
    QProcess* m_debugProcess;
    QTcpSocket* m_debugSocket;
    QLocalSocket* m_localSocket;
    int m_currentFrame;
    QString m_currentThread;
    QQueue<QJsonObject> m_responseQueue;
    QMutex m_responseMutex;
    QWaitCondition m_responseCondition;
    
    void setupProcess();
    void setupCommunication();
    void processDebuggerOutput();
    QJsonObject parseDebuggerMessage(const QString& message);
};

// Breakpoint manager for handling breakpoints
class BreakpointManager : public QObject {
    Q_OBJECT

public:
    explicit BreakpointManager(QObject* parent = nullptr);
    ~BreakpointManager() override;

    // Breakpoint operations
    QString addBreakpoint(const Breakpoint& breakpoint);
    void removeBreakpoint(const QString& breakpointId);
    void updateBreakpoint(const Breakpoint& breakpoint);
    Breakpoint getBreakpoint(const QString& breakpointId) const;
    QList<Breakpoint> getAllBreakpoints() const;
    QList<Breakpoint> getBreakpoints(const QString& pluginId) const;
    QList<Breakpoint> getBreakpoints(const QString& filePath, int lineNumber) const;
    
    // Breakpoint state
    void enableBreakpoint(const QString& breakpointId, bool enable);
    void setCondition(const QString& breakpointId, const QString& condition);
    void setIgnoreCount(const QString& breakpointId, int count);
    void setLogMessage(const QString& breakpointId, const QString& message);
    void recordHit(const QString& breakpointId);
    
    // Bulk operations
    void enableAllBreakpoints(bool enable);
    void clearAllBreakpoints();
    void clearBreakpoints(const QString& pluginId);
    void clearBreakpointsInFile(const QString& filePath);
    
    // Persistence
    void saveBreakpoints(const QString& filePath = "");
    void loadBreakpoints(const QString& filePath = "");
    void exportBreakpoints(const QString& filePath) const;
    void importBreakpoints(const QString& filePath);

signals:
    void breakpointAdded(const QString& breakpointId);
    void breakpointRemoved(const QString& breakpointId);
    void breakpointUpdated(const QString& breakpointId);
    void breakpointHit(const QString& breakpointId);

private:
    QMap<QString, Breakpoint> m_breakpoints;
    QString m_breakpointsFile;
    
    QString generateBreakpointId() const;
    void ensureBreakpointsDirectory();
    QString getDefaultBreakpointsFile() const;
};

// Variable inspector for examining variables
class VariableInspector : public QObject {
    Q_OBJECT

public:
    explicit VariableInspector(QObject* parent = nullptr);
    ~VariableInspector() override;

    // Variable inspection
    void setDebugSession(DebugSession* session);
    DebugSession* debugSession() const;
    QList<Variable> getVariables(const QString& scope = "local") const;
    Variable getVariable(const QString& name) const;
    QList<Variable> expandVariable(const QString& name) const;
    
    // Variable modification
    bool setVariableValue(const QString& name, const QString& value);
    bool canModifyVariable(const QString& name) const;
    
    // Expression evaluation
    QString evaluateExpression(const QString& expression) const;
    Variable evaluateToVariable(const QString& expression) const;
    bool isValidExpression(const QString& expression) const;
    
    // Watch variables
    void addWatchVariable(const QString& expression);
    void removeWatchVariable(const QString& expression);
    QStringList getWatchVariables() const;
    QList<Variable> getWatchValues() const;
    void clearWatchVariables();
    
    // Variable formatting
    void setVariableFormat(const QString& name, const QString& format);
    QString getVariableFormat(const QString& name) const;
    QStringList getAvailableFormats() const;

signals:
    void variablesUpdated();
    void variableChanged(const Variable& variable);
    void watchVariableAdded(const QString& expression);
    void watchVariableRemoved(const QString& expression);
    void expressionEvaluated(const QString& expression, const QString& result);

private slots:
    void onDebugStateChanged(DebugState state);
    void onStackFrameChanged(int frameLevel);

private:
    DebugSession* m_debugSession;
    QMap<QString, Variable> m_variables;
    QStringList m_watchVariables;
    QMap<QString, QString> m_variableFormats;
    
    void refreshVariables();
    void parseVariableData(const QJsonObject& data);
    Variable parseVariable(const QJsonObject& varData) const;
    VariableType determineVariableType(const QString& type) const;
};

// Call stack analyzer for examining the call stack
class CallStackAnalyzer : public QObject {
    Q_OBJECT

public:
    explicit CallStackAnalyzer(QObject* parent = nullptr);
    ~CallStackAnalyzer() override;

    // Call stack analysis
    void setDebugSession(DebugSession* session);
    DebugSession* debugSession() const;
    QList<StackFrame> getCallStack() const;
    StackFrame getCurrentFrame() const;
    StackFrame getFrame(int level) const;
    int getCurrentFrameLevel() const;
    
    // Frame navigation
    void selectFrame(int level);
    void moveUp();
    void moveDown();
    bool canMoveUp() const;
    bool canMoveDown() const;
    
    // Frame analysis
    QStringList getFrameFunctions() const;
    QStringList getFrameModules() const;
    QMap<QString, int> getFunctionCallCounts() const;
    QString getCallPath() const;
    
    // Source navigation
    bool hasSourceInfo(int level) const;
    QString getSourceFile(int level) const;
    int getSourceLine(int level) const;
    void navigateToFrame(int level);

signals:
    void callStackUpdated();
    void currentFrameChanged(int level);
    void frameSelected(int level);
    void sourceNavigationRequested(const QString& filePath, int lineNumber);

private slots:
    void onDebugStateChanged(DebugState state);

private:
    DebugSession* m_debugSession;
    QList<StackFrame> m_callStack;
    int m_currentFrameLevel;
    
    void refreshCallStack();
    void parseCallStackData(const QJsonObject& data);
    StackFrame parseStackFrame(const QJsonObject& frameData) const;
};

// Debug console for interactive debugging
class DebugConsole : public QWidget {
    Q_OBJECT

public:
    explicit DebugConsole(QWidget* parent = nullptr);
    ~DebugConsole() override;

    // Console management
    void setDebugSession(DebugSession* session);
    DebugSession* debugSession() const;
    void clear();
    void appendOutput(const QString& text, const QString& category = "output");
    void appendError(const QString& text);
    void appendCommand(const QString& command);
    
    // Command execution
    void executeCommand(const QString& command);
    void setCommandHistory(const QStringList& history);
    QStringList commandHistory() const;
    void clearHistory();
    
    // Console configuration
    void setMaxLines(int maxLines);
    int maxLines() const;
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;
    void setShowTimestamps(bool show);
    bool showTimestamps() const;

signals:
    void commandExecuted(const QString& command);
    void outputReceived(const QString& output);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onReturnPressed();
    void onDebugOutputReceived(const QString& output);

private:
    DebugSession* m_debugSession;
    QTextEdit* m_outputEdit;
    QLineEdit* m_commandEdit;
    QStringList m_commandHistory;
    int m_historyIndex;
    int m_maxLines;
    bool m_autoScroll;
    bool m_showTimestamps;
    
    void setupUI();
    void addToHistory(const QString& command);
    void navigateHistory(int direction);
    void formatOutput(const QString& text, const QString& category);
    void trimOutput();
};
