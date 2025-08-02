// PluginStoreWidget.h - Enhanced Plugin Store Interface
#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QTabWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDateEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QMovie>
#include <QPixmap>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <memory>

class PluginStoreItem;
class PluginDetailsWidget;
class PluginCategoryWidget;
class PluginSearchWidget;
class PluginInstaller;
class PluginRatingWidget;
class PluginReviewWidget;

class PluginStoreWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginStoreWidget(QWidget* parent = nullptr);
    ~PluginStoreWidget() override;

    void setStoreUrl(const QString& url);
    void refreshStore();
    void searchPlugins(const QString& query);
    void filterByCategory(const QString& category);
    void sortBy(const QString& criteria, Qt::SortOrder order = Qt::AscendingOrder);

signals:
    void pluginInstallRequested(const QString& pluginId);
    void pluginUninstallRequested(const QString& pluginId);
    void pluginUpdateRequested(const QString& pluginId);
    void pluginDetailsRequested(const QString& pluginId);
    void storeRefreshed();
    void searchCompleted(int resultCount);

public slots:
    void installPlugin(const QString& pluginId);
    void uninstallPlugin(const QString& pluginId);
    void updatePlugin(const QString& pluginId);
    void showPluginDetails(const QString& pluginId);
    void ratePlugin(const QString& pluginId, int rating);
    void reviewPlugin(const QString& pluginId, const QString& review);

private slots:
    void onStoreDataReceived();
    void onSearchTextChanged();
    void onCategoryChanged();
    void onSortChanged();
    void onFilterChanged();
    void onPluginItemClicked();
    void onInstallClicked();
    void onUpdateClicked();
    void onRefreshClicked();
    void onNetworkError(QNetworkReply::NetworkError error);

private:
    void setupUI();
    void setupToolbar();
    void setupSidebar();
    void setupMainContent();
    void setupDetailsPanel();
    void connectSignals();
    void loadStoreData();
    void updatePluginList();
    void applyFilters();
    void showLoadingIndicator(bool show);

    // Network
    QNetworkAccessManager* m_networkManager;
    QString m_storeUrl;
    QJsonArray m_storeData;
    QTimer* m_searchTimer;

    // UI Components
    QSplitter* m_mainSplitter;
    QSplitter* m_contentSplitter;

    // Toolbar
    QFrame* m_toolbarFrame;
    PluginSearchWidget* m_searchWidget;
    QComboBox* m_sortCombo;
    QPushButton* m_refreshBtn;
    QPushButton* m_settingsBtn;

    // Sidebar
    QFrame* m_sidebarFrame;
    PluginCategoryWidget* m_categoryWidget;
    QGroupBox* m_filtersGroup;
    QCheckBox* m_freeOnlyCheck;
    QCheckBox* m_verifiedOnlyCheck;
    QSlider* m_ratingSlider;
    QDateEdit* m_dateFromEdit;
    QDateEdit* m_dateToEdit;

    // Main Content
    QScrollArea* m_contentArea;
    QWidget* m_contentWidget;
    QGridLayout* m_contentLayout;
    QList<PluginStoreItem*> m_pluginItems;
    QLabel* m_statusLabel;
    QProgressBar* m_loadingBar;

    // Details Panel
    PluginDetailsWidget* m_detailsWidget;

    // Models and Filters
    QStandardItemModel* m_pluginModel;
    QSortFilterProxyModel* m_proxyModel;

    // State
    QString m_currentSearch;
    QString m_currentCategory;
    QString m_currentSort = "popularity";
    Qt::SortOrder m_currentOrder = Qt::DescendingOrder;
    bool m_loading = false;
};

// Plugin Store Item Widget
class PluginStoreItem : public QFrame
{
    Q_OBJECT

public:
    explicit PluginStoreItem(const QJsonObject& pluginData, QWidget* parent = nullptr);

    QString pluginId() const { return m_pluginId; }
    QJsonObject pluginData() const { return m_pluginData; }
    bool isInstalled() const { return m_installed; }
    bool hasUpdate() const { return m_hasUpdate; }

    void setInstalled(bool installed);
    void setHasUpdate(bool hasUpdate);
    void updateData(const QJsonObject& data);

signals:
    void clicked();
    void installRequested();
    void updateRequested();
    void detailsRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onInstallClicked();
    void onUpdateClicked();
    void onDetailsClicked();

private:
    void setupUI();
    void updateInstallButton();
    void loadPluginIcon();

    QString m_pluginId;
    QJsonObject m_pluginData;
    bool m_installed = false;
    bool m_hasUpdate = false;
    bool m_hovered = false;

    QLabel* m_iconLabel;
    QLabel* m_nameLabel;
    QLabel* m_authorLabel;
    QLabel* m_descriptionLabel;
    QLabel* m_versionLabel;
    QLabel* m_categoryLabel;
    PluginRatingWidget* m_ratingWidget;
    QPushButton* m_installBtn;
    QPushButton* m_detailsBtn;
    QLabel* m_priceLabel;
    QLabel* m_downloadCountLabel;
};

// Plugin Details Widget
class PluginDetailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginDetailsWidget(QWidget* parent = nullptr);

    void showPlugin(const QJsonObject& pluginData);
    void clearDetails();

