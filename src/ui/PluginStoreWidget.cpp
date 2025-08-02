// PluginStoreWidget.cpp - Implementation of enhanced plugin store
#include "PluginStoreWidget.h"
#include <QApplication>
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
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDateTime>

PluginStoreWidget::PluginStoreWidget(QWidget* parent)
    : QWidget(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_searchTimer(new QTimer(this))
    , m_pluginModel(new QStandardItemModel(this))
    , m_proxyModel(new QSortFilterProxyModel(this))
{
    setupUI();
    connectSignals();
    
    // Setup search timer
    m_searchTimer->setSingleShot(true);
    m_searchTimer->setInterval(500); // 500ms delay
    connect(m_searchTimer, &QTimer::timeout, this, &PluginStoreWidget::onSearchTextChanged);
    
    // Setup proxy model
    m_proxyModel->setSourceModel(m_pluginModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    
    // Load initial data
    loadStoreData();
}

PluginStoreWidget::~PluginStoreWidget() = default;

void PluginStoreWidget::setStoreUrl(const QString& url)
{
    m_storeUrl = url;
}

void PluginStoreWidget::refreshStore()
{
    loadStoreData();
}

void PluginStoreWidget::searchPlugins(const QString& query)
{
    m_currentSearch = query;
    if (m_searchWidget) {
        m_searchWidget->setSearchText(query);
    }
    applyFilters();
}

void PluginStoreWidget::filterByCategory(const QString& category)
{
    m_currentCategory = category;
    applyFilters();
}

void PluginStoreWidget::sortBy(const QString& criteria, Qt::SortOrder order)
{
    m_currentSort = criteria;
    m_currentOrder = order;
    if (m_sortCombo) {
        // Update sort combo to reflect current sort
        QString sortText = criteria;
        if (order == Qt::DescendingOrder) {
            sortText += " (Desc)";
        }
        m_sortCombo->setCurrentText(sortText);
    }
    updatePluginList();
}

void PluginStoreWidget::installPlugin(const QString& pluginId)
{
    qDebug() << "Installing plugin:" << pluginId;
    emit pluginInstallRequested(pluginId);
}

void PluginStoreWidget::uninstallPlugin(const QString& pluginId)
{
    qDebug() << "Uninstalling plugin:" << pluginId;
    emit pluginUninstallRequested(pluginId);
}

void PluginStoreWidget::updatePlugin(const QString& pluginId)
{
    qDebug() << "Updating plugin:" << pluginId;
    emit pluginUpdateRequested(pluginId);
}

void PluginStoreWidget::showPluginDetails(const QString& pluginId)
{
    qDebug() << "Showing details for plugin:" << pluginId;
    emit pluginDetailsRequested(pluginId);
    
    // Find plugin data and show in details panel
    for (const auto& item : m_pluginItems) {
        if (item->pluginId() == pluginId) {
            if (m_detailsWidget) {
                m_detailsWidget->showPlugin(item->pluginData());
            }
            break;
        }
    }
}

void PluginStoreWidget::ratePlugin(const QString& pluginId, int rating)
{
    qDebug() << "Rating plugin:" << pluginId << "with rating:" << rating;
    // TODO: Implement rating submission
}

void PluginStoreWidget::reviewPlugin(const QString& pluginId, const QString& review)
{
    qDebug() << "Reviewing plugin:" << pluginId << "with review:" << review;
    // TODO: Implement review submission
}

void PluginStoreWidget::onStoreDataReceived()
{
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Failed to load store data:" << reply->errorString();
        showLoadingIndicator(false);
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse store data:" << error.errorString();
        showLoadingIndicator(false);
        return;
    }
    
    if (doc.isArray()) {
        m_storeData = doc.array();
    } else if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("plugins")) {
            m_storeData = obj["plugins"].toArray();
        }
    }
    
    updatePluginList();
    showLoadingIndicator(false);
    emit storeRefreshed();
}

void PluginStoreWidget::onSearchTextChanged()
{
    if (m_searchWidget) {
        m_currentSearch = m_searchWidget->searchText();
        applyFilters();
    }
}

void PluginStoreWidget::onCategoryChanged()
{
    if (m_categoryWidget) {
        m_currentCategory = m_categoryWidget->selectedCategory();
        applyFilters();
    }
}

