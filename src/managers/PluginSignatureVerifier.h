// PluginSignatureVerifier.h - Digital Signature Verification and Certificate Management
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
#include <QTimer>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslSocket>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <memory>

// Forward declarations
class PluginSignatureVerifier;
class CertificateManager;
class SignatureValidator;
class TrustStore;
class RevocationChecker;
class SigningDialog;

// Signature algorithms
enum class SignatureAlgorithm {
    RSA_SHA256,
    RSA_SHA384,
    RSA_SHA512,
    ECDSA_SHA256,
    ECDSA_SHA384,
    ECDSA_SHA512,
    DSA_SHA256,
    Unknown
};

// Certificate types
enum class CertificateType {
    Root,           // Root certificate authority
    Intermediate,   // Intermediate certificate authority
    EndEntity,      // End entity (plugin developer)
    SelfSigned,     // Self-signed certificate
    Unknown
};

// Verification status
enum class VerificationStatus {
    Valid,          // Signature is valid and trusted
    Invalid,        // Signature is invalid
    Untrusted,      // Signature is valid but not trusted
    Expired,        // Certificate has expired
    Revoked,        // Certificate has been revoked
    Unknown,        // Unable to verify
    NotSigned       // Plugin is not signed
};

// Trust level
enum class TrustLevel {
    Trusted,        // Fully trusted
    Conditional,    // Conditionally trusted
    Untrusted,      // Not trusted
    Blocked         // Explicitly blocked
};

// Certificate information
struct CertificateInfo {
    QString fingerprint;
    QString subject;
    QString issuer;
    QString serialNumber;
    QDateTime validFrom;
    QDateTime validTo;
    SignatureAlgorithm algorithm;
    CertificateType type;
    int keySize;
    QString publicKey;
    QStringList extensions;
    QSslCertificate certificate;
    
    CertificateInfo() = default;
    CertificateInfo(const QSslCertificate& cert);
    
    bool isValid() const;
    bool isExpired() const;
    bool isSelfSigned() const;
    QString getCommonName() const;
    QString getOrganization() const;
    QString getCountry() const;
};

// Signature information
struct SignatureInfo {
    QString pluginPath;
    QString signatureData;
    QString timestampData;
    CertificateInfo signerCertificate;
    QList<CertificateInfo> certificateChain;
    SignatureAlgorithm algorithm;
    QDateTime signedDate;
    QDateTime timestampDate;
    VerificationStatus status;
    QString statusMessage;
    QStringList validationErrors;
    QJsonObject metadata;
    
    SignatureInfo() = default;
    SignatureInfo(const QString& path) : pluginPath(path) {}
    
    bool isValid() const { return status == VerificationStatus::Valid; }
    bool isTrusted() const { return status == VerificationStatus::Valid || status == VerificationStatus::Untrusted; }
    bool hasTimestamp() const { return !timestampData.isEmpty(); }
};

// Trust store entry
struct TrustStoreEntry {
    QString fingerprint;
    CertificateInfo certificate;
    TrustLevel trustLevel;
    QString description;
    QDateTime addedDate;
    QDateTime lastUsed;
    QString addedBy;
    bool isEnabled;
    QJsonObject metadata;
    
    TrustStoreEntry() = default;
    TrustStoreEntry(const CertificateInfo& cert, TrustLevel level)
        : certificate(cert), trustLevel(level), addedDate(QDateTime::currentDateTime()), isEnabled(true) {
        fingerprint = cert.fingerprint;
    }
};

// Main signature verifier
class PluginSignatureVerifier : public QObject {
    Q_OBJECT

public:
    explicit PluginSignatureVerifier(QObject* parent = nullptr);
    ~PluginSignatureVerifier() override;

    // Verification operations
    SignatureInfo verifyPlugin(const QString& pluginPath);
    VerificationStatus verifySignature(const QString& pluginPath, const QString& signatureData);
    bool verifyIntegrity(const QString& pluginPath, const QString& expectedHash = "");
    QStringList verifyPluginBundle(const QStringList& pluginPaths);
    
    // Certificate management
    void addTrustedCertificate(const CertificateInfo& certificate, const QString& description = "");
    void removeTrustedCertificate(const QString& fingerprint);
    void blockCertificate(const QString& fingerprint, const QString& reason = "");
    void unblockCertificate(const QString& fingerprint);
    QList<TrustStoreEntry> getTrustedCertificates() const;
    QList<TrustStoreEntry> getBlockedCertificates() const;
    
