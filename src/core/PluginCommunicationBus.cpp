// PluginCommunicationBus.cpp
#include "PluginCommunicationBus.h"
#include "PluginInterface.h"
#include <QDateTime>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(pluginManager)

PluginCommunicationBus::PluginCommunicationBus(QObject* parent)
    : QObject(parent)
{
}

void PluginCommunicationBus::registerPlugin(const QString& name, IPlugin* plugin) {
    m_plugins[name] = plugin;
    qCInfo(pluginManager) << "Plugin registered with communication bus:" << name;
}

void PluginCommunicationBus::unregisterPlugin(const QString& name) {
    m_plugins.remove(name);
    m_subscriptions.remove(name);
    qCInfo(pluginManager) << "Plugin unregistered from communication bus:" << name;
}

bool PluginCommunicationBus::sendMessage(const QString& from, const QString& to, const QVariantMap& message) {
    auto it = m_plugins.find(to);
    if (it == m_plugins.end()) {
        qCWarning(pluginManager) << "Cannot send message to unknown plugin:" << to;
        return false;
    }
    
    // **Send message to target plugin**
    QVariantMap fullMessage = message;
    fullMessage["_from"] = from;
    fullMessage["_timestamp"] = QDateTime::currentDateTime();
    
    QVariant result = it.value()->executeCommand("receive_message", fullMessage);
    
    emit messageReceived(from, to, message);
    
    return result.toBool();
}

void PluginCommunicationBus::broadcastMessage(const QString& from, const QVariantMap& message) {
    QVariantMap fullMessage = message;
    fullMessage["_from"] = from;
    fullMessage["_timestamp"] = QDateTime::currentDateTime();
    fullMessage["_broadcast"] = true;
    
    for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it) {
        if (it.key() != from) {
            it.value()->executeCommand("receive_broadcast", fullMessage);
        }
    }
    
    emit messageReceived(from, "broadcast", message);
}