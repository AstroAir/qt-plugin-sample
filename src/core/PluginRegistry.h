// PluginRegistry.h - Enhanced registry with filtering and sorting
#pragma once
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <QtQml/qqmlregistration.h>
#include <QUuid>
#include <QJsonObject>
#include "PluginInterface.h"

class PluginManager;

class PluginRegistry : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    Q_PROPERTY(QString sortRole READ sortRole WRITE setSortRole NOTIFY sortRoleChanged)
    Q_PROPERTY(Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder NOTIFY sortOrderChanged)

public:
    enum PluginRoles {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        VersionRole,
        AuthorRole,
        EnabledRole,
        TypeRole,
        StatusRole,
        CapabilitiesRole,
        SecurityLevelRole,
        MemoryUsageRole,
        CpuUsageRole,
        LoadTimeRole,
        ErrorCountRole,
        UuidRole,
        FileSizeRole,
        LastUpdateRole
    };

    struct PluginDescriptor {
        QString name;
        QString description;
        QVersionNumber version;
        QString author;
        QString type;
        QString status;
        bool enabled = false;
        QStringList capabilities;
        QString securityLevel;
        qint64 memoryUsage = 0;
        double cpuUsage = 0.0;
        QDateTime loadTime;
        int errorCount = 0;
        QUuid uuid;
        qint64 fileSize = 0;
        QDateTime lastUpdate;
        QJsonObject metadata;
        
        bool matchesFilter(const QString& filter) const;
    };

    explicit PluginRegistry(QObject* parent = nullptr);

    // **QAbstractListModel implementation**
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    // **Filtering and sorting**
    QString filterText() const { return m_filterText; }
    void setFilterText(const QString& text);
    
    QString sortRole() const { return m_sortRole; }
    void setSortRole(const QString& role);
    
    Qt::SortOrder sortOrder() const { return m_sortOrder; }
    void setSortOrder(Qt::SortOrder order);

    // **QML-accessible methods**
    Q_INVOKABLE bool enablePlugin(const QString& name);
    Q_INVOKABLE bool disablePlugin(const QString& name);
    Q_INVOKABLE QJsonObject getPluginInfo(const QString& name) const;
    Q_INVOKABLE QStringList getPluginsByType(const QString& type) const;
    Q_INVOKABLE QStringList getPluginsByCapability(const QString& capability) const;
    Q_INVOKABLE void installPlugin(const QString& filePath);
    Q_INVOKABLE void uninstallPlugin(const QString& name);
    Q_INVOKABLE void updatePlugin(const QString& name);
    Q_INVOKABLE QJsonObject getPluginMetrics(const QString& name) const;
    Q_INVOKABLE void configurePlugin(const QString& name, const QJsonObject& config);
    Q_INVOKABLE QStringList getPluginErrors(const QString& name) const;
    Q_INVOKABLE void clearPluginErrors(const QString& name);

public slots:
    void refreshPlugins();
    void sortPlugins();

signals:
    void countChanged();
    void filterTextChanged();
    void sortRoleChanged();
    void sortOrderChanged();
    void pluginStateChanged(const QString& name, bool enabled);
    void pluginInstalled(const QString& name);
    void pluginUninstalled(const QString& name);
    void pluginUpdated(const QString& name);

private slots:
    void onPluginLoaded(const QString& name);
    void onPluginUnloaded(const QString& name);
    void onPluginStatusChanged(const QString& name, PluginStatus status);

private:
    void updatePluginDescriptor(const QString& name);
    void applyFilter();
    void applySorting();
    QString statusToString(PluginStatus status) const;
    
    PluginManager* m_pluginManager;
    QList<PluginDescriptor> m_plugins;
    QList<PluginDescriptor> m_filteredPlugins;
    QString m_filterText;
    QString m_sortRole = "name";
    Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
};
