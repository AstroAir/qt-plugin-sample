// PluginSecurityManager.h
#pragma once
#include <QObject>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include "../core/PluginInterface.h"

class PluginSecurityManager : public QObject {
    Q_OBJECT
    
public:
    explicit PluginSecurityManager(QObject* parent = nullptr);
    
    bool validateSignature(const QString& filePath) const;
    bool checkPermissions(const QString& pluginName, const QStringList& permissions) const;
    void applySandbox(const QString& pluginName);
    SecurityLevel evaluateSecurityLevel(const QString& filePath) const;
    
    void addTrustedPublisher(const QString& publisher);
    void removeTrustedPublisher(const QString& publisher);
    QStringList trustedPublishers() const;
    
signals:
    void securityViolationDetected(const QString& pluginName, const QString& violation);
    
private:
    QStringList m_trustedPublishers;
    QJsonObject m_securityPolicies;
    
    QString calculateHash(const QString& filePath) const;
    bool verifyDigitalSignature(const QString& filePath) const;
    QJsonObject loadSecurityPolicy() const;
};