void PluginStoreWidget::onSortChanged()
{
    if (m_sortCombo) {
        QString sortText = m_sortCombo->currentText();
        if (sortText.contains("(Desc)")) {
            m_currentSort = sortText.replace(" (Desc)", "");
            m_currentOrder = Qt::DescendingOrder;
        } else {
            m_currentSort = sortText;
            m_currentOrder = Qt::AscendingOrder;
        }
        updatePluginList();
    }
}

void PluginStoreWidget::onFilterChanged()
{
    applyFilters();
}

void PluginStoreWidget::onPluginItemClicked()
{
    auto* item = qobject_cast<PluginStoreItem*>(sender());
    if (item) {
        showPluginDetails(item->pluginId());
    }
}

void PluginStoreWidget::onInstallClicked()
{
    auto* item = qobject_cast<PluginStoreItem*>(sender());
    if (item) {
        installPlugin(item->pluginId());
    }
}

void PluginStoreWidget::onUpdateClicked()
{
    auto* item = qobject_cast<PluginStoreItem*>(sender());
    if (item) {
        updatePlugin(item->pluginId());
    }
}

void PluginStoreWidget::onRefreshClicked()
{
    refreshStore();
}

void PluginStoreWidget::onNetworkError(QNetworkReply::NetworkError error)
{
    qWarning() << "Network error:" << error;
    showLoadingIndicator(false);
}

void PluginStoreWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Setup main splitter
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    setupToolbar();
    setupSidebar();
    setupMainContent();
    setupDetailsPanel();
    
    // Add toolbar
    layout->addWidget(m_toolbarFrame);
    
    // Add main splitter
    m_mainSplitter->addWidget(m_sidebarFrame);
    m_mainSplitter->addWidget(m_contentSplitter);
    m_mainSplitter->setSizes({250, 750});
    
    layout->addWidget(m_mainSplitter);
}

void PluginStoreWidget::setupToolbar()
{
    m_toolbarFrame = new QFrame;
    m_toolbarFrame->setFrameStyle(QFrame::StyledPanel);
    m_toolbarFrame->setStyleSheet("QFrame { background-color: white; border-bottom: 1px solid #e0e0e0; padding: 8px; }");
    
    auto* toolbarLayout = new QHBoxLayout(m_toolbarFrame);
    
    // Search widget
    m_searchWidget = new PluginSearchWidget;
    toolbarLayout->addWidget(m_searchWidget);
    
    toolbarLayout->addStretch();
    
    // Sort combo
    m_sortCombo = new QComboBox;
    m_sortCombo->addItems({"Name", "Popularity", "Rating", "Date Added", "Downloads"});
    m_sortCombo->setCurrentText("Popularity");
    toolbarLayout->addWidget(m_sortCombo);
    
    // Refresh button
    m_refreshBtn = new QPushButton("Refresh");
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 8px 16px; border-radius: 4px; }");
    toolbarLayout->addWidget(m_refreshBtn);
    
    // Settings button
    m_settingsBtn = new QPushButton("Settings");
    m_settingsBtn->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; padding: 8px 16px; border-radius: 4px; }");
    toolbarLayout->addWidget(m_settingsBtn);
}

void PluginStoreWidget::setupSidebar()
{
    m_sidebarFrame = new QFrame;
    m_sidebarFrame->setFrameStyle(QFrame::StyledPanel);
    m_sidebarFrame->setStyleSheet("QFrame { background-color: #f8f9fa; border-right: 1px solid #e0e0e0; }");
    m_sidebarFrame->setMinimumWidth(200);
    m_sidebarFrame->setMaximumWidth(300);
    
    auto* sidebarLayout = new QVBoxLayout(m_sidebarFrame);
    
    // Category widget
    m_categoryWidget = new PluginCategoryWidget;
    sidebarLayout->addWidget(m_categoryWidget);
    
    // Filters group
    m_filtersGroup = new QGroupBox("Filters");
    auto* filtersLayout = new QVBoxLayout(m_filtersGroup);
    
    m_freeOnlyCheck = new QCheckBox("Free only");
    filtersLayout->addWidget(m_freeOnlyCheck);
    
    m_verifiedOnlyCheck = new QCheckBox("Verified only");
    filtersLayout->addWidget(m_verifiedOnlyCheck);
    
    // Rating filter
    auto* ratingLayout = new QHBoxLayout;
    ratingLayout->addWidget(new QLabel("Min Rating:"));
    m_ratingSlider = new QSlider(Qt::Horizontal);
    m_ratingSlider->setRange(0, 5);
    m_ratingSlider->setValue(0);
    ratingLayout->addWidget(m_ratingSlider);
    filtersLayout->addLayout(ratingLayout);
    
    // Date filters
    filtersLayout->addWidget(new QLabel("Date Range:"));
    m_dateFromEdit = new QDateEdit(QDate::currentDate().addMonths(-12));
    m_dateToEdit = new QDateEdit(QDate::currentDate());
    filtersLayout->addWidget(m_dateFromEdit);
    filtersLayout->addWidget(m_dateToEdit);
    
    sidebarLayout->addWidget(m_filtersGroup);
    sidebarLayout->addStretch();
}

