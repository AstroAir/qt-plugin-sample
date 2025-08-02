// PluginRESTAPI.h - RESTful API Server for Plugin System Integration
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
#include <QTabWidget>
#include <QTimer>
#include <QSpinBox>
#include <QLineEdit>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>

#ifdef QT_HTTPSERVER_AVAILABLE
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#endif

#ifdef QT_WEBSOCKETS_AVAILABLE
#include <QWebSocket>
#include <QWebSocketServer>
#endif
#include <QJsonArray>
#include <QUrlQuery>
#include <QMimeDatabase>
#include <QCryptographicHash>
#include <QDateTime>
#include <QUuid>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <memory>

// Forward declarations
class PluginRESTAPIServer;
class APIEndpoint;
class APIAuthentication;
class APIRateLimiter;
class APIDocumentation;
class WebSocketHandler;
class APIWidget;

// HTTP methods
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};

// API response status
enum class APIStatus {
    Success = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    Conflict = 409,
    TooManyRequests = 429,
    InternalServerError = 500,
    NotImplemented = 501,
    ServiceUnavailable = 503
};

// Authentication types
enum class AuthType {
    None,           // No authentication
    Basic,          // Basic HTTP authentication
    Bearer,         // Bearer token
    ApiKey,         // API key
    OAuth2,         // OAuth 2.0
    JWT,            // JSON Web Token
    Custom          // Custom authentication
};

// API request information
struct APIRequest {
    QString requestId;
    HttpMethod method;
    QString path;
    QUrlQuery query;
    QJsonObject body;
    QMap<QString, QString> headers;
    QString clientIP;
    QString userAgent;
    QDateTime timestamp;
    QString userId;
    QString sessionId;
    QJsonObject metadata;
    
    APIRequest() = default;
    APIRequest(HttpMethod m, const QString& p)
        : method(m), path(p), timestamp(QDateTime::currentDateTime()) {
        requestId = generateRequestId();
    }
    
    QString getMethodString() const;
    bool hasQueryParameter(const QString& key) const;
    QString getQueryParameter(const QString& key) const;
    bool hasHeader(const QString& key) const;
    QString getHeader(const QString& key) const;
    
private:
    QString generateRequestId() const;
};

// API response information
struct APIResponse {
    APIStatus status;
    QJsonObject data;
    QMap<QString, QString> headers;
    QString contentType;
    QByteArray rawData;
    QString errorMessage;
    QStringList errors;
    QJsonObject metadata;
    QDateTime timestamp;
    
    APIResponse() = default;
    APIResponse(APIStatus s, const QJsonObject& d = QJsonObject())
        : status(s), data(d), contentType("application/json"), timestamp(QDateTime::currentDateTime()) {}
    
    static APIResponse success(const QJsonObject& data = QJsonObject());
    static APIResponse error(APIStatus status, const QString& message, const QStringList& errors = QStringList());
    static APIResponse notFound(const QString& resource = "");
    static APIResponse badRequest(const QString& message = "");
    static APIResponse unauthorized(const QString& message = "");
    static APIResponse forbidden(const QString& message = "");
    
    QByteArray toJson() const;
    void setHeader(const QString& key, const QString& value);
    QString getStatusText() const;
    bool isSuccess() const;
    bool isError() const;
};

// API endpoint definition
struct APIEndpoint {
    QString path;
    HttpMethod method;
    QString description;
    QStringList tags;
    QJsonObject parameters;
    QJsonObject requestSchema;
    QJsonObject responseSchema;
    AuthType authType;
    QStringList requiredPermissions;
    int rateLimitPerMinute;
    bool isDeprecated;
    QString deprecationMessage;
    QDateTime createdDate;
    QDateTime lastModified;
    QJsonObject metadata;
    
    APIEndpoint() = default;
    APIEndpoint(const QString& p, HttpMethod m, const QString& desc = "")
        : path(p), method(m), description(desc), authType(AuthType::None),
          rateLimitPerMinute(60), isDeprecated(false), createdDate(QDateTime::currentDateTime()),
          lastModified(QDateTime::currentDateTime()) {}
    
    QString getFullPath() const;
    QString getMethodString() const;
    bool matchesRequest(const APIRequest& request) const;
    bool requiresAuthentication() const;
};

// API configuration
struct APIConfig {
    QString serverName = "Qt Plugin System API";
    QString version = "1.0.0";
    QString description = "RESTful API for Qt Plugin System";
    QString host = "localhost";
    int port = 8080;
    bool enableSSL = false;
    QString sslCertPath;
    QString sslKeyPath;
    bool enableCORS = true;
    QStringList allowedOrigins = {"*"};
    QStringList allowedMethods = {"GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS"};
    QStringList allowedHeaders = {"Content-Type", "Authorization", "X-API-Key"};
    bool enableRateLimit = true;
    int defaultRateLimit = 100; // requests per minute
    bool enableLogging = true;
    QString logLevel = "info";
    QString logDirectory;
    bool enableDocumentation = true;
    QString documentationPath = "/docs";
    bool enableWebSocket = true;
    int webSocketPort = 8081;
    int maxConnections = 1000;
    int requestTimeout = 30000; // 30 seconds
    QJsonObject customSettings;
    
    APIConfig() = default;
};

// Main REST API server
class PluginRESTAPIServer : public QObject {
    Q_OBJECT

public:
    explicit PluginRESTAPIServer(QObject* parent = nullptr);
    ~PluginRESTAPIServer() override;

    // Server management
    bool start(const APIConfig& config = APIConfig());
    void stop();
    bool isRunning() const;
    QString serverUrl() const;
    APIConfig configuration() const;
    void setConfiguration(const APIConfig& config);
    
    // Endpoint management
    void registerEndpoint(const APIEndpoint& endpoint, std::function<APIResponse(const APIRequest&)> handler);
    void unregisterEndpoint(const QString& path, HttpMethod method);
    QList<APIEndpoint> getEndpoints() const;
    APIEndpoint getEndpoint(const QString& path, HttpMethod method) const;
    bool hasEndpoint(const QString& path, HttpMethod method) const;
    
    // Plugin API endpoints
    void registerPluginEndpoints();
    void registerSystemEndpoints();
    void registerManagementEndpoints();
    void registerMetricsEndpoints();
    
    // Authentication
    void setAuthenticationHandler(std::function<bool(const APIRequest&, QString&)> handler);
    void setAuthorizationHandler(std::function<bool(const APIRequest&, const QStringList&)> handler);
    void addAPIKey(const QString& key, const QString& userId, const QStringList& permissions = QStringList());
    void removeAPIKey(const QString& key);
    bool isValidAPIKey(const QString& key) const;
    
    // Rate limiting
    void setRateLimit(const QString& endpoint, int requestsPerMinute);
    void setGlobalRateLimit(int requestsPerMinute);
    bool isRateLimited(const QString& clientIP, const QString& endpoint) const;
    void resetRateLimit(const QString& clientIP);
    
    // WebSocket support
    void enableWebSocket(bool enable);
    bool isWebSocketEnabled() const;
    void broadcastMessage(const QJsonObject& message);
    void sendMessageToClient(const QString& clientId, const QJsonObject& message);
    QStringList getConnectedClients() const;
    
    // Logging and monitoring
    void setLoggingEnabled(bool enabled);
    bool isLoggingEnabled() const;
    QStringList getRequestLog(int maxEntries = 100) const;
    void clearRequestLog();
    QJsonObject getServerStatistics() const;
    QJsonObject getEndpointStatistics(const QString& path, HttpMethod method) const;

signals:
    void serverStarted(const QString& url);
    void serverStopped();
    void requestReceived(const APIRequest& request);
    void responseSent(const APIRequest& request, const APIResponse& response);
    void clientConnected(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void webSocketMessageReceived(const QString& clientId, const QJsonObject& message);
    void authenticationFailed(const APIRequest& request);
    void rateLimitExceeded(const QString& clientIP, const QString& endpoint);
    void serverError(const QString& error);

public slots:
    void reloadConfiguration();
    void showAPIWidget();
    void generateDocumentation();

private slots:
#ifdef QT_HTTPSERVER_AVAILABLE
    void onHttpRequestReceived(const QHttpServerRequest& request);
#endif
#ifdef QT_WEBSOCKETS_AVAILABLE
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketMessageReceived(const QString& message);
#endif
    void onRateLimitTimer();

private:
    struct RESTAPIServerPrivate;
    std::unique_ptr<RESTAPIServerPrivate> d;
    
    void initializeServer();
    void loadConfiguration();
    void saveConfiguration();
    void setupHttpServer();
    void setupWebSocketServer();
    void setupCORS();
    void setupRateLimit();
    void setupLogging();
    APIResponse handleRequest(const APIRequest& request);
    APIResponse routeRequest(const APIRequest& request);
    bool authenticateRequest(const APIRequest& request, QString& userId);
    bool authorizeRequest(const APIRequest& request, const QStringList& requiredPermissions);
    void logRequest(const APIRequest& request, const APIResponse& response);
    void updateStatistics(const APIRequest& request, const APIResponse& response);
    QString generateClientId() const;
};

// API authentication handler
class APIAuthentication : public QObject {
    Q_OBJECT

public:
    explicit APIAuthentication(QObject* parent = nullptr);
    ~APIAuthentication() override;

    // Authentication methods
    bool authenticateBasic(const QString& credentials, QString& userId);
    bool authenticateBearer(const QString& token, QString& userId);
    bool authenticateAPIKey(const QString& key, QString& userId);
    bool authenticateJWT(const QString& token, QString& userId);
    bool authenticateOAuth2(const QString& token, QString& userId);
    
    // User management
    void addUser(const QString& userId, const QString& password, const QStringList& roles = QStringList());
    void removeUser(const QString& userId);
    void updateUserPassword(const QString& userId, const QString& newPassword);
    void setUserRoles(const QString& userId, const QStringList& roles);
    QStringList getUserRoles(const QString& userId) const;
    bool isValidUser(const QString& userId) const;
    
    // Token management
    QString generateJWT(const QString& userId, const QStringList& roles, int expirationHours = 24);
    bool validateJWT(const QString& token, QString& userId, QStringList& roles);
    void revokeToken(const QString& token);
    bool isTokenRevoked(const QString& token) const;
    
    // API key management
    QString generateAPIKey(const QString& userId, const QStringList& permissions = QStringList());
    void revokeAPIKey(const QString& key);
    bool isValidAPIKey(const QString& key, QString& userId, QStringList& permissions) const;
    QStringList getUserAPIKeys(const QString& userId) const;
    
    // Session management
    QString createSession(const QString& userId);
    void destroySession(const QString& sessionId);
    bool isValidSession(const QString& sessionId, QString& userId) const;
    void extendSession(const QString& sessionId, int minutes = 30);

signals:
    void userAuthenticated(const QString& userId);
    void authenticationFailed(const QString& reason);
    void tokenGenerated(const QString& token, const QString& userId);
    void tokenRevoked(const QString& token);
    void sessionCreated(const QString& sessionId, const QString& userId);
    void sessionDestroyed(const QString& sessionId);

private:
    struct UserInfo {
        QString userId;
        QString passwordHash;
        QStringList roles;
        QDateTime createdDate;
        QDateTime lastLogin;
        bool isActive;
    };
    
    struct APIKeyInfo {
        QString key;
        QString userId;
        QStringList permissions;
        QDateTime createdDate;
        QDateTime lastUsed;
        bool isActive;
    };
    
    struct SessionInfo {
        QString sessionId;
        QString userId;
        QDateTime createdDate;
        QDateTime lastAccessed;
        QDateTime expirationDate;
    };
    
    QMap<QString, UserInfo> m_users;
    QMap<QString, APIKeyInfo> m_apiKeys;
    QMap<QString, SessionInfo> m_sessions;
    QSet<QString> m_revokedTokens;
    QString m_jwtSecret;
    
    void loadUsers();
    void saveUsers();
    void loadAPIKeys();
    void saveAPIKeys();
    QString hashPassword(const QString& password) const;
    bool verifyPassword(const QString& password, const QString& hash) const;
    QString generateSecureKey() const;
    void cleanupExpiredSessions();
};

// Rate limiter for API requests
class APIRateLimiter : public QObject {
    Q_OBJECT

public:
    explicit APIRateLimiter(QObject* parent = nullptr);
    ~APIRateLimiter() override;

    // Rate limiting
    bool isAllowed(const QString& clientId, const QString& endpoint = "");
    void recordRequest(const QString& clientId, const QString& endpoint = "");
    void setLimit(const QString& endpoint, int requestsPerMinute);
    void setGlobalLimit(int requestsPerMinute);
    int getLimit(const QString& endpoint) const;
    int getRemainingRequests(const QString& clientId, const QString& endpoint = "") const;
    QDateTime getResetTime(const QString& clientId, const QString& endpoint = "") const;
    
