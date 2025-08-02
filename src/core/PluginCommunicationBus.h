// PluginCommunicationBus.h
#pragma once
#include <QObject>
#include <QVariantMap>
#include <QString>
#include <QStringList>
#include <QHash>

class IPlugin;

class PluginCommunicationBus : public QObject {
    Q_OBJECT
    
public:
    explicit PluginCommunicationBus(QObject* parent = nullptr);
    
    void registerPlugin(const QString& name, IPlugin* plugin);
    void unregisterPlugin(const QString& name);
    
    bool sendMessage(const QString& from, const QString& to, const QVariantMap& message);
    void broadcastMessage(const QString& from, const QVariantMap& message);
    
    void subscribeToEvents(const QString& pluginName, const QStringList& events);
    void unsubscribeFromEvents(const QString& pluginName, const QStringList& events);
    
signals:
    void messageReceived(const QString& from, const QString& to, const QVariantMap& message);
    void eventTriggered(const QString& event, const QVariantMap& data);
    
private:
    QHash<QString, IPlugin*> m_plugins;
    QHash<QString, QStringList> m_subscriptions;
    
    void routeMessage(const QString& from, const QString& to, const QVariantMap& message);
};

