// PluginCommunication.h - Inter-Plugin Communication and Event System
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
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QUuid>
#include <QCryptographicHash>
#include <QStringList>
#include <QMap>
#include <QSet>
#include <QQueue>
#include <QStack>
#include <memory>

// Forward declarations
class PluginCommunicationManager;
class EventBus;
class MessageRouter;
class SharedDataManager;
class PluginChannel;
class CommunicationWidget;

// Message types
enum class MessageType {
    Event,          // Event notification
    Request,        // Request for data/action
    Response,       // Response to request
    Broadcast,      // Broadcast message
    Notification,   // System notification
    Command,        // Command execution
    Data,           // Data transfer
    Heartbeat,      // Keep-alive message
    Custom          // Custom message type
};

// Message priority
enum class MessagePriority {
    Low,            // Low priority
    Normal,         // Normal priority
    High,           // High priority
    Critical,       // Critical priority
    Immediate       // Immediate delivery
};

// Delivery mode
enum class DeliveryMode {
    Async,          // Asynchronous delivery
    Sync,           // Synchronous delivery
    Queued,         // Queued delivery
    Broadcast,      // Broadcast to all
    Multicast,      // Multicast to group
    Unicast         // Unicast to specific recipient
};

// Communication protocol
enum class CommunicationProtocol {
    InProcess,      // In-process communication
    LocalSocket,    // Local socket (IPC)
    TCP,            // TCP socket
    UDP,            // UDP socket
    SharedMemory,   // Shared memory
    MessageQueue,   // Message queue
    HTTP,           // HTTP protocol
    WebSocket,      // WebSocket protocol
    Custom          // Custom protocol
};

// Plugin message
struct PluginMessage {
    QString messageId;
    QString senderId;
    QString receiverId;
    QString channelId;
    MessageType type;
    MessagePriority priority;
    DeliveryMode deliveryMode;
    QString subject;
    QJsonObject data;
    QByteArray binaryData;
    QDateTime timestamp;
    QDateTime expiryTime;
    int retryCount;
    int maxRetries;
    QString correlationId;
    QString replyTo;
    QMap<QString, QString> headers;
    QJsonObject metadata;
    
    PluginMessage() = default;
    PluginMessage(const QString& sender, const QString& receiver, MessageType t, const QJsonObject& d)
        : senderId(sender), receiverId(receiver), type(t), priority(MessagePriority::Normal),
          deliveryMode(DeliveryMode::Async), data(d),
          timestamp(QDateTime::currentDateTime()), retryCount(0), maxRetries(3) {
        messageId = generateMessageId();
    }
    
    bool isExpired() const;
    bool canRetry() const;
    QString getTypeString() const;
    QString getPriorityString() const;
    QString getDeliveryModeString() const;
    QByteArray serialize() const;
    static PluginMessage deserialize(const QByteArray& data);
    
private:
    QString generateMessageId() const;
};

// Event information
struct PluginEvent {
    QString eventId;
    QString sourceId;
    QString eventType;
    QString category;
    QJsonObject eventData;
    QDateTime timestamp;
    MessagePriority priority;
    QStringList tags;
    QString description;
    bool isCancellable;
    bool isCancelled;
    QJsonObject metadata;
    
    PluginEvent() = default;
    PluginEvent(const QString& source, const QString& type, const QJsonObject& data)
        : sourceId(source), eventType(type), eventData(data),
          timestamp(QDateTime::currentDateTime()), priority(MessagePriority::Normal),
          isCancellable(false), isCancelled(false) {
        eventId = generateEventId();
    }
    
    void cancel();
    bool isValid() const;
    QString getFullEventType() const;
    
private:
    QString generateEventId() const;
};

// Shared data entry
struct SharedDataEntry {
    QString key;
    QString ownerId;
    QJsonValue value;
    QByteArray binaryValue;
    QString dataType;
    QDateTime createdTime;
    QDateTime modifiedTime;
    QDateTime accessedTime;
    QStringList readPermissions;
    QStringList writePermissions;
    bool isReadOnly;
    bool isPersistent;
    int version;
    QString description;
    QJsonObject metadata;
    
