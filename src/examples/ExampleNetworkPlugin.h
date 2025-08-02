// ExampleNetworkPlugin.h - Example Network Plugin demonstrating network operations
#pragma once

#include "../core/PluginInterface.h"
#include "../core/AdvancedInterfaces.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QVersionNumber>
#include <QUrl>
#include <QUrlQuery>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QHostInfo>
#include <QNetworkInterface>
#include <memory>

class HttpClient;
class WebSocketClient;
class TcpServer;
class UdpClient;
class NetworkDiscovery;

class ExampleNetworkPlugin : public QObject, public INetworkPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.IPlugin/2.0" FILE "ExampleNetworkPlugin.json")
    Q_INTERFACES(IPlugin INetworkPlugin)

public:
    ExampleNetworkPlugin(QObject* parent = nullptr);
    ~ExampleNetworkPlugin() override;

    // IPlugin interface
    QString name() const override { return "Example Network Plugin"; }
    QString description() const override { return "Demonstrates network operations including HTTP requests, WebSockets, TCP/UDP communication, and network discovery"; }
    QVersionNumber version() const override { return QVersionNumber(1, 0, 0); }
    QString author() const override { return "Plugin Framework Team"; }
    QUuid uuid() const override { return QUuid("{11223344-5566-7788-99aa-bbccddeeff00}"); }
    QString category() const override { return "Network"; }
    QString homepage() const override { return "https://example.com/network-plugin"; }
    QString license() const override { return "MIT"; }

    PluginCapabilities capabilities() const override {
        return PluginCapability::Network | PluginCapability::Threading | PluginCapability::Configuration;
    }

    bool initialize() override;
    void cleanup() override;
    bool isInitialized() const override { return m_initialized; }
    PluginStatus status() const override { return m_status; }

    // Configuration
    QJsonObject defaultConfiguration() const override;
    bool configure(const QJsonObject& config) override;
    QJsonObject currentConfiguration() const override;

    // INetworkPlugin interface
    bool sendRequest(const QNetworkRequest& request, const QByteArray& data = QByteArray()) override;
    bool sendGetRequest(const QUrl& url, const QVariantMap& headers = {}) override;
    bool sendPostRequest(const QUrl& url, const QByteArray& data, const QVariantMap& headers = {}) override;
    QNetworkReply* createRequest(QNetworkAccessManager::Operation operation, const QNetworkRequest& request, const QByteArray& data = QByteArray()) override;
    
    bool startServer(int port) override;
    bool stopServer() override;
    bool isServerRunning() const override { return m_serverRunning; }
    int serverPort() const override { return m_serverPort; }
    
    QStringList supportedProtocols() const override;
    bool supportsProtocol(const QString& protocol) const override;

    // Commands
    QVariant executeCommand(const QString& command, const QVariantMap& params = {}) override;
    QStringList availableCommands() const override;

signals:
    void requestFinished(const QString& requestId, const QJsonObject& response);
    void requestError(const QString& requestId, const QString& error);
    void serverStarted(int port);
    void serverStopped();
    void clientConnected(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void dataReceived(const QString& clientId, const QByteArray& data);
    void networkDiscovered(const QJsonObject& networkInfo);

public slots:
    // HTTP operations
    void sendHttpGet(const QString& url, const QVariantMap& headers = {});
    void sendHttpPost(const QString& url, const QJsonObject& data, const QVariantMap& headers = {});
    void sendHttpPut(const QString& url, const QJsonObject& data, const QVariantMap& headers = {});
    void sendHttpDelete(const QString& url, const QVariantMap& headers = {});
    
    // WebSocket operations
    void connectWebSocket(const QString& url);
    void disconnectWebSocket();
    void sendWebSocketMessage(const QJsonObject& message);
    
    // TCP operations
    void startTcpServer(int port);
    void stopTcpServer();
    void connectTcpClient(const QString& host, int port);
    void disconnectTcpClient();
    void sendTcpData(const QByteArray& data);
    
    // UDP operations
    void sendUdpData(const QString& host, int port, const QByteArray& data);
    void startUdpListener(int port);
    void stopUdpListener();
    
    // Network discovery
    void discoverNetworkDevices();
    void scanPortRange(const QString& host, int startPort, int endPort);
    void lookupHost(const QString& hostname);

private slots:
    void onHttpRequestFinished();
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketMessageReceived(const QString& message);
    void onTcpClientConnected();
    void onTcpClientDisconnected();
    void onTcpDataReceived();
    void onUdpDataReceived();
    void onNetworkDiscoveryFinished();
    void onHostLookupFinished(const QHostInfo& hostInfo);

private:
    void setupNetworkComponents();
    void cleanupNetworkComponents();
    QNetworkRequest createRequest(const QString& url, const QVariantMap& headers = {});
    QString generateRequestId();
    void logNetworkActivity(const QString& activity);

    bool m_initialized = false;
    bool m_serverRunning = false;
    PluginStatus m_status = PluginStatus::Unknown;
    QJsonObject m_configuration;
    int m_serverPort = 8080;

    // Network components
    std::unique_ptr<HttpClient> m_httpClient;
    std::unique_ptr<WebSocketClient> m_webSocketClient;
    std::unique_ptr<TcpServer> m_tcpServer;
    std::unique_ptr<UdpClient> m_udpClient;
    std::unique_ptr<NetworkDiscovery> m_networkDiscovery;

    // Request tracking
    QHash<QNetworkReply*, QString> m_pendingRequests;
    QStringList m_activityLog;
    int m_requestCounter = 0;
};

// HTTP Client Class
class HttpClient : public QObject
{
    Q_OBJECT

public:
    explicit HttpClient(QObject* parent = nullptr);
    ~HttpClient() override;

    void setDefaultHeaders(const QVariantMap& headers);
    void setTimeout(int timeoutMs);
    void setUserAgent(const QString& userAgent);

    QString sendGet(const QString& url, const QVariantMap& headers = {});
    QString sendPost(const QString& url, const QJsonObject& data, const QVariantMap& headers = {});
    QString sendPut(const QString& url, const QJsonObject& data, const QVariantMap& headers = {});
    QString sendDelete(const QString& url, const QVariantMap& headers = {});

signals:
    void requestFinished(const QString& requestId, const QJsonObject& response);
    void requestError(const QString& requestId, const QString& error);

private slots:
    void onReplyFinished();
    void onReplyError(QNetworkReply::NetworkError error);

private:
    QNetworkRequest createRequest(const QString& url, const QVariantMap& headers);
    QString generateRequestId();

    QNetworkAccessManager* m_manager = nullptr;
    QVariantMap m_defaultHeaders;
    QString m_userAgent = "ExampleNetworkPlugin/1.0";
    int m_timeout = 30000;
    int m_requestCounter = 0;
    QHash<QNetworkReply*, QString> m_pendingRequests;
};

// WebSocket Client Class
class WebSocketClient : public QObject
{
    Q_OBJECT

public:
    explicit WebSocketClient(QObject* parent = nullptr);
    ~WebSocketClient() override;

    void connectToServer(const QString& url);
    void disconnect();
    void sendMessage(const QJsonObject& message);
    void sendTextMessage(const QString& message);
    void sendBinaryMessage(const QByteArray& data);

    bool isConnected() const;
    QUrl serverUrl() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QJsonObject& message);
    void textMessageReceived(const QString& message);
    void binaryMessageReceived(const QByteArray& data);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& data);
    void onError(QAbstractSocket::SocketError error);

private:
    QWebSocket* m_webSocket = nullptr;
    QUrl m_serverUrl;
};

// TCP Server Class
class TcpServer : public QObject
{
    Q_OBJECT

public:
    explicit TcpServer(QObject* parent = nullptr);
    ~TcpServer() override;

    bool startServer(int port);
    void stopServer();
    bool isListening() const;
    int serverPort() const;
    QStringList connectedClients() const;

    void sendDataToClient(const QString& clientId, const QByteArray& data);
    void sendDataToAllClients(const QByteArray& data);
    void disconnectClient(const QString& clientId);

signals:
    void serverStarted(int port);
    void serverStopped();
    void clientConnected(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void dataReceived(const QString& clientId, const QByteArray& data);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientDataReceived();

private:
    QString generateClientId();

    QTcpServer* m_server = nullptr;
    QHash<QString, QTcpSocket*> m_clients;
    QHash<QTcpSocket*, QString> m_clientIds;
    int m_clientCounter = 0;
};

// UDP Client Class
class UdpClient : public QObject
{
    Q_OBJECT

public:
    explicit UdpClient(QObject* parent = nullptr);
    ~UdpClient() override;

    bool startListener(int port);
    void stopListener();
    void sendData(const QString& host, int port, const QByteArray& data);
    void sendBroadcast(int port, const QByteArray& data);

    bool isListening() const;
    int listenerPort() const;

signals:
    void dataReceived(const QString& sender, int senderPort, const QByteArray& data);
    void dataSent(const QString& host, int port, int bytesWritten);
    void errorOccurred(const QString& error);

private slots:
    void onDataReceived();

private:
    QUdpSocket* m_socket = nullptr;
    int m_listenerPort = 0;
};

// Network Discovery Class
class NetworkDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit NetworkDiscovery(QObject* parent = nullptr);

    void discoverDevices();
    void scanPortRange(const QString& host, int startPort, int endPort);
    void lookupHost(const QString& hostname);
    QStringList getNetworkInterfaces();

signals:
    void deviceDiscovered(const QJsonObject& deviceInfo);
    void portScanCompleted(const QString& host, const QList<int>& openPorts);
    void hostLookupCompleted(const QString& hostname, const QStringList& addresses);
    void discoveryFinished();

private slots:
    void onPingScanFinished();
    void onPortScanFinished();
    void onHostLookupFinished(const QHostInfo& hostInfo);

private:
    void pingHost(const QString& host);
    void scanPort(const QString& host, int port);

    QTimer* m_discoveryTimer = nullptr;
    QStringList m_hostsToScan;
    QList<int> m_portsToScan;
    QString m_currentHost;
    int m_currentPortIndex = 0;
    QList<int> m_openPorts;
};