void PluginStoreWidget::setupMainContent()
{
    m_contentSplitter = new QSplitter(Qt::Horizontal);
    
    // Content area
    m_contentArea = new QScrollArea;
    m_contentArea->setWidgetResizable(true);
    m_contentArea->setFrameShape(QFrame::NoFrame);
    
    m_contentWidget = new QWidget;
    m_contentLayout = new QGridLayout(m_contentWidget);
    m_contentLayout->setSpacing(16);
    
    // Status label
    m_statusLabel = new QLabel("Loading plugins...");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 40px;");
    m_contentLayout->addWidget(m_statusLabel, 0, 0);
    
    // Loading bar
    m_loadingBar = new QProgressBar;
    m_loadingBar->setRange(0, 0); // Indeterminate progress
    m_loadingBar->hide();
    m_contentLayout->addWidget(m_loadingBar, 1, 0);
    
    m_contentArea->setWidget(m_contentWidget);
    m_contentSplitter->addWidget(m_contentArea);
}

void PluginStoreWidget::setupDetailsPanel()
{
    m_detailsWidget = new PluginDetailsWidget;
    m_detailsWidget->setMinimumWidth(400);
    m_contentSplitter->addWidget(m_detailsWidget);
    m_contentSplitter->setSizes({600, 400});
}

void PluginStoreWidget::connectSignals()
{
    // Toolbar signals
    if (m_refreshBtn) {
        connect(m_refreshBtn, &QPushButton::clicked, this, &PluginStoreWidget::onRefreshClicked);
    }
    if (m_sortCombo) {
        connect(m_sortCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &PluginStoreWidget::onSortChanged);
    }
    if (m_searchWidget) {
        connect(m_searchWidget, &PluginSearchWidget::searchChanged, 
                [this](const QString&) { m_searchTimer->start(); });
    }
    
    // Sidebar signals
    if (m_categoryWidget) {
        connect(m_categoryWidget, &PluginCategoryWidget::categorySelected, 
                this, &PluginStoreWidget::onCategoryChanged);
    }
    if (m_freeOnlyCheck) {
        connect(m_freeOnlyCheck, &QCheckBox::toggled, this, &PluginStoreWidget::onFilterChanged);
    }
    if (m_verifiedOnlyCheck) {
        connect(m_verifiedOnlyCheck, &QCheckBox::toggled, this, &PluginStoreWidget::onFilterChanged);
    }
    if (m_ratingSlider) {
        connect(m_ratingSlider, &QSlider::valueChanged, this, &PluginStoreWidget::onFilterChanged);
    }
    
    // Details panel signals
    if (m_detailsWidget) {
        connect(m_detailsWidget, &PluginDetailsWidget::installRequested, 
                this, &PluginStoreWidget::installPlugin);
        connect(m_detailsWidget, &PluginDetailsWidget::updateRequested, 
                this, &PluginStoreWidget::updatePlugin);
        connect(m_detailsWidget, &PluginDetailsWidget::uninstallRequested, 
                this, &PluginStoreWidget::uninstallPlugin);
    }
    
    // Network signals
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &PluginStoreWidget::onStoreDataReceived);
}

void PluginStoreWidget::loadStoreData()
{
    if (m_storeUrl.isEmpty()) {
        // Load sample data for demonstration
        QJsonArray sampleData;
        QJsonObject plugin1;
        plugin1["id"] = "sample-ui-plugin";
        plugin1["name"] = "Sample UI Plugin";
        plugin1["description"] = "A sample plugin demonstrating UI capabilities";
        plugin1["author"] = "Plugin Team";
        plugin1["version"] = "1.0.0";
        plugin1["category"] = "UI";
        plugin1["rating"] = 4.5;
        plugin1["downloads"] = 1250;
        plugin1["price"] = 0;
        plugin1["verified"] = true;
        sampleData.append(plugin1);
        
        QJsonObject plugin2;
        plugin2["id"] = "sample-service-plugin";
        plugin2["name"] = "Sample Service Plugin";
        plugin2["description"] = "A sample plugin demonstrating service capabilities";
        plugin2["author"] = "Plugin Team";
        plugin2["version"] = "1.1.0";
        plugin2["category"] = "Service";
        plugin2["rating"] = 4.2;
        plugin2["downloads"] = 890;
        plugin2["price"] = 0;
        plugin2["verified"] = true;
        sampleData.append(plugin2);
        
        m_storeData = sampleData;
        updatePluginList();
        return;
    }
    
    showLoadingIndicator(true);
    
    QNetworkRequest request{QUrl(m_storeUrl)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_networkManager->get(request);
}

void PluginStoreWidget::updatePluginList()
{
    // Clear existing items
    for (auto* item : m_pluginItems) {
        item->deleteLater();
    }
    m_pluginItems.clear();
    
    // Clear layout
    QLayoutItem* child;
    while ((child = m_contentLayout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    
    if (m_storeData.isEmpty()) {
        m_statusLabel = new QLabel("No plugins available");
        m_statusLabel->setAlignment(Qt::AlignCenter);
        m_statusLabel->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 40px;");
        m_contentLayout->addWidget(m_statusLabel, 0, 0);
        return;
    }
    
    // Add plugin items
    int row = 0, col = 0;
    const int maxCols = 2;
    
    for (const auto& value : m_storeData) {
        QJsonObject pluginData = value.toObject();
        
        auto* item = new PluginStoreItem(pluginData);
        connect(item, &PluginStoreItem::clicked, this, &PluginStoreWidget::onPluginItemClicked);
        connect(item, &PluginStoreItem::installRequested, this, &PluginStoreWidget::onInstallClicked);
        connect(item, &PluginStoreItem::updateRequested, this, &PluginStoreWidget::onUpdateClicked);
        connect(item, &PluginStoreItem::detailsRequested, this, &PluginStoreWidget::onPluginItemClicked);
        
        m_pluginItems.append(item);
        m_contentLayout->addWidget(item, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
    
    // Add stretch to push items to top
    m_contentLayout->setRowStretch(row + 1, 1);
    
    emit searchCompleted(m_pluginItems.size());
}

void PluginStoreWidget::applyFilters()
{
    // TODO: Implement filtering logic
    updatePluginList();
}

void PluginStoreWidget::showLoadingIndicator(bool show)
{
    m_loading = show;
    if (m_loadingBar) {
        m_loadingBar->setVisible(show);
    }
    if (m_statusLabel) {
        m_statusLabel->setText(show ? "Loading plugins..." : "");
    }
}

// Placeholder implementations for other classes
PluginStoreItem::PluginStoreItem(const QJsonObject& pluginData, QWidget* parent)
    : QFrame(parent), m_pluginId(pluginData["id"].toString()), m_pluginData(pluginData)
{
    setupUI();
}

void PluginStoreItem::setupUI()
{
    setFrameStyle(QFrame::StyledPanel);
    setStyleSheet("QFrame { background-color: white; border: 1px solid #e0e0e0; border-radius: 8px; padding: 16px; }");
    setMinimumHeight(120);
    
    auto* layout = new QVBoxLayout(this);
    
    m_nameLabel = new QLabel(m_pluginData["name"].toString());
    m_nameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50;");
    layout->addWidget(m_nameLabel);
    
    m_descriptionLabel = new QLabel(m_pluginData["description"].toString());
    m_descriptionLabel->setStyleSheet("color: #7f8c8d; font-size: 12px;");
    m_descriptionLabel->setWordWrap(true);
    layout->addWidget(m_descriptionLabel);
    
    auto* infoLayout = new QHBoxLayout;
    m_authorLabel = new QLabel("by " + m_pluginData["author"].toString());
    m_authorLabel->setStyleSheet("color: #95a5a6; font-size: 10px;");
    infoLayout->addWidget(m_authorLabel);
    
    infoLayout->addStretch();
    
    m_installBtn = new QPushButton("Install");
    m_installBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 4px 12px; border-radius: 4px; }");
    connect(m_installBtn, &QPushButton::clicked, this, &PluginStoreItem::installRequested);
    infoLayout->addWidget(m_installBtn);
    
    layout->addLayout(infoLayout);
}

void PluginStoreItem::setInstalled(bool installed)
{
    m_installed = installed;
    updateInstallButton();
}

void PluginStoreItem::setHasUpdate(bool hasUpdate)
{
    m_hasUpdate = hasUpdate;
    updateInstallButton();
}

void PluginStoreItem::updateData(const QJsonObject& data)
{
    m_pluginData = data;
    // Update UI elements
}

void PluginStoreItem::updateInstallButton()
{
    if (!m_installBtn) return;
    
    if (m_installed) {
        if (m_hasUpdate) {
            m_installBtn->setText("Update");
            m_installBtn->setStyleSheet("QPushButton { background-color: #f39c12; color: white; border: none; padding: 4px 12px; border-radius: 4px; }");
        } else {
            m_installBtn->setText("Installed");
            m_installBtn->setStyleSheet("QPushButton { background-color: #2ecc71; color: white; border: none; padding: 4px 12px; border-radius: 4px; }");
            m_installBtn->setEnabled(false);
        }
    } else {
        m_installBtn->setText("Install");
        m_installBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; padding: 4px 12px; border-radius: 4px; }");
        m_installBtn->setEnabled(true);
    }
}

void PluginStoreItem::loadPluginIcon()
{
    // TODO: Implement icon loading
}

void PluginStoreItem::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
    QFrame::mousePressEvent(event);
}

void PluginStoreItem::enterEvent(QEnterEvent* event)
{
    m_hovered = true;
    setStyleSheet("QFrame { background-color: #f8f9fa; border: 1px solid #3498db; border-radius: 8px; padding: 16px; }");
    QFrame::enterEvent(event);
}

void PluginStoreItem::leaveEvent(QEvent* event)
{
    m_hovered = false;
    setStyleSheet("QFrame { background-color: white; border: 1px solid #e0e0e0; border-radius: 8px; padding: 16px; }");
    QFrame::leaveEvent(event);
}

void PluginStoreItem::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
}

void PluginStoreItem::onInstallClicked()
{
    emit installRequested();
}

void PluginStoreItem::onUpdateClicked()
{
    emit updateRequested();
}

void PluginStoreItem::onDetailsClicked()
{
    emit detailsRequested();
}

// Placeholder implementations for other widget classes
PluginDetailsWidget::PluginDetailsWidget(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* label = new QLabel("Plugin Details");
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 18px; color: #7f8c8d; padding: 40px;");
    layout->addWidget(label);
}

void PluginDetailsWidget::showPlugin(const QJsonObject& pluginData)
{
    m_pluginData = pluginData;
    m_pluginId = pluginData["id"].toString();
    // TODO: Update UI with plugin details
}

void PluginDetailsWidget::clearDetails()
{
    m_pluginData = QJsonObject();
    m_pluginId.clear();
}

void PluginDetailsWidget::onInstallClicked()
{
    if (!m_pluginId.isEmpty()) {
        emit installRequested(m_pluginId);
    }
}

void PluginDetailsWidget::onUpdateClicked()
{
    if (!m_pluginId.isEmpty()) {
        emit updateRequested(m_pluginId);
    }
}

void PluginDetailsWidget::onUninstallClicked()
{
    if (!m_pluginId.isEmpty()) {
        emit uninstallRequested(m_pluginId);
    }
}

void PluginDetailsWidget::onRatingChanged(int rating)
{
    if (!m_pluginId.isEmpty()) {
        emit ratingSubmitted(m_pluginId, rating);
    }
}

void PluginDetailsWidget::onReviewSubmitted()
{
    // TODO: Get review text from review widget
    if (!m_pluginId.isEmpty()) {
        emit reviewSubmitted(m_pluginId, QString("Sample review"));
    }
}

void PluginDetailsWidget::onScreenshotClicked()
{
    // TODO: Implement screenshot viewer
}

void PluginDetailsWidget::onTabChanged(int index)
{
    Q_UNUSED(index)
    // TODO: Handle tab changes if needed
}

PluginCategoryWidget::PluginCategoryWidget(QWidget* parent) : QTreeWidget(parent)
{
    setHeaderLabel("Categories");
    
    // Add sample categories
    auto* allItem = new QTreeWidgetItem(this, QStringList("All Plugins"));
    auto* uiItem = new QTreeWidgetItem(this, QStringList("UI Plugins"));
    auto* serviceItem = new QTreeWidgetItem(this, QStringList("Service Plugins"));
    auto* networkItem = new QTreeWidgetItem(this, QStringList("Network Plugins"));
    auto* dataItem = new QTreeWidgetItem(this, QStringList("Data Plugins"));
    
    m_allItem = allItem;
    m_categoryItems["UI"] = uiItem;
    m_categoryItems["Service"] = serviceItem;
    m_categoryItems["Network"] = networkItem;
    m_categoryItems["Data"] = dataItem;
    
    connect(this, &QTreeWidget::itemClicked, this, &PluginCategoryWidget::onItemClicked);
}

void PluginCategoryWidget::setCategories(const QStringList& categories)
{
    Q_UNUSED(categories)
    // TODO: Update categories
}

void PluginCategoryWidget::setCategoryCounts(const QHash<QString, int>& counts)
{
    m_categoryCounts = counts;
    updateCounts();
}

QString PluginCategoryWidget::selectedCategory() const
{
    auto* current = currentItem();
    if (!current || current == m_allItem) {
        return QString();
    }
    return current->text(0).replace(" Plugins", "");
}

void PluginCategoryWidget::onItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    QString category;
    if (item != m_allItem) {
        category = item->text(0).replace(" Plugins", "");
    }
    emit categorySelected(category);
}

void PluginCategoryWidget::updateCounts()
{
    // TODO: Update category counts
}

PluginSearchWidget::PluginSearchWidget(QWidget* parent) : QWidget(parent)
{
    setupUI();
}

void PluginSearchWidget::setupUI()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search plugins...");
    layout->addWidget(m_searchEdit);
    
    m_clearBtn = new QPushButton("Clear");
    layout->addWidget(m_clearBtn);
    
    connect(m_searchEdit, &QLineEdit::textChanged, this, &PluginSearchWidget::onTextChanged);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &PluginSearchWidget::onReturnPressed);
    connect(m_clearBtn, &QPushButton::clicked, this, &PluginSearchWidget::onClearClicked);
}

QString PluginSearchWidget::searchText() const
{
    return m_searchEdit ? m_searchEdit->text() : QString();
}

void PluginSearchWidget::setSearchText(const QString& text)
{
    if (m_searchEdit) {
        m_searchEdit->setText(text);
    }
}

void PluginSearchWidget::clearSearch()
{
    if (m_searchEdit) {
        m_searchEdit->clear();
    }
}

void PluginSearchWidget::onTextChanged()
{
    emit searchChanged(searchText());
}

void PluginSearchWidget::onReturnPressed()
{
    emit searchSubmitted(searchText());
}

void PluginSearchWidget::onClearClicked()
{
    clearSearch();
}





// PluginRatingWidget Implementation
PluginRatingWidget::PluginRatingWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumHeight(m_starSize.height() + 4);
    setCursor(m_readOnly ? Qt::ArrowCursor : Qt::PointingHandCursor);
}

void PluginRatingWidget::setRating(double rating)
{
    if (qAbs(rating - m_rating) > 0.01 && rating >= 0 && rating <= m_maxRating) {
        m_rating = rating;
        update();
        emit ratingChanged(static_cast<int>(rating));
    }
}

void PluginRatingWidget::setMaxRating(int maxRating)
{
    if (maxRating > 0 && maxRating != m_maxRating) {
        m_maxRating = maxRating;
        if (m_rating > m_maxRating) {
            setRating(m_maxRating);
        }
        update();
    }
}

void PluginRatingWidget::setReadOnly(bool readOnly)
{
    m_readOnly = readOnly;
    setCursor(readOnly ? Qt::ArrowCursor : Qt::PointingHandCursor);
}

void PluginRatingWidget::setShowText(bool showText)
{
    m_showText = showText;
    update();
}

void PluginRatingWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int starSpacing = 2;
    int totalWidth = m_maxRating * (m_starSize.width() + starSpacing) - starSpacing;
    int startX = (width() - totalWidth) / 2;
    int startY = (height() - m_starSize.height()) / 2;

    for (int i = 0; i < m_maxRating; ++i) {
        QRect starRect(startX + i * (m_starSize.width() + starSpacing), startY,
                      m_starSize.width(), m_starSize.height());

        if (i < m_rating) {
            painter.setBrush(QColor("#f39c12")); // Gold color for filled stars
        } else {
            painter.setBrush(QColor("#bdc3c7")); // Gray color for empty stars
        }

        painter.setPen(QPen(QColor("#95a5a6"), 1));

        // Draw a simple star shape (simplified as a circle for now)
        painter.drawEllipse(starRect);
    }

    // Draw rating text if enabled
    if (m_showText) {
        QString text = QString("%1/%2").arg(m_rating).arg(m_maxRating);
        QRect textRect = rect();
        textRect.setLeft(startX + totalWidth + 10);
        painter.setPen(QColor("#2c3e50"));
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
}

void PluginRatingWidget::mousePressEvent(QMouseEvent* event)
{
    if (!m_readOnly && event->button() == Qt::LeftButton) {
        int starSpacing = 2;
        int totalWidth = m_maxRating * (m_starSize.width() + starSpacing) - starSpacing;
        int startX = (width() - totalWidth) / 2;

        int clickedStar = (event->position().x() - startX) / (m_starSize.width() + starSpacing) + 1;
        if (clickedStar >= 1 && clickedStar <= m_maxRating) {
            setRating(clickedStar);
        }
    }
    QWidget::mousePressEvent(event);
}

void PluginRatingWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_readOnly) {
        // TODO: Implement hover effects
    }
    QWidget::mouseMoveEvent(event);
}

void PluginRatingWidget::leaveEvent(QEvent* event)
{
    // TODO: Implement leave event handling
    QWidget::leaveEvent(event);
}

// PluginReviewWidget Implementation
PluginReviewWidget::PluginReviewWidget(QWidget* parent)
    : QWidget(parent)
    , m_reviewsLayout(nullptr)
    , m_reviewEdit(nullptr)
    , m_ratingWidget(nullptr)
    , m_submitBtn(nullptr)
    , m_showMoreBtn(nullptr)
{
    setupUI();
}

void PluginReviewWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);

    // Review submission area
    auto* submitFrame = new QFrame;
    submitFrame->setFrameStyle(QFrame::StyledPanel);
    auto* submitLayout = new QVBoxLayout(submitFrame);

    auto* submitLabel = new QLabel("Write a Review:");
    submitLabel->setStyleSheet("font-weight: bold;");
    submitLayout->addWidget(submitLabel);

    m_reviewEdit = new QTextEdit;
    m_reviewEdit->setMaximumHeight(80);
    m_reviewEdit->setPlaceholderText("Share your experience with this plugin...");
    submitLayout->addWidget(m_reviewEdit);

    // Rating widget
    m_ratingWidget = new PluginRatingWidget;
    m_ratingWidget->setReadOnly(false);
    submitLayout->addWidget(m_ratingWidget);

    auto* submitButtonLayout = new QHBoxLayout;
    submitButtonLayout->addStretch();
    m_submitBtn = new QPushButton("Submit Review");
    m_submitBtn->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 6px 12px; border-radius: 3px; }");
    submitButtonLayout->addWidget(m_submitBtn);

    submitLayout->addLayout(submitButtonLayout);
    layout->addWidget(submitFrame);

    // Reviews layout
    m_reviewsLayout = new QVBoxLayout;
    layout->addLayout(m_reviewsLayout);

    // Show more button
    auto* moreButtonLayout = new QHBoxLayout;
    moreButtonLayout->addStretch();
    m_showMoreBtn = new QPushButton("Show More Reviews");
    m_showMoreBtn->setStyleSheet("QPushButton { color: #3498db; border: 1px solid #3498db; padding: 6px 12px; border-radius: 3px; }");
    moreButtonLayout->addWidget(m_showMoreBtn);
    moreButtonLayout->addStretch();

    layout->addLayout(moreButtonLayout);

    // Connect signals
    connect(m_submitBtn, &QPushButton::clicked, this, &PluginReviewWidget::onSubmitReview);
    connect(m_showMoreBtn, &QPushButton::clicked, this, &PluginReviewWidget::onShowMoreReviews);
}

void PluginReviewWidget::setReviews(const QJsonArray& reviews)
{
    // Clear existing reviews
    clearReviews();

    // Add new reviews
    for (const auto& value : reviews) {
        if (value.isObject()) {
            addReview(value.toObject());
        }
    }
}

void PluginReviewWidget::addReview(const QJsonObject& review)
{
    addReviewWidget(review);
}

void PluginReviewWidget::clearReviews()
{
    // Clear all review widgets
    for (auto* widget : m_reviewWidgets) {
        widget->deleteLater();
    }
    m_reviewWidgets.clear();
}

void PluginReviewWidget::onSubmitReview()
{
    if (m_reviewEdit && m_ratingWidget) {
        QString reviewText = m_reviewEdit->toPlainText();
        int rating = static_cast<int>(m_ratingWidget->rating());

        if (!reviewText.isEmpty() && rating > 0) {
            emit reviewSubmitted(reviewText, rating);
            m_reviewEdit->clear();
            m_ratingWidget->setRating(0);
        }
    }
}

void PluginReviewWidget::onShowMoreReviews()
{
    m_visibleReviews += 5;
    // TODO: Load more reviews if needed
}

void PluginReviewWidget::addReviewWidget(const QJsonObject& review)
{
    auto* reviewWidget = new QWidget;
    auto* reviewLayout = new QVBoxLayout(reviewWidget);
    reviewLayout->setContentsMargins(8, 8, 8, 8);

    // Header with author and rating
    auto* headerLayout = new QHBoxLayout;
    auto* authorLabel = new QLabel(review["author"].toString("Anonymous"));
    authorLabel->setStyleSheet("font-weight: bold;");
    headerLayout->addWidget(authorLabel);

    auto* ratingWidget = new PluginRatingWidget;
    ratingWidget->setRating(review["rating"].toDouble(0));
    ratingWidget->setReadOnly(true);
    ratingWidget->setShowText(false);
    headerLayout->addWidget(ratingWidget);

    headerLayout->addStretch();

    auto* dateLabel = new QLabel(review["date"].toString("Unknown date"));
    dateLabel->setStyleSheet("color: #7f8c8d; font-size: 12px;");
    headerLayout->addWidget(dateLabel);

    reviewLayout->addLayout(headerLayout);

    // Review text
    auto* textLabel = new QLabel(review["text"].toString("No review text"));
    textLabel->setWordWrap(true);
    textLabel->setStyleSheet("margin-top: 4px;");
    reviewLayout->addWidget(textLabel);

    // Add to layout and track
    if (m_reviewsLayout) {
        m_reviewsLayout->addWidget(reviewWidget);
    }
    m_reviewWidgets.append(reviewWidget);
}

// PluginInstaller Implementation
PluginInstaller::PluginInstaller(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void PluginInstaller::installPlugin(const QString& pluginId, const QUrl& downloadUrl)
{
    if (m_installing) {
        return; // Already installing
    }

    m_currentPlugin = pluginId;
    m_installing = true;

    emit installStarted(pluginId);
    downloadPlugin(downloadUrl);
}

void PluginInstaller::updatePlugin(const QString& pluginId, const QUrl& downloadUrl)
{
    // For now, treat update as install
    installPlugin(pluginId, downloadUrl);
}

void PluginInstaller::uninstallPlugin(const QString& pluginId)
{
    // TODO: Implement plugin uninstallation
    emit installFinished(pluginId, true);
}

void PluginInstaller::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        int percentage = static_cast<int>((bytesReceived * 100) / bytesTotal);
        emit installProgress(m_currentPlugin, percentage);
    }
}

void PluginInstaller::onDownloadFinished()
{
    // TODO: Implement download finished handling
    m_installing = false;
    emit installFinished(m_currentPlugin, true);
}

void PluginInstaller::onDownloadError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    m_installing = false;
    emit installError(m_currentPlugin, "Download failed");
}

void PluginInstaller::downloadPlugin(const QUrl& url)
{
    // TODO: Implement plugin download
    Q_UNUSED(url)
}

bool PluginInstaller::extractPlugin(const QString& archivePath, const QString& extractPath)
{
    // TODO: Implement plugin extraction
    Q_UNUSED(archivePath)
    Q_UNUSED(extractPath)
    return true;
}

bool PluginInstaller::validatePlugin(const QString& pluginPath)
{
    // TODO: Implement plugin validation
    Q_UNUSED(pluginPath)
    return true;
}

bool PluginInstaller::installExtractedPlugin(const QString& pluginPath)
{
    // TODO: Implement plugin installation
    Q_UNUSED(pluginPath)
    return true;
}