    // Client management
    void resetClient(const QString& clientId);
    void blockClient(const QString& clientId, int minutes = 60);
    void unblockClient(const QString& clientId);
    bool isClientBlocked(const QString& clientId) const;
    QStringList getBlockedClients() const;
    
    // Statistics
    int getRequestCount(const QString& clientId, const QString& endpoint = "") const;
    QMap<QString, int> getClientStatistics(const QString& clientId) const;
    QMap<QString, int> getEndpointStatistics() const;
    void clearStatistics();

signals:
    void rateLimitExceeded(const QString& clientId, const QString& endpoint);
    void clientBlocked(const QString& clientId);
    void clientUnblocked(const QString& clientId);

private slots:
    void onCleanupTimer();

private:
    struct RateLimitInfo {
        int requestCount;
        QDateTime windowStart;
        QDateTime lastRequest;
        bool isBlocked;
        QDateTime blockExpiry;
    };
    
    QMap<QString, int> m_endpointLimits;
    int m_globalLimit;
    QMap<QString, QMap<QString, RateLimitInfo>> m_clientLimits; // clientId -> endpoint -> info
    QTimer* m_cleanupTimer;
    
    void cleanupExpiredWindows();
    QString getWindowKey(const QDateTime& timestamp) const;
};

#ifdef QT_WEBSOCKETS_AVAILABLE
// WebSocket handler for real-time communication
class WebSocketHandler : public QObject {
    Q_OBJECT

public:
    explicit WebSocketHandler(QWebSocket* socket, QObject* parent = nullptr);
    ~WebSocketHandler() override;

    // Connection management
    QString clientId() const;
    bool isConnected() const;
    QDateTime connectionTime() const;
    QString clientAddress() const;
    
    // Message handling
    void sendMessage(const QJsonObject& message);
    void sendBinaryMessage(const QByteArray& data);
    void sendPing();
    void close(const QString& reason = "");
    
    // Subscription management
    void subscribe(const QString& topic);
    void unsubscribe(const QString& topic);
    QStringList subscriptions() const;
    bool isSubscribed(const QString& topic) const;
    
    // Authentication
    void authenticate(const QString& token);
    bool isAuthenticated() const;
    QString userId() const;
    QStringList permissions() const;

signals:
    void messageReceived(const QJsonObject& message);
    void binaryMessageReceived(const QByteArray& data);
    void subscribed(const QString& topic);
    void unsubscribed(const QString& topic);
    void authenticated(const QString& userId);
    void disconnected();
    void error(const QString& error);

private slots:
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& data);
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onPongReceived(quint64 elapsedTime, const QByteArray& payload);

private:
    QWebSocket* m_socket;
    QString m_clientId;
    QDateTime m_connectionTime;
    QStringList m_subscriptions;
    bool m_isAuthenticated;
    QString m_userId;
    QStringList m_permissions;
    
    void processMessage(const QJsonObject& message);
    void handleSubscriptionMessage(const QJsonObject& message);
    void handleAuthenticationMessage(const QJsonObject& message);
    QString generateClientId() const;
};
#endif // QT_WEBSOCKETS_AVAILABLE

// API documentation generator
class APIDocumentation : public QObject {
    Q_OBJECT

public:
    explicit APIDocumentation(QObject* parent = nullptr);
    ~APIDocumentation() override;

    // Documentation generation
    QString generateOpenAPISpec(const QList<APIEndpoint>& endpoints, const APIConfig& config);
    QString generateHTMLDocumentation(const QList<APIEndpoint>& endpoints, const APIConfig& config);
    QString generateMarkdownDocumentation(const QList<APIEndpoint>& endpoints, const APIConfig& config);
    void exportDocumentation(const QString& filePath, const QString& format = "html");
    
    // Schema management
    void addSchema(const QString& name, const QJsonObject& schema);
    void removeSchema(const QString& name);
    QJsonObject getSchema(const QString& name) const;
    QStringList getSchemas() const;
    
    // Example management
    void addExample(const QString& endpoint, const QString& method, const QJsonObject& example);
    void removeExample(const QString& endpoint, const QString& method);
    QJsonObject getExample(const QString& endpoint, const QString& method) const;
    
    // Template customization
    void setTemplate(const QString& format, const QString& templateContent);
    QString getTemplate(const QString& format) const;
    void loadTemplateFromFile(const QString& format, const QString& filePath);

signals:
    void documentationGenerated(const QString& format, const QString& content);
    void schemaAdded(const QString& name);
    void schemaRemoved(const QString& name);
    void exampleAdded(const QString& endpoint, const QString& method);

private:
    QMap<QString, QJsonObject> m_schemas;
    QMap<QString, QMap<QString, QJsonObject>> m_examples; // endpoint -> method -> example
    QMap<QString, QString> m_templates;
    
    QString generateEndpointDocumentation(const APIEndpoint& endpoint, const QString& format);
    QString formatParameterDocumentation(const QJsonObject& parameters, const QString& format);
    QString formatSchemaDocumentation(const QJsonObject& schema, const QString& format);
    QString loadDefaultTemplate(const QString& format);
    QString processTemplate(const QString& templateContent, const QMap<QString, QString>& variables);
};

// API management widget
class APIWidget : public QWidget {
    Q_OBJECT

public:
    explicit APIWidget(PluginRESTAPIServer* server, QWidget* parent = nullptr);
    ~APIWidget() override;

    // Display management
    void refreshEndpoints();
    void refreshClients();
    void refreshLogs();
    void refreshStatistics();
    void showEndpointDetails(const QString& path, HttpMethod method);
    
    // Server control
    void startServer();
    void stopServer();
    void restartServer();
    bool isServerRunning() const;

signals:
    void endpointSelected(const QString& path, HttpMethod method);
    void clientSelected(const QString& clientId);
    void serverStartRequested();
    void serverStopRequested();
    void configurationRequested();
    void documentationRequested();

private slots:
    void onEndpointItemClicked();
    void onClientItemClicked();
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onConfigureButtonClicked();
    void onDocumentationButtonClicked();
    void onRefreshButtonClicked();
    void onClearLogsButtonClicked();

private:
    PluginRESTAPIServer* m_server;
    
    // UI components
    QTabWidget* m_tabWidget;
    QTreeWidget* m_endpointsTree;
    QTableWidget* m_clientsTable;
    QTextEdit* m_logsView;
    QWidget* m_statisticsWidget;
    QLabel* m_statusLabel;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_configureButton;
    QPushButton* m_documentationButton;
    
    void setupUI();
    void setupEndpointsTab();
    void setupClientsTab();
    void setupLogsTab();
    void setupStatisticsTab();
    void populateEndpointsTree();
    void populateClientsTable();
    void updateLogsView();
    void updateStatistics();
    void updateServerStatus();
    QTreeWidgetItem* createEndpointItem(const APIEndpoint& endpoint);
    void addClientRow(const QString& clientId, const QJsonObject& clientInfo);
    QString formatLogEntry(const QJsonObject& logEntry);
};

// API configuration dialog
class APIConfigDialog : public QDialog {
    Q_OBJECT

public:
    explicit APIConfigDialog(const APIConfig& config, QWidget* parent = nullptr);
    ~APIConfigDialog() override;

    // Configuration management
    APIConfig getConfiguration() const;
    void setConfiguration(const APIConfig& config);

public slots:
    void accept() override;
    void reject() override;

signals:
    void configurationChanged(const APIConfig& config);

private slots:
    void onSSLToggled();
    void onCORSToggled();
    void onRateLimitToggled();
    void onWebSocketToggled();
    void onBrowseSSLCert();
    void onBrowseSSLKey();
    void onBrowseLogDirectory();
    void onTestConnection();
    void onResetToDefaults();

private:
    APIConfig m_config;
    
    // UI components
    QLineEdit* m_hostEdit;
    QSpinBox* m_portSpinBox;
    QCheckBox* m_sslCheckBox;
    QLineEdit* m_sslCertEdit;
    QLineEdit* m_sslKeyEdit;
    QCheckBox* m_corsCheckBox;
    QLineEdit* m_allowedOriginsEdit;
    QCheckBox* m_rateLimitCheckBox;
    QSpinBox* m_defaultRateLimitSpinBox;
    QCheckBox* m_webSocketCheckBox;
    QSpinBox* m_webSocketPortSpinBox;
    QLineEdit* m_logDirectoryEdit;
    QComboBox* m_logLevelCombo;
    
    void setupUI();
    void setupServerTab();
    void setupSecurityTab();
    void setupLoggingTab();
    void setupAdvancedTab();
    void updateUIFromConfig();
    void updateConfigFromUI();
    void validateConfiguration();
};