    SharedDataEntry() = default;
    SharedDataEntry(const QString& k, const QString& owner, const QJsonValue& v)
        : key(k), ownerId(owner), value(v), createdTime(QDateTime::currentDateTime()),
          modifiedTime(QDateTime::currentDateTime()), accessedTime(QDateTime::currentDateTime()),
          isReadOnly(false), isPersistent(false), version(1) {}
    
    bool canRead(const QString& pluginId) const;
    bool canWrite(const QString& pluginId) const;
    void updateValue(const QJsonValue& newValue);
    void recordAccess();
    QString getDataTypeString() const;
};

// Communication channel
struct CommunicationChannel {
    QString channelId;
    QString name;
    QString description;
    QString ownerId;
    CommunicationProtocol protocol;
    QStringList subscribers;
    QStringList publishers;
    bool isPrivate;
    bool isPersistent;
    int maxMessageSize;
    int maxQueueSize;
    QDateTime createdTime;
    QDateTime lastActivity;
    QJsonObject configuration;
    QJsonObject metadata;
    
    CommunicationChannel() = default;
    CommunicationChannel(const QString& id, const QString& n, const QString& owner)
        : channelId(id), name(n), ownerId(owner), protocol(CommunicationProtocol::InProcess),
          isPrivate(false), isPersistent(false), maxMessageSize(1024*1024), maxQueueSize(1000),
          createdTime(QDateTime::currentDateTime()), lastActivity(QDateTime::currentDateTime()) {}
    
    bool canSubscribe(const QString& pluginId) const;
    bool canPublish(const QString& pluginId) const;
    void addSubscriber(const QString& pluginId);
    void removeSubscriber(const QString& pluginId);
    void addPublisher(const QString& pluginId);
    void removePublisher(const QString& pluginId);
    void updateActivity();
};

// Main communication manager
class PluginCommunicationManager : public QObject {
    Q_OBJECT

public:
    explicit PluginCommunicationManager(QObject* parent = nullptr);
    ~PluginCommunicationManager() override;

    // Message handling
    QString sendMessage(const PluginMessage& message);
    void sendBroadcast(const QString& senderId, const QString& subject, const QJsonObject& data);
    void sendNotification(const QString& senderId, const QString& receiverId, const QString& message);
    bool deliverMessage(const QString& messageId);
    void cancelMessage(const QString& messageId);
    PluginMessage getMessage(const QString& messageId) const;
    QList<PluginMessage> getPendingMessages(const QString& pluginId) const;
    
    // Event system
    void publishEvent(const PluginEvent& event);
    void subscribeToEvent(const QString& pluginId, const QString& eventType);
    void unsubscribeFromEvent(const QString& pluginId, const QString& eventType);
    QStringList getEventSubscriptions(const QString& pluginId) const;
    QList<PluginEvent> getEventHistory(const QString& eventType = "", int maxEvents = 100) const;
    void clearEventHistory(const QString& eventType = "");
    
    // Channel management
    QString createChannel(const QString& name, const QString& ownerId, CommunicationProtocol protocol = CommunicationProtocol::InProcess);
    void deleteChannel(const QString& channelId);
    void subscribeToChannel(const QString& pluginId, const QString& channelId);
    void unsubscribeFromChannel(const QString& pluginId, const QString& channelId);
    void publishToChannel(const QString& channelId, const PluginMessage& message);
    CommunicationChannel getChannel(const QString& channelId) const;
    QList<CommunicationChannel> getChannels(const QString& pluginId = "") const;
    
    // Shared data management
    void setSharedData(const QString& key, const QString& ownerId, const QJsonValue& value);
    QJsonValue getSharedData(const QString& key, const QString& requesterId) const;
    void removeSharedData(const QString& key, const QString& requesterId);
    bool hasSharedData(const QString& key) const;
    QStringList getSharedDataKeys(const QString& pluginId = "") const;
    void setDataPermissions(const QString& key, const QStringList& readPermissions, const QStringList& writePermissions);
    
    // Plugin registration
    void registerPlugin(const QString& pluginId, const QString& name = "");
    void unregisterPlugin(const QString& pluginId);
    bool isPluginRegistered(const QString& pluginId) const;
    QStringList getRegisteredPlugins() const;
    void setPluginStatus(const QString& pluginId, const QString& status);
    QString getPluginStatus(const QString& pluginId) const;
    
    // Request-response pattern
    QString sendRequest(const QString& senderId, const QString& receiverId, const QString& request, const QJsonObject& parameters = QJsonObject());
    void sendResponse(const QString& requestId, const QJsonObject& response);
    QJsonObject waitForResponse(const QString& requestId, int timeoutMs = 5000);
    void setRequestHandler(const QString& pluginId, std::function<QJsonObject(const QString&, const QJsonObject&)> handler);
    
    // Configuration
    void setMaxMessageSize(int bytes);
    int maxMessageSize() const;
    void setMaxQueueSize(int messages);
    int maxQueueSize() const;
    void setMessageTimeout(int seconds);
    int messageTimeout() const;
    void setRetryAttempts(int attempts);
    int retryAttempts() const;
    
    // Statistics and monitoring
    QJsonObject getCommunicationStatistics() const;
    QJsonObject getPluginStatistics(const QString& pluginId) const;
    QJsonObject getChannelStatistics(const QString& channelId) const;
    int getMessageCount(const QString& pluginId = "") const;
    int getEventCount(const QString& eventType = "") const;
    void clearStatistics();

signals:
    void messageReceived(const PluginMessage& message);
    void messageSent(const PluginMessage& message);
    void messageDelivered(const QString& messageId);
    void messageExpired(const QString& messageId);
    void eventPublished(const PluginEvent& event);
    void eventReceived(const QString& pluginId, const PluginEvent& event);
    void channelCreated(const QString& channelId);
    void channelDeleted(const QString& channelId);
    void pluginSubscribed(const QString& pluginId, const QString& channelId);
    void pluginUnsubscribed(const QString& pluginId, const QString& channelId);
    void sharedDataChanged(const QString& key, const QJsonValue& value);
    void pluginRegistered(const QString& pluginId);
    void pluginUnregistered(const QString& pluginId);
    void communicationError(const QString& error);

public slots:
    void processMessageQueue();
    void cleanupExpiredMessages();
    void showCommunicationWidget();

private slots:
    void onMessageTimer();
    void onCleanupTimer();
    void onHeartbeatTimer();

private:
    struct CommunicationManagerPrivate;
    std::unique_ptr<CommunicationManagerPrivate> d;
    
    void initializeManager();
    void loadConfiguration();
    void saveConfiguration();
    void setupTimers();
    void processMessage(const PluginMessage& message);
    void deliverEvent(const PluginEvent& event);
    void routeMessage(const PluginMessage& message);
    void handleRequest(const PluginMessage& message);
    void handleResponse(const PluginMessage& message);
    void updateStatistics(const PluginMessage& message);
    void logCommunication(const QString& action, const QJsonObject& details);
    QString generateCorrelationId() const;
};

// Event bus for publish-subscribe messaging
class EventBus : public QObject {
    Q_OBJECT

public:
    explicit EventBus(QObject* parent = nullptr);
    ~EventBus() override;

    // Event publishing
    void publish(const PluginEvent& event);
    void publish(const QString& eventType, const QJsonObject& data, const QString& sourceId = "");
    void publishAsync(const PluginEvent& event);
    void publishDelayed(const PluginEvent& event, int delayMs);
    
    // Event subscription
    void subscribe(const QString& subscriberId, const QString& eventType, std::function<void(const PluginEvent&)> handler);
    void subscribe(const QString& subscriberId, const QStringList& eventTypes, std::function<void(const PluginEvent&)> handler);
    void unsubscribe(const QString& subscriberId, const QString& eventType);
    void unsubscribeAll(const QString& subscriberId);
    
    // Event filtering
    void addEventFilter(const QString& filterId, std::function<bool(const PluginEvent&)> filter);
    void removeEventFilter(const QString& filterId);
    void setGlobalFilter(std::function<bool(const PluginEvent&)> filter);
    
    // Event transformation
    void addEventTransformer(const QString& transformerId, std::function<PluginEvent(const PluginEvent&)> transformer);
    void removeEventTransformer(const QString& transformerId);
    
    // Subscription management
    QStringList getSubscribers(const QString& eventType) const;
    QStringList getSubscriptions(const QString& subscriberId) const;
    int getSubscriberCount(const QString& eventType) const;
    
    // Event history
    void setEventHistoryEnabled(bool enabled);
    bool isEventHistoryEnabled() const;
    void setMaxHistorySize(int maxEvents);
    int maxHistorySize() const;
    QList<PluginEvent> getEventHistory(const QString& eventType = "") const;
    void clearEventHistory();

signals:
    void eventPublished(const PluginEvent& event);
    void eventDelivered(const QString& subscriberId, const PluginEvent& event);
    void subscriberAdded(const QString& subscriberId, const QString& eventType);
    void subscriberRemoved(const QString& subscriberId, const QString& eventType);

private slots:
    void onDelayedEventTimer();

private:
    struct EventSubscription {
        QString subscriberId;
        QString eventType;
        std::function<void(const PluginEvent&)> handler;
        QDateTime subscriptionTime;
        int eventCount;
    };
    
    QMultiMap<QString, EventSubscription> m_subscriptions; // eventType -> subscriptions
    QMap<QString, std::function<bool(const PluginEvent&)>> m_filters;
    QMap<QString, std::function<PluginEvent(const PluginEvent&)>> m_transformers;
    std::function<bool(const PluginEvent&)> m_globalFilter;
    QList<PluginEvent> m_eventHistory;
    bool m_eventHistoryEnabled;
    int m_maxHistorySize;
    QTimer* m_delayedEventTimer;
    QQueue<QPair<PluginEvent, QDateTime>> m_delayedEvents;
    
    void deliverEvent(const PluginEvent& event);
    bool passesFilters(const PluginEvent& event);
    PluginEvent applyTransformers(const PluginEvent& event);
    void addToHistory(const PluginEvent& event);
    void processDelayedEvents();
};

// Message router for intelligent message routing
class MessageRouter : public QObject {
    Q_OBJECT

public:
    explicit MessageRouter(QObject* parent = nullptr);
    ~MessageRouter() override;

    // Routing configuration
    void addRoute(const QString& routeId, const QString& pattern, const QString& destination);
    void removeRoute(const QString& routeId);
    void updateRoute(const QString& routeId, const QString& pattern, const QString& destination);
    QStringList getRoutes() const;
    
    // Message routing
    QStringList routeMessage(const PluginMessage& message);
    QString findBestRoute(const PluginMessage& message);
    bool canRoute(const PluginMessage& message, const QString& destination);
    
    // Load balancing
    void setLoadBalancingStrategy(const QString& strategy); // round-robin, least-connections, random
    QString loadBalancingStrategy() const;
    void addDestination(const QString& group, const QString& destination);
    void removeDestination(const QString& group, const QString& destination);
    QString selectDestination(const QString& group);
    
    // Routing rules
    void addRoutingRule(const QString& ruleId, std::function<bool(const PluginMessage&)> condition, const QString& action);
    void removeRoutingRule(const QString& ruleId);
    void setDefaultRoute(const QString& destination);
    QString defaultRoute() const;
    
    // Statistics
    QJsonObject getRoutingStatistics() const;
    int getRouteUsageCount(const QString& routeId) const;
    void clearStatistics();

signals:
    void messageRouted(const PluginMessage& message, const QString& destination);
    void routingFailed(const PluginMessage& message, const QString& reason);
    void routeAdded(const QString& routeId);
    void routeRemoved(const QString& routeId);

private:
    struct Route {
        QString routeId;
        QString pattern;
        QString destination;
        QDateTime createdTime;
        int usageCount;
    };
    
    struct RoutingRule {
        QString ruleId;
        std::function<bool(const PluginMessage&)> condition;
        QString action;
        int priority;
    };
    
    QList<Route> m_routes;
    QList<RoutingRule> m_rules;
    QMap<QString, QStringList> m_destinationGroups;
    QMap<QString, int> m_destinationConnections;
    QString m_loadBalancingStrategy;
    QString m_defaultRoute;
    
    bool matchesPattern(const QString& text, const QString& pattern);
    void updateRouteStatistics(const QString& routeId);
    QString applyLoadBalancing(const QStringList& destinations);
};

// Shared data manager for inter-plugin data sharing
class SharedDataManager : public QObject {
    Q_OBJECT

public:
    explicit SharedDataManager(QObject* parent = nullptr);
    ~SharedDataManager() override;

    // Data operations
    void setData(const QString& key, const QString& ownerId, const QJsonValue& value);
    QJsonValue getData(const QString& key, const QString& requesterId) const;
    void removeData(const QString& key, const QString& requesterId);
    bool hasData(const QString& key) const;
    QStringList getKeys(const QString& pluginId = "") const;
    
    // Binary data operations
    void setBinaryData(const QString& key, const QString& ownerId, const QByteArray& data);
    QByteArray getBinaryData(const QString& key, const QString& requesterId) const;
    
    // Permissions
    void setPermissions(const QString& key, const QStringList& readPermissions, const QStringList& writePermissions);
    void addReadPermission(const QString& key, const QString& pluginId);
    void addWritePermission(const QString& key, const QString& pluginId);
    void removeReadPermission(const QString& key, const QString& pluginId);
    void removeWritePermission(const QString& key, const QString& pluginId);
    bool canRead(const QString& key, const QString& pluginId) const;
    bool canWrite(const QString& key, const QString& pluginId) const;
    
    // Data properties
    void setReadOnly(const QString& key, bool readOnly);
    bool isReadOnly(const QString& key) const;
    void setPersistent(const QString& key, bool persistent);
    bool isPersistent(const QString& key) const;
    void setDescription(const QString& key, const QString& description);
    QString getDescription(const QString& key) const;
    
    // Data monitoring
    void watchData(const QString& key, const QString& watcherId, std::function<void(const QString&, const QJsonValue&)> callback);
    void unwatchData(const QString& key, const QString& watcherId);
    QStringList getWatchers(const QString& key) const;
    
    // Data synchronization
    void lockData(const QString& key, const QString& lockerId);
    void unlockData(const QString& key, const QString& lockerId);
    bool isDataLocked(const QString& key) const;
    QString getDataLocker(const QString& key) const;
    
    // Persistence
    void savePersistentData();
    void loadPersistentData();
    void setPersistenceDirectory(const QString& directory);
    QString persistenceDirectory() const;

signals:
    void dataChanged(const QString& key, const QJsonValue& value);
    void dataAdded(const QString& key);
    void dataRemoved(const QString& key);
    void dataLocked(const QString& key, const QString& lockerId);
    void dataUnlocked(const QString& key);
    void permissionsChanged(const QString& key);

private:
    QMap<QString, SharedDataEntry> m_data;
    QMap<QString, QMap<QString, std::function<void(const QString&, const QJsonValue&)>>> m_watchers; // key -> watcherId -> callback
    QMap<QString, QString> m_dataLocks; // key -> lockerId
    QString m_persistenceDirectory;
    
    void notifyWatchers(const QString& key, const QJsonValue& value);
    void validatePermissions(const QString& key, const QString& pluginId, bool write) const;
    QString getDataFilePath(const QString& key) const;
    void ensurePersistenceDirectory();
};

// Plugin channel for dedicated communication
class PluginChannel : public QObject {
    Q_OBJECT

public:
    explicit PluginChannel(const QString& channelId, const QString& name, QObject* parent = nullptr);
    ~PluginChannel() override;

    // Channel information
    QString channelId() const;
    QString name() const;
    void setName(const QString& name);
    QString description() const;
    void setDescription(const QString& description);
    
    // Message handling
    void sendMessage(const PluginMessage& message);
    void broadcastMessage(const PluginMessage& message);
    QList<PluginMessage> getMessages(const QString& pluginId) const;
    void clearMessages(const QString& pluginId);
    
    // Subscription management
    void subscribe(const QString& pluginId);
    void unsubscribe(const QString& pluginId);
    bool isSubscribed(const QString& pluginId) const;
    QStringList getSubscribers() const;
    int getSubscriberCount() const;
    
    // Channel configuration
    void setMaxMessageSize(int bytes);
    int maxMessageSize() const;
    void setMaxQueueSize(int messages);
    int maxQueueSize() const;
    void setPrivate(bool isPrivate);
    bool isPrivate() const;
    void setPersistent(bool persistent);
    bool isPersistent() const;
    
    // Message filtering
    void addMessageFilter(std::function<bool(const PluginMessage&)> filter);
    void clearMessageFilters();
    
    // Statistics
    int getMessageCount() const;
    QDateTime getLastActivity() const;
    QJsonObject getStatistics() const;

signals:
    void messageReceived(const PluginMessage& message);
    void subscriberAdded(const QString& pluginId);
    void subscriberRemoved(const QString& pluginId);
    void channelConfigurationChanged();

private:
    QString m_channelId;
    QString m_name;
    QString m_description;
    QStringList m_subscribers;
    QMap<QString, QQueue<PluginMessage>> m_messageQueues; // pluginId -> messages
    QList<std::function<bool(const PluginMessage&)>> m_messageFilters;
    int m_maxMessageSize;
    int m_maxQueueSize;
    bool m_isPrivate;
    bool m_isPersistent;
    QDateTime m_lastActivity;
    int m_totalMessageCount;
    
    bool passesFilters(const PluginMessage& message);
    void updateActivity();
    void enforceQueueLimits(const QString& pluginId);
};

// Communication widget for monitoring and management
class CommunicationWidget : public QWidget {
    Q_OBJECT

public:
    explicit CommunicationWidget(PluginCommunicationManager* manager, QWidget* parent = nullptr);
    ~CommunicationWidget() override;

    // Display management
    void refreshMessages();
    void refreshEvents();
    void refreshChannels();
    void refreshSharedData();
    void refreshPlugins();
    
    // Filtering and search
    void setMessageFilter(const QString& filter);
    void setEventFilter(const QString& filter);
    void setPluginFilter(const QString& filter);

signals:
    void messageSelected(const PluginMessage& message);
    void eventSelected(const PluginEvent& event);
    void channelSelected(const QString& channelId);
    void pluginSelected(const QString& pluginId);
    void sendMessageRequested();
    void publishEventRequested();
    void createChannelRequested();

private slots:
    void onMessageItemClicked();
    void onEventItemClicked();
    void onChannelItemClicked();
    void onPluginItemClicked();
    void onSendMessageClicked();
    void onPublishEventClicked();
    void onCreateChannelClicked();
    void onRefreshClicked();
    void onClearClicked();

private:
    PluginCommunicationManager* m_manager;
    
    // UI components
    QTabWidget* m_tabWidget;
    QTableWidget* m_messagesTable;
    QTableWidget* m_eventsTable;
    QTreeWidget* m_channelsTree;
    QTableWidget* m_sharedDataTable;
    QListWidget* m_pluginsList;
    QTextEdit* m_detailsView;
    QLineEdit* m_filterEdit;
    
    void setupUI();
    void setupMessagesTab();
    void setupEventsTab();
    void setupChannelsTab();
    void setupSharedDataTab();
    void setupPluginsTab();
    void populateMessagesTable();
    void populateEventsTable();
    void populateChannelsTree();
    void populateSharedDataTable();
    void populatePluginsList();
    void updateDetailsView(const QString& content);
    void addMessageRow(const PluginMessage& message);
    void addEventRow(const PluginEvent& event);
    QTreeWidgetItem* createChannelItem(const CommunicationChannel& channel);
    void addSharedDataRow(const QString& key, const SharedDataEntry& entry);
    QString formatMessageDetails(const PluginMessage& message);
    QString formatEventDetails(const PluginEvent& event);
};