    // Trust management
    TrustLevel getCertificateTrustLevel(const QString& fingerprint) const;
    void setCertificateTrustLevel(const QString& fingerprint, TrustLevel level);
    bool isCertificateTrusted(const QString& fingerprint) const;
    bool isCertificateBlocked(const QString& fingerprint) const;
    
    // Certificate validation
    bool validateCertificateChain(const QList<CertificateInfo>& chain) const;
    bool checkCertificateRevocation(const CertificateInfo& certificate) const;
    QStringList validateCertificate(const CertificateInfo& certificate) const;
    
    // Signing operations (for plugin developers)
    bool signPlugin(const QString& pluginPath, const QString& certificatePath, const QString& privateKeyPath, const QString& password = "");
    bool addTimestamp(const QString& pluginPath, const QString& timestampUrl = "");
    QString generateSignature(const QByteArray& data, const QString& privateKeyPath, const QString& password = "");
    
    // Configuration
    void setRequireSignatures(bool require);
    bool requireSignatures() const;
    void setAllowSelfSigned(bool allow);
    bool allowSelfSigned() const;
    void setCheckRevocation(bool check);
    bool checkRevocation() const;
    void setTimestampRequired(bool required);
    bool timestampRequired() const;
    void setTrustStoreDirectory(const QString& directory);
    QString trustStoreDirectory() const;
    
    // Certificate store operations
    void importCertificate(const QString& certificatePath);
    void exportCertificate(const QString& fingerprint, const QString& filePath);
    void importTrustStore(const QString& filePath);
    void exportTrustStore(const QString& filePath);
    void refreshCertificateStore();

signals:
    void verificationCompleted(const QString& pluginPath, VerificationStatus status);
    void certificateAdded(const QString& fingerprint);
    void certificateRemoved(const QString& fingerprint);
    void certificateBlocked(const QString& fingerprint, const QString& reason);
    void trustLevelChanged(const QString& fingerprint, TrustLevel oldLevel, TrustLevel newLevel);
    void revocationStatusChanged(const QString& fingerprint, bool isRevoked);
    void verificationError(const QString& error);

public slots:
    void refreshRevocationLists();
    void cleanupExpiredCertificates();
    void showCertificateManager();

private slots:
    void onRevocationCheckFinished();
    void onCertificateStoreChanged();

private:
    struct SignatureVerifierPrivate;
    std::unique_ptr<SignatureVerifierPrivate> d;
    
    void initializeVerifier();
    void loadConfiguration();
    void saveConfiguration();
    void loadTrustStore();
    void saveTrustStore();
    void setupRevocationChecking();
    SignatureInfo extractSignatureInfo(const QString& pluginPath);
    bool validateSignatureData(const QString& signatureData, const QByteArray& pluginData, const CertificateInfo& certificate);
    QString calculateFileHash(const QString& filePath, QCryptographicHash::Algorithm algorithm = QCryptographicHash::Sha256);
};

// Certificate manager for certificate operations
class CertificateManager : public QObject {
    Q_OBJECT

public:
    explicit CertificateManager(QObject* parent = nullptr);
    ~CertificateManager() override;

    // Certificate operations
    CertificateInfo loadCertificate(const QString& filePath) const;
    QList<CertificateInfo> loadCertificateChain(const QString& filePath) const;
    bool saveCertificate(const CertificateInfo& certificate, const QString& filePath) const;
    bool validateCertificate(const CertificateInfo& certificate) const;
    
    // Certificate parsing
    CertificateInfo parseCertificate(const QSslCertificate& certificate) const;
    QSslKey parsePrivateKey(const QString& keyData, const QString& password = "") const;
    SignatureAlgorithm getSignatureAlgorithm(const QSslCertificate& certificate) const;
    CertificateType getCertificateType(const QSslCertificate& certificate) const;
    
    // Certificate generation (for development)
    bool generateSelfSignedCertificate(const QString& subject, const QString& filePath, int validityDays = 365);
    bool generateCertificateRequest(const QString& subject, const QString& filePath);
    
    // Certificate store integration
    QList<CertificateInfo> getSystemCertificates() const;
    QList<CertificateInfo> getUserCertificates() const;
    bool installCertificate(const CertificateInfo& certificate, bool systemStore = false);
    bool removeCertificate(const QString& fingerprint, bool systemStore = false);

signals:
    void certificateLoaded(const CertificateInfo& certificate);
    void certificateValidated(const QString& fingerprint, bool isValid);
    void certificateGenerated(const QString& filePath);

private:
    QString extractSubjectField(const QString& subject, const QString& field) const;
    QStringList extractExtensions(const QSslCertificate& certificate) const;
    bool isRootCertificate(const QSslCertificate& certificate) const;
    bool isIntermediateCertificate(const QSslCertificate& certificate) const;
};

// Signature validator for cryptographic operations
class SignatureValidator : public QObject {
    Q_OBJECT

public:
    explicit SignatureValidator(QObject* parent = nullptr);
    ~SignatureValidator() override;

    // Signature validation
    bool validateSignature(const QByteArray& data, const QString& signature, const CertificateInfo& certificate) const;
    bool validateRSASignature(const QByteArray& data, const QString& signature, const QSslKey& publicKey, QCryptographicHash::Algorithm hashAlgorithm) const;
    bool validateECDSASignature(const QByteArray& data, const QString& signature, const QSslKey& publicKey, QCryptographicHash::Algorithm hashAlgorithm) const;
    
    // Hash verification
    QString calculateHash(const QByteArray& data, QCryptographicHash::Algorithm algorithm) const;
    bool verifyHash(const QByteArray& data, const QString& expectedHash, QCryptographicHash::Algorithm algorithm) const;
    
    // Signature creation
    QString createSignature(const QByteArray& data, const QSslKey& privateKey, QCryptographicHash::Algorithm hashAlgorithm) const;
    QString createRSASignature(const QByteArray& data, const QSslKey& privateKey, QCryptographicHash::Algorithm hashAlgorithm) const;
    QString createECDSASignature(const QByteArray& data, const QSslKey& privateKey, QCryptographicHash::Algorithm hashAlgorithm) const;
    
    // Algorithm support
    QList<SignatureAlgorithm> supportedAlgorithms() const;
    QCryptographicHash::Algorithm getHashAlgorithm(SignatureAlgorithm algorithm) const;
    QString getAlgorithmName(SignatureAlgorithm algorithm) const;

signals:
    void validationCompleted(bool isValid);
    void signatureCreated(const QString& signature);

private:
    QByteArray hashData(const QByteArray& data, QCryptographicHash::Algorithm algorithm) const;
    bool verifyRSAPKCS1Signature(const QByteArray& hash, const QByteArray& signature, const QSslKey& publicKey) const;
    bool verifyECDSASignature(const QByteArray& hash, const QByteArray& signature, const QSslKey& publicKey) const;
};

// Trust store for managing trusted certificates
class TrustStore : public QObject {
    Q_OBJECT

public:
    explicit TrustStore(const QString& storeDirectory, QObject* parent = nullptr);
    ~TrustStore() override;

    // Trust store operations
    void addEntry(const TrustStoreEntry& entry);
    void removeEntry(const QString& fingerprint);
    void updateEntry(const TrustStoreEntry& entry);
    TrustStoreEntry getEntry(const QString& fingerprint) const;
    QList<TrustStoreEntry> getAllEntries() const;
    QList<TrustStoreEntry> getEntriesByTrustLevel(TrustLevel level) const;
    
    // Trust queries
    bool isTrusted(const QString& fingerprint) const;
    bool isBlocked(const QString& fingerprint) const;
    TrustLevel getTrustLevel(const QString& fingerprint) const;
    void setTrustLevel(const QString& fingerprint, TrustLevel level);
    
    // Store management
    void loadStore();
    void saveStore();
    void clearStore();
    void importStore(const QString& filePath);
    void exportStore(const QString& filePath);
    
    // Statistics
    int getTrustedCount() const;
    int getBlockedCount() const;
    int getTotalCount() const;
    QDateTime getLastModified() const;

signals:
    void entryAdded(const QString& fingerprint);
    void entryRemoved(const QString& fingerprint);
    void entryUpdated(const QString& fingerprint);
    void trustLevelChanged(const QString& fingerprint, TrustLevel oldLevel, TrustLevel newLevel);
    void storeLoaded();
    void storeSaved();

private:
    QString m_storeDirectory;
    QString m_storeFilePath;
    QMap<QString, TrustStoreEntry> m_entries;
    QDateTime m_lastModified;
    
    void ensureStoreDirectory();
    QString getStoreFilePath() const;
    QJsonObject entryToJson(const TrustStoreEntry& entry) const;
    TrustStoreEntry entryFromJson(const QJsonObject& json) const;
};

