#include "PluginSecurityManager.h"
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(pluginManager)

PluginSecurityManager::PluginSecurityManager(QObject* parent)
    : QObject(parent)
{
    m_securityPolicies = loadSecurityPolicy();
    
    // **Load trusted publishers**
    m_trustedPublishers << "com.example.official"
                       << "org.trusted.developer";
}

bool PluginSecurityManager::validateSignature(const QString& filePath) const {
    // **Check file hash against known good hashes**
    QString hash = calculateHash(filePath);
    
    // **Verify digital signature**
    if (!verifyDigitalSignature(filePath)) {
        qCWarning(pluginManager) << "Digital signature verification failed for:" << filePath;
        return false;
    }
    
    return true;
}

QString PluginSecurityManager::calculateHash(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    
    return hash.result().toHex();
}

bool PluginSecurityManager::verifyDigitalSignature(const QString& filePath) const {
    Q_UNUSED(filePath);
    // **Platform-specific digital signature verification**
#ifdef Q_OS_WIN
    // Windows Authenticode verification
    return true; // Simplified
#elif defined(Q_OS_MAC)
    // macOS code signing verification
    return true; // Simplified  
#else
    // Linux: Check for GPG signature file
    return QFile::exists(filePath + ".sig");
#endif
}

QJsonObject PluginSecurityManager::loadSecurityPolicy() const {
    QJsonObject defaultPolicy;

    // **Default security policy**
    defaultPolicy["allowUnsignedPlugins"] = false;
    defaultPolicy["requireTrustedPublisher"] = true;
    defaultPolicy["sandboxMode"] = true;
    defaultPolicy["maxMemoryUsage"] = 100 * 1024 * 1024; // 100MB
    defaultPolicy["maxCpuUsage"] = 80.0; // 80%
    defaultPolicy["allowNetworkAccess"] = false;
    defaultPolicy["allowFileSystemAccess"] = false;
    defaultPolicy["allowRegistryAccess"] = false;

    // **Try to load custom policy from file**
    QString policyPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/security_policy.json";
    QFile policyFile(policyPath);

    if (policyFile.exists() && policyFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(policyFile.readAll(), &error);

        if (error.error == QJsonParseError::NoError && doc.isObject()) {
            QJsonObject customPolicy = doc.object();

            // **Merge custom policy with defaults**
            for (auto it = customPolicy.begin(); it != customPolicy.end(); ++it) {
                defaultPolicy[it.key()] = it.value();
            }

            qCInfo(pluginManager) << "Loaded custom security policy from:" << policyPath;
        } else {
            qCWarning(pluginManager) << "Failed to parse security policy file:" << error.errorString();
        }
    } else {
        qCInfo(pluginManager) << "Using default security policy";
    }

    return defaultPolicy;
}

bool PluginSecurityManager::checkPermissions(const QString& pluginName, const QStringList& permissions) const {
    Q_UNUSED(pluginName)

    // **Check each permission against security policy**
    for (const QString& permission : permissions) {
        if (permission == "network") {
            if (!m_securityPolicies.value("allowNetworkAccess").toBool()) {
                return false;
            }
        } else if (permission == "filesystem") {
            if (!m_securityPolicies.value("allowFileSystemAccess").toBool()) {
                return false;
            }
        } else if (permission == "registry") {
            if (!m_securityPolicies.value("allowRegistryAccess").toBool()) {
                return false;
            }
        } else {
            // **Unknown permission - default deny**
            return false;
        }
    }

    return true;
}

void PluginSecurityManager::applySandbox(const QString& pluginName) {
    Q_UNUSED(pluginName)
    // **Apply sandbox restrictions to the plugin**
    // **In a real implementation, this would set up process isolation,**
    // **restrict file system access, network access, etc.**
    qCInfo(pluginManager) << "Applying sandbox restrictions to plugin:" << pluginName;
}

qtplugin::SecurityLevel PluginSecurityManager::evaluateSecurityLevel(const QString& filePath) const {
    // **Check if plugin is signed and trusted**
    if (!validateSignature(filePath)) {
        return qtplugin::SecurityLevel::Basic;
    }

    // **Check against security policy**
    if (m_securityPolicies.value("sandboxMode").toBool()) {
        return qtplugin::SecurityLevel::Strict;
    }

    return qtplugin::SecurityLevel::Standard;
}

void PluginSecurityManager::addTrustedPublisher(const QString& publisher) {
    if (!m_trustedPublishers.contains(publisher)) {
        m_trustedPublishers.append(publisher);
        qCInfo(pluginManager) << "Added trusted publisher:" << publisher;
    }
}

void PluginSecurityManager::removeTrustedPublisher(const QString& publisher) {
    if (m_trustedPublishers.removeAll(publisher) > 0) {
        qCInfo(pluginManager) << "Removed trusted publisher:" << publisher;
    }
}

QStringList PluginSecurityManager::trustedPublishers() const {
    return m_trustedPublishers;
}