// PluginMarketplace.h - Online Plugin Marketplace Integration System
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QProgressBar>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QFrame>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QPixmap>
#include <QIcon>
#include <QDateTime>
#include <QVersionNumber>
#include <QCryptographicHash>
#include <QMutex>
#include <QThread>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <memory>
#include "ui/PluginStoreWidget.h"

// Forward declarations
class PluginMarketplaceManager;
class MarketplaceRepository;
class PluginDetailsWidget;
class PluginSearchWidget;
class PluginCategoryWidget;
class PluginReviewWidget;
class MarketplaceDownloader;

// Plugin categories
enum class PluginCategory {
    All,
    Development,
    Productivity,
    Graphics,
    Audio,
    Video,
    Games,
    Utilities,
    Security,
    Network,
    Database,
    WebDevelopment,
    MobileDevelopment,
    DataScience,
    MachineLearning,
    Education,
    Business,
    Entertainment,
    Custom
};

// Plugin licensing
enum class PluginLicense {
    Unknown,
    MIT,
    GPL,
    LGPL,
    Apache,
    BSD,
    Commercial,
    Proprietary,
    Creative,
    PublicDomain,
    Custom
};

// Plugin maturity levels
enum class PluginMaturity {
    Alpha,
    Beta,
    Stable,
    Mature,
    Legacy,
    Deprecated
};

// Marketplace plugin information
struct MarketplacePlugin {
    QString id;
    QString name;
    QString version;
    QString description;
    QString longDescription;
    QString author;
    QString authorEmail;
    QString authorWebsite;
    QString website;
    QString repository;
    QString documentation;
    PluginCategory category;
    QStringList tags;
    PluginLicense license;
    QString licenseText;
    PluginMaturity maturity;
    
    // Ratings and reviews
    double rating = 0.0;
    int reviewCount = 0;
    int downloadCount = 0;
    int likeCount = 0;
    
    // Technical information
    QVersionNumber versionNumber;
    QStringList supportedPlatforms;
    QStringList supportedArchitectures;
    QString minSystemVersion;
    QStringList dependencies;
    QStringList conflicts;
    qint64 size = 0;
    
    // Download information
    QUrl downloadUrl;
    QString checksum;
    QDateTime releaseDate;
    QDateTime lastUpdate;
    
    // Media
    QUrl iconUrl;
    QList<QUrl> screenshotUrls;
    QUrl videoUrl;
    
    // Marketplace metadata
    bool isFeatured = false;
    bool isVerified = false;
    bool isOpenSource = false;
    bool isFree = true;
    double price = 0.0;
    QString currency = "USD";
    
    QJsonObject metadata;
    
    MarketplacePlugin() = default;
    MarketplacePlugin(const QString& i, const QString& n, const QString& v)
        : id(i), name(n), version(v), versionNumber(QVersionNumber::fromString(v)) {}
    
    bool isCompatible() const;
    QString getCategoryString() const;
    QString getLicenseString() const;
    QString getMaturityString() const;
};

// Plugin review information
struct PluginReview {
    QString id;
    QString pluginId;
    QString userId;
    QString userName;
    QString title;
    QString content;
    int rating = 0; // 1-5 stars
    QDateTime date;
    int helpfulCount = 0;
    bool isVerified = false;
    QString version; // Plugin version this review is for
    
    PluginReview() = default;
    PluginReview(const QString& i, const QString& pid, int r)
        : id(i), pluginId(pid), rating(r) {}
};

// Search filters
struct MarketplaceSearchFilter {
    QString query;
    PluginCategory category = PluginCategory::All;
    QStringList tags;
    PluginLicense license = PluginLicense::Unknown;
    PluginMaturity maturity = PluginMaturity::Stable;
    double minRating = 0.0;
    bool freeOnly = false;
    bool openSourceOnly = false;
    bool verifiedOnly = false;
    QString platform;
    QString sortBy = "popularity"; // popularity, rating, date, name, downloads
    bool sortDescending = true;
    int limit = 50;
    int offset = 0;
    
    MarketplaceSearchFilter() = default;
};

// Main marketplace manager
class PluginMarketplaceManager : public QObject {
    Q_OBJECT

public:
    explicit PluginMarketplaceManager(QObject* parent = nullptr);
    ~PluginMarketplaceManager() override;

    // Repository management
    void addRepository(const QString& name, const QUrl& url, const QString& apiKey = "");
    void removeRepository(const QString& name);
    QStringList repositories() const;
    void refreshRepositories();
    
    // Plugin discovery
    QList<MarketplacePlugin> searchPlugins(const MarketplaceSearchFilter& filter);
    QList<MarketplacePlugin> getFeaturedPlugins();
    QList<MarketplacePlugin> getPopularPlugins(int limit = 20);
    QList<MarketplacePlugin> getRecentPlugins(int limit = 20);
    QList<MarketplacePlugin> getPluginsByCategory(PluginCategory category);
    QList<MarketplacePlugin> getPluginsByAuthor(const QString& author);
    
    // Plugin information
    MarketplacePlugin getPluginDetails(const QString& pluginId);
    QList<PluginReview> getPluginReviews(const QString& pluginId, int limit = 10, int offset = 0);
    QStringList getPluginVersions(const QString& pluginId);
    QList<MarketplacePlugin> getRelatedPlugins(const QString& pluginId);
    QList<MarketplacePlugin> getPluginDependencies(const QString& pluginId);
    
    // Plugin operations
    void downloadPlugin(const QString& pluginId, const QString& version = "");
    void installPlugin(const QString& pluginId, const QString& version = "");
    void uninstallPlugin(const QString& pluginId);
    void updatePlugin(const QString& pluginId);
    
    // User operations (if authenticated)
    void submitReview(const QString& pluginId, const PluginReview& review);
    void updateReview(const QString& reviewId, const PluginReview& review);
    void deleteReview(const QString& reviewId);
    void likePlugin(const QString& pluginId);
    void unlikePlugin(const QString& pluginId);
    void reportPlugin(const QString& pluginId, const QString& reason);
    
    // Authentication
    void authenticate(const QString& username, const QString& password);
    void logout();
    bool isAuthenticated() const;
    QString currentUser() const;
    
    // Configuration
    void setCacheDirectory(const QString& directory);
    QString cacheDirectory() const;
    void setMaxConcurrentDownloads(int count);
    int maxConcurrentDownloads() const;
    void setAutoRefreshInterval(int minutes);
    int autoRefreshInterval() const;

signals:
    void repositoryAdded(const QString& name);
    void repositoryRemoved(const QString& name);
    void repositoryRefreshed(const QString& name);
    void searchCompleted(const QList<MarketplacePlugin>& results);
    void pluginDetailsLoaded(const MarketplacePlugin& plugin);
    void pluginDownloadStarted(const QString& pluginId);
    void pluginDownloadProgress(const QString& pluginId, int percentage);
    void pluginDownloadCompleted(const QString& pluginId);
    void pluginInstalled(const QString& pluginId);
    void pluginUninstalled(const QString& pluginId);
    void pluginUpdated(const QString& pluginId);
    void reviewSubmitted(const QString& pluginId, const QString& reviewId);
    void authenticationChanged(bool authenticated);
    void errorOccurred(const QString& error);

public slots:
    void refreshCache();
    void clearCache();
    void showMarketplace();

private slots:
    void onRepositoryRefreshFinished();
    void onSearchFinished();
    void onDownloadFinished();
    void onAuthenticationFinished();

private:
    struct MarketplaceManagerPrivate;
    std::unique_ptr<MarketplaceManagerPrivate> d;
    
    void initializeManager();
    void loadConfiguration();
    void saveConfiguration();
    void setupAutoRefresh();
    void processSearchResults(const QByteArray& data);
    void processPluginDetails(const QByteArray& data, const QString& pluginId);
    MarketplacePlugin parsePluginData(const QJsonObject& json);
    PluginReview parseReviewData(const QJsonObject& json);
};

// Marketplace repository interface
class MarketplaceRepository : public QObject {
    Q_OBJECT

public:
    explicit MarketplaceRepository(const QString& name, const QUrl& baseUrl, const QString& apiKey = "", QObject* parent = nullptr);
    ~MarketplaceRepository() override;

    // Repository information
    QString name() const;
    QUrl baseUrl() const;
    QString apiKey() const;
    void setApiKey(const QString& apiKey);
    bool isOnline() const;
    QDateTime lastRefresh() const;
    
    // API operations
    void searchPlugins(const MarketplaceSearchFilter& filter);
    void getPluginDetails(const QString& pluginId);
    void getPluginReviews(const QString& pluginId, int limit = 10, int offset = 0);
    void getFeaturedPlugins();
    void getPopularPlugins(int limit = 20);
    void getRecentPlugins(int limit = 20);
    
    // Authentication
    void authenticate(const QString& username, const QString& password);
    void submitReview(const QString& pluginId, const PluginReview& review);
    void likePlugin(const QString& pluginId);
    void reportPlugin(const QString& pluginId, const QString& reason);
    
    // Configuration
    void setTimeout(int seconds);
    int timeout() const;
    void setUserAgent(const QString& userAgent);
    QString userAgent() const;

signals:
    void searchCompleted(const QList<MarketplacePlugin>& results);
    void pluginDetailsLoaded(const MarketplacePlugin& plugin);
    void reviewsLoaded(const QString& pluginId, const QList<PluginReview>& reviews);
    void featuredPluginsLoaded(const QList<MarketplacePlugin>& plugins);
    void authenticationCompleted(bool success, const QString& token);
    void operationCompleted(const QString& operation, bool success);
    void errorOccurred(const QString& error);

private slots:
    void onNetworkReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    QString m_name;
    QUrl m_baseUrl;
    QString m_apiKey;
    QString m_authToken;
    QNetworkAccessManager* m_networkManager;
    QMap<QNetworkReply*, QString> m_activeRequests;
    QDateTime m_lastRefresh;
    bool m_isOnline;
    int m_timeout;
    QString m_userAgent;
    
    QUrl buildApiUrl(const QString& endpoint, const QUrlQuery& query = QUrlQuery()) const;
    QNetworkRequest createRequest(const QUrl& url) const;
    void processApiResponse(const QByteArray& data, const QString& operation);
};

// Note: PluginStoreWidget class is defined in ui/PluginStoreWidget.h

// Note: PluginSearchWidget class should be defined in ui/PluginStoreWidget.h or a separate header
