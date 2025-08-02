// PluginDebugger.h - Built-in Plugin Debugging Tools and Breakpoint Support
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QMutex>
#include <QThread>
#include <QDebug>
#include <memory>

// Forward declarations
class PluginDebugSession;
class BreakpointManager;
class VariableInspector;
class CallStackViewer;
class DebugConsole;
class PluginProfiler;

// Debug session states
enum class DebugState {
    NotStarted,
    Starting,
    Running,
    Paused,
    Stepping,
    Stopping,
    Stopped,
    Error
};

// Breakpoint types
enum class BreakpointType {
    Line,           // Line breakpoint
    Function,       // Function breakpoint
    Exception,      // Exception breakpoint
    Conditional,    // Conditional breakpoint
    Watchpoint      // Data watchpoint
};

// Debug event types
enum class DebugEventType {
    BreakpointHit,
    ExceptionThrown,
    StepCompleted,
    VariableChanged,
    FunctionEntered,
    FunctionExited,
    PluginLoaded,
    PluginUnloaded
};

// Breakpoint information
struct Breakpoint {
    QString id;
    BreakpointType type;
    QString file;
    int line = -1;
    QString function;
    QString condition;
    bool enabled = true;
    int hitCount = 0;
    QString expression; // For watchpoints
    
    Breakpoint() = default;
    Breakpoint(BreakpointType t, const QString& f, int l)
        : id(QUuid::createUuid().toString(QUuid::WithoutBraces)), type(t), file(f), line(l) {}
};

// Debug event information
struct DebugEvent {
    DebugEventType type;
    QString pluginName;
    QString file;
    int line = -1;
    QString function;
    QString message;
    QJsonObject data;
    QDateTime timestamp;
    
    DebugEvent() : timestamp(QDateTime::currentDateTime()) {}
    DebugEvent(DebugEventType t, const QString& msg)
        : type(t), message(msg), timestamp(QDateTime::currentDateTime()) {}
};

// Variable information for inspection
struct VariableInfo {
    QString name;
    QString type;
    QString value;
    QString scope; // local, member, global
    bool isPointer = false;
    bool isReference = false;
    QList<VariableInfo> children; // For complex types
    
    VariableInfo() = default;
    VariableInfo(const QString& n, const QString& t, const QString& v)
        : name(n), type(t), value(v) {}
};

// Call stack frame information
struct StackFrame {
    QString function;
    QString file;
    int line = -1;
    QString module;
    QMap<QString, QString> locals;
    
    StackFrame() = default;
    StackFrame(const QString& func, const QString& f, int l)
        : function(func), file(f), line(l) {}
};

// Main plugin debugger class
class PluginDebugger : public QObject {
    Q_OBJECT

public:
    explicit PluginDebugger(QObject* parent = nullptr);
    ~PluginDebugger() override;

    // Debug session management
    bool startDebugging(const QString& pluginPath, const QStringList& arguments = {});
    void stopDebugging();
    void pauseDebugging();
    void resumeDebugging();
    void stepInto();
    void stepOver();
    void stepOut();
    void runToCursor(const QString& file, int line);
    
    // Breakpoint management
    QString addBreakpoint(BreakpointType type, const QString& file, int line, const QString& condition = "");
    QString addFunctionBreakpoint(const QString& function, const QString& condition = "");
    QString addWatchpoint(const QString& expression);
    bool removeBreakpoint(const QString& breakpointId);
    void removeAllBreakpoints();
    void enableBreakpoint(const QString& breakpointId, bool enabled);
    QList<Breakpoint> getBreakpoints() const;
    
    // State and information
    DebugState currentState() const;
    QList<StackFrame> getCallStack() const;
    QList<VariableInfo> getVariables(const QString& scope = "") const;
    QList<DebugEvent> getDebugEvents() const;
    
    // Expression evaluation
    QString evaluateExpression(const QString& expression);
    void watchExpression(const QString& expression);
    void unwatchExpression(const QString& expression);
    
    // Configuration
    void setDebuggerPath(const QString& path);
    void setSourcePaths(const QStringList& paths);
    void setSymbolPaths(const QStringList& paths);
    void setEnvironmentVariables(const QMap<QString, QString>& env);

signals:
    void debugStateChanged(DebugState newState, DebugState oldState);
    void breakpointHit(const Breakpoint& breakpoint, const StackFrame& frame);
    void debugEventOccurred(const DebugEvent& event);
    void variablesUpdated(const QList<VariableInfo>& variables);
    void callStackUpdated(const QList<StackFrame>& callStack);
    void debugOutput(const QString& output, bool isError = false);
    void expressionEvaluated(const QString& expression, const QString& result);

private slots:
    void onDebugProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDebugProcessError(QProcess::ProcessError error);
    void onDebugProcessOutput();
    void onDebugProcessErrorOutput();
    void onBreakpointManagerUpdated();

private:
    DebugState m_state;
    std::unique_ptr<QProcess> m_debugProcess;
    // TODO: Implement breakpoint management
    
    QString m_debuggerPath;
    QStringList m_sourcePaths;
    QStringList m_symbolPaths;
    QMap<QString, QString> m_environmentVariables;
    QString m_currentPluginPath;
    QList<DebugEvent> m_debugEvents;
    mutable QMutex m_eventsMutex;
    
    void initializeDebugger();
    void setupDebugProcess();
    void parseDebugOutput(const QString& output);
    void handleBreakpointHit(const QString& output);
    void handleVariableUpdate(const QString& output);
    void handleCallStackUpdate(const QString& output);
    void sendDebugCommand(const QString& command);
    QString formatDebugCommand(const QString& command, const QStringList& args = {});
};

// BreakpointManager is defined in PluginLiveDebugger.h

// VariableInspector is defined in PluginLiveDebugger.h

// CallStackViewer is defined in PluginLiveDebugger.h

// DebugConsole is defined in PluginLiveDebugger.h

// PluginProfiler is defined in PluginPerformanceProfiler.h

// PluginDebugWindow is defined in PluginLiveDebugger.h
