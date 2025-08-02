// ApplicationManager.h - Application lifecycle management
#pragma once

#include <QObject>
#include <QString>

class ApplicationManager : public QObject {
    Q_OBJECT

public:
    explicit ApplicationManager(QObject* parent = nullptr);
    ~ApplicationManager();
    
    bool initialize();
    void shutdown();
    
    QString applicationDataPath() const;
    QString pluginsPath() const;
    QString configPath() const;

signals:
    void initialized();
    void shutdownRequested();

private:
    bool m_initialized;
    QString m_dataPath;
    QString m_pluginsPath;
    QString m_configPath;
    
    void createDirectories();
};