signals:
    void installRequested(const QString& pluginId);
    void updateRequested(const QString& pluginId);
    void uninstallRequested(const QString& pluginId);
    void ratingSubmitted(const QString& pluginId, int rating);
    void reviewSubmitted(const QString& pluginId, const QString& review);

private slots:
    void onInstallClicked();
    void onUpdateClicked();
    void onUninstallClicked();
    void onRatingChanged(int rating);
    void onReviewSubmitted();
    void onScreenshotClicked();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupHeaderSection();
    void setupTabWidget();
    void setupActionButtons();
    void updateActionButtons();
    void loadScreenshots();

    QJsonObject m_pluginData;
    QString m_pluginId;
    bool m_installed = false;
    bool m_hasUpdate = false;

    // Header
    QFrame* m_headerFrame;
    QLabel* m_iconLabel;
    QLabel* m_nameLabel;
    QLabel* m_authorLabel;
    QLabel* m_versionLabel;
    QLabel* m_categoryLabel;
    PluginRatingWidget* m_ratingWidget;
    QLabel* m_downloadCountLabel;
    QLabel* m_priceLabel;

    // Tabs
    QTabWidget* m_tabWidget;
    QTextEdit* m_descriptionText;
    QWidget* m_screenshotsWidget;
    QWidget* m_reviewsWidget;
    QWidget* m_changelogWidget;
    QWidget* m_dependenciesWidget;

    // Action Buttons
    QFrame* m_actionFrame;
    QPushButton* m_installBtn;
    QPushButton* m_updateBtn;
    QPushButton* m_uninstallBtn;
    PluginRatingWidget* m_userRatingWidget;
    PluginReviewWidget* m_reviewWidget;
};

// Plugin Category Widget
class PluginCategoryWidget : public QTreeWidget
{
    Q_OBJECT

public:
    explicit PluginCategoryWidget(QWidget* parent = nullptr);

    void setCategories(const QStringList& categories);
    void setCategoryCounts(const QHash<QString, int>& counts);
    QString selectedCategory() const;

signals:
    void categorySelected(const QString& category);

private slots:
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    void setupCategories();
    void updateCounts();

    QHash<QString, int> m_categoryCounts;
    QTreeWidgetItem* m_allItem;
    QHash<QString, QTreeWidgetItem*> m_categoryItems;
};

// Plugin Search Widget
class PluginSearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginSearchWidget(QWidget* parent = nullptr);

    QString searchText() const;
    void setSearchText(const QString& text);
    void clearSearch();

signals:
    void searchChanged(const QString& text);
    void searchSubmitted(const QString& text);

private slots:
    void onTextChanged();
    void onReturnPressed();
    void onClearClicked();

private:
    void setupUI();

    QLineEdit* m_searchEdit;
    QPushButton* m_clearBtn;
    QTimer* m_searchTimer;
};

// Plugin Rating Widget
class PluginRatingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginRatingWidget(QWidget* parent = nullptr);

    void setRating(double rating);
    void setMaxRating(int maxRating);
    void setReadOnly(bool readOnly);
    void setShowText(bool showText);

    double rating() const { return m_rating; }
    int maxRating() const { return m_maxRating; }

signals:
    void ratingChanged(int rating);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void updateHoverRating(const QPoint& pos);
    int getRatingFromPosition(const QPoint& pos);

    double m_rating = 0.0;
    int m_maxRating = 5;
    int m_hoverRating = 0;
    bool m_readOnly = true;
    bool m_showText = true;
    QSize m_starSize = QSize(16, 16);
};

// Plugin Review Widget
class PluginReviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginReviewWidget(QWidget* parent = nullptr);

    void setReviews(const QJsonArray& reviews);
    void addReview(const QJsonObject& review);
    void clearReviews();

signals:
    void reviewSubmitted(const QString& review, int rating);

private slots:
    void onSubmitReview();
    void onShowMoreReviews();

private:
    void setupUI();
    void addReviewWidget(const QJsonObject& review);

    QVBoxLayout* m_reviewsLayout;
    QTextEdit* m_reviewEdit;
    PluginRatingWidget* m_ratingWidget;
    QPushButton* m_submitBtn;
    QPushButton* m_showMoreBtn;
    QList<QWidget*> m_reviewWidgets;
    int m_visibleReviews = 5;
};

// Plugin Installer
class PluginInstaller : public QObject
{
    Q_OBJECT

public:
    explicit PluginInstaller(QObject* parent = nullptr);

    void installPlugin(const QString& pluginId, const QUrl& downloadUrl);
    void updatePlugin(const QString& pluginId, const QUrl& downloadUrl);
    void uninstallPlugin(const QString& pluginId);

    bool isInstalling() const { return m_installing; }
    QString currentPlugin() const { return m_currentPlugin; }

signals:
    void installStarted(const QString& pluginId);
    void installProgress(const QString& pluginId, int percentage);
    void installFinished(const QString& pluginId, bool success);
    void installError(const QString& pluginId, const QString& error);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError error);

private:
    void downloadPlugin(const QUrl& url);
    bool extractPlugin(const QString& archivePath, const QString& extractPath);
    bool validatePlugin(const QString& pluginPath);
    bool installExtractedPlugin(const QString& pluginPath);

    QNetworkAccessManager* m_networkManager;
    QString m_currentPlugin;
    QString m_downloadPath;
    bool m_installing = false;
};
