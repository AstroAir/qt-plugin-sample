// ApplicationManager.cpp - Application lifecycle implementation
#include "ApplicationManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

ApplicationManager::ApplicationManager(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_pluginsPath = m_dataPath + "/plugins";
    m_configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    
    initialize();
}

ApplicationManager::~ApplicationManager() {
    shutdown();
}

bool ApplicationManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    createDirectories();
    m_initialized = true;
    emit initialized();
    return true;
}

void ApplicationManager::shutdown() {
    if (m_initialized) {
        emit shutdownRequested();
        m_initialized = false;
    }
}

QString ApplicationManager::applicationDataPath() const {
    return m_dataPath;
}

QString ApplicationManager::pluginsPath() const {
    return m_pluginsPath;
}

QString ApplicationManager::configPath() const {
    return m_configPath;
}

void ApplicationManager::createDirectories() {
    QDir().mkpath(m_dataPath);
    QDir().mkpath(m_pluginsPath);
    QDir().mkpath(m_configPath);
}