// Revocation checker for certificate revocation lists
class RevocationChecker : public QObject {
    Q_OBJECT

public:
    explicit RevocationChecker(QObject* parent = nullptr);
    ~RevocationChecker() override;

    // Revocation checking
    void checkRevocation(const CertificateInfo& certificate);
    bool isRevoked(const QString& fingerprint) const;
    QDateTime getRevocationDate(const QString& fingerprint) const;
    QString getRevocationReason(const QString& fingerprint) const;
    
    // CRL management
    void downloadCRL(const QUrl& crlUrl);
    void loadCRL(const QString& filePath);
    void refreshAllCRLs();
    QStringList getLoadedCRLs() const;
    
    // OCSP support
    void checkOCSP(const CertificateInfo& certificate, const QUrl& ocspUrl);
    bool isOCSPEnabled() const;
    void setOCSPEnabled(bool enabled);
    
    // Configuration
    void setCRLCacheDirectory(const QString& directory);
    QString crlCacheDirectory() const;
    void setCRLRefreshInterval(int hours);
    int crlRefreshInterval() const;

signals:
    void revocationCheckCompleted(const QString& fingerprint, bool isRevoked);
    void crlDownloaded(const QUrl& crlUrl);
    void crlLoadFailed(const QUrl& crlUrl, const QString& error);
    void ocspResponseReceived(const QString& fingerprint, bool isRevoked);

private slots:
    void onCRLDownloadFinished();
    void onOCSPResponseFinished();
    void onRefreshTimer();

private:
    struct RevocationInfo {
        QString fingerprint;
        bool isRevoked;
        QDateTime revocationDate;
        QString reason;
    };
    
    QNetworkAccessManager* m_networkManager;
    QMap<QString, RevocationInfo> m_revocationCache;
    QStringList m_loadedCRLs;
    QString m_crlCacheDirectory;
    QTimer* m_refreshTimer;
    int m_refreshInterval;
    bool m_ocspEnabled;
    
    void parseCRL(const QByteArray& crlData);
    void parseOCSPResponse(const QByteArray& responseData);
    QString getCRLCacheFilePath(const QUrl& crlUrl) const;
};

// Certificate manager widget
class CertificateManagerWidget : public QWidget {
    Q_OBJECT

public:
    explicit CertificateManagerWidget(PluginSignatureVerifier* verifier, QWidget* parent = nullptr);
    ~CertificateManagerWidget() override;

    // Display management
    void refreshCertificateList();
    void refreshTrustStore();
    void showCertificateDetails(const QString& fingerprint);

signals:
    void certificateSelected(const QString& fingerprint);
    void trustLevelChangeRequested(const QString& fingerprint, TrustLevel level);
    void certificateImportRequested();
    void certificateExportRequested(const QString& fingerprint);

private slots:
    void onCertificateItemClicked();
    void onTrustLevelChanged();
    void onImportClicked();
    void onExportClicked();
    void onRemoveClicked();
    void onRefreshClicked();

private:
    PluginSignatureVerifier* m_verifier;
    QTreeWidget* m_certificateTree;
    QTextEdit* m_detailsView;
    QComboBox* m_trustLevelCombo;
    QSplitter* m_splitter;
    
    void setupUI();
    void populateCertificateTree();
    void updateDetailsView(const CertificateInfo& certificate);
    QTreeWidgetItem* createCertificateItem(const TrustStoreEntry& entry);
    QString formatCertificateDetails(const CertificateInfo& certificate);
};

// Plugin signing dialog
class SigningDialog : public QDialog {
    Q_OBJECT

public:
    explicit SigningDialog(const QString& pluginPath, QWidget* parent = nullptr);
    ~SigningDialog() override;

    // Signing configuration
    QString getCertificatePath() const;
    QString getPrivateKeyPath() const;
    QString getPassword() const;
    bool includeTimestamp() const;
    QString getTimestampUrl() const;

public slots:
    void accept() override;
    void reject() override;

signals:
    void signingRequested(const QString& pluginPath, const QString& certificatePath, const QString& privateKeyPath, const QString& password);

private slots:
    void onBrowseCertificate();
    void onBrowsePrivateKey();
    void onTimestampToggled();
    void onSignClicked();

private:
    QString m_pluginPath;
    QLineEdit* m_certificateEdit;
    QLineEdit* m_privateKeyEdit;
    QLineEdit* m_passwordEdit;
    QCheckBox* m_timestampCheck;
    QLineEdit* m_timestampUrlEdit;
    QPushButton* m_signButton;
    
    void setupUI();
    void validateInputs();
};
