// RibbonInterface.cpp - Implementation of Modern Ribbon Interface Framework
#include "RibbonInterface.h"
#include "RibbonThemes.h"
#include <QApplication>
#include <QStyleOption>
#include <QStylePainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFontMetrics>
#include <QTextOption>
#include <QLoggingCategory>
#include <QFile>
#include <QUuid>
#include <QTimer>
#include <QSettings>
#include <QScreen>

Q_LOGGING_CATEGORY(ribbonInterface, "ui.ribbon.interface")

// RibbonBar Private Implementation
struct RibbonBar::RibbonBarPrivate {
    QVBoxLayout* mainLayout = nullptr;
    QHBoxLayout* topLayout = nullptr;
    QTabWidget* tabWidget = nullptr;
    RibbonQuickAccessToolbar* quickAccessToolbar = nullptr;
    RibbonApplicationButton* applicationButton = nullptr;
    RibbonThemeManager* themeManager = nullptr;
    
    QMap<QString, RibbonTab*> tabs;
    QMap<QString, RibbonTab*> contextTabs;
    QStringList tabOrder;
    
    RibbonTheme currentTheme = RibbonTheme::Light;
    bool minimized = false;
    bool animationsEnabled = true;
    int animationDuration = 250;
    int tabHeight = 100;
    int groupSpacing = 6;
    
    QPropertyAnimation* minimizeAnimation = nullptr;
    QTimer* layoutTimer = nullptr;
    
    explicit RibbonBarPrivate(RibbonBar* parent) {
        themeManager = RibbonThemeManager::instance();
        minimizeAnimation = new QPropertyAnimation(parent, "geometry", parent);
        minimizeAnimation->setDuration(animationDuration);
        minimizeAnimation->setEasingCurve(QEasingCurve::OutCubic);
        
        layoutTimer = new QTimer(parent);
        layoutTimer->setSingleShot(true);
        layoutTimer->setInterval(50);
    }
};

// RibbonBar Implementation
RibbonBar::RibbonBar(QWidget* parent)
    : QWidget(parent)
    , d(std::make_unique<RibbonBarPrivate>(this))
{
    setupUI();
    setupAnimations();
    updateTheme();
    
    // Connect theme manager
    connect(d->themeManager, &RibbonThemeManager::themeChanged,
            this, &RibbonBar::updateTheme);
    
    // Connect layout timer
    connect(d->layoutTimer, &QTimer::timeout,
            this, &RibbonBar::updateLayout);
    
    qCInfo(ribbonInterface) << "RibbonBar created";
}

RibbonBar::~RibbonBar() = default;

RibbonTab* RibbonBar::addTab(const QString& title, const QString& id) {
    return insertTab(d->tabWidget->count(), title, id);
}

RibbonTab* RibbonBar::insertTab(int index, const QString& title, const QString& id) {
    QString tabId = id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id;
    
    if (d->tabs.contains(tabId)) {
        qCWarning(ribbonInterface) << "Tab with ID already exists:" << tabId;
        return d->tabs[tabId];
    }
    
    auto tab = new RibbonTab(title, tabId, this);
    d->tabs[tabId] = tab;
    d->tabOrder.insert(index, tabId);
    
    d->tabWidget->insertTab(index, tab, title);
    
    emit tabAdded(index, tabId);
    qCInfo(ribbonInterface) << "Added tab:" << title << "with ID:" << tabId;
    
    return tab;
}

void RibbonBar::removeTab(int index) {
    if (index < 0 || index >= d->tabWidget->count()) {
        return;
    }
    
    QString tabId = d->tabOrder[index];
    removeTab(tabId);
}

void RibbonBar::removeTab(const QString& id) {
    if (!d->tabs.contains(id)) {
        return;
    }
    
    int index = d->tabOrder.indexOf(id);
    RibbonTab* tab = d->tabs[id];
    
    d->tabWidget->removeTab(index);
    d->tabs.remove(id);
    d->tabOrder.removeAt(index);
    
    tab->deleteLater();
    
    emit tabRemoved(index, id);
    qCInfo(ribbonInterface) << "Removed tab with ID:" << id;
}

RibbonTab* RibbonBar::tab(int index) const {
    if (index < 0 || index >= d->tabOrder.size()) {
        return nullptr;
    }
    
    QString tabId = d->tabOrder[index];
    return d->tabs.value(tabId);
}

RibbonTab* RibbonBar::tab(const QString& id) const {
    return d->tabs.value(id);
}

int RibbonBar::tabCount() const {
    return d->tabWidget->count();
}

int RibbonBar::currentTabIndex() const {
    return d->tabWidget->currentIndex();
}

RibbonTab* RibbonBar::currentTab() const {
    return tab(currentTabIndex());
}

void RibbonBar::setCurrentTab(int index) {
    if (index >= 0 && index < d->tabWidget->count()) {
        d->tabWidget->setCurrentIndex(index);
    }
}

void RibbonBar::setCurrentTab(const QString& id) {
    int index = d->tabOrder.indexOf(id);
    if (index >= 0) {
        setCurrentTab(index);
    }
}

RibbonQuickAccessToolbar* RibbonBar::quickAccessToolbar() const {
    return d->quickAccessToolbar;
}

void RibbonBar::setQuickAccessToolbarVisible(bool visible) {
    d->quickAccessToolbar->setVisible(visible);
}

bool RibbonBar::isQuickAccessToolbarVisible() const {
    return d->quickAccessToolbar->isVisible();
}

RibbonApplicationButton* RibbonBar::applicationButton() const {
    return d->applicationButton;
}

void RibbonBar::setApplicationButtonVisible(bool visible) {
    d->applicationButton->setVisible(visible);
}

bool RibbonBar::isApplicationButtonVisible() const {
    return d->applicationButton->isVisible();
}

bool RibbonBar::isMinimized() const {
    return d->minimized;
}

void RibbonBar::setMinimized(bool minimized) {
    if (d->minimized == minimized) {
        return;
    }
    
    d->minimized = minimized;
    
    if (d->animationsEnabled) {
        animateMinimize(minimized);
    } else {
        d->tabWidget->setVisible(!minimized);
        updateLayout();
    }
    
    emit minimizedChanged(minimized);
    qCInfo(ribbonInterface) << "Ribbon minimized:" << minimized;
}

void RibbonBar::toggleMinimized() {
    setMinimized(!d->minimized);
}

RibbonTheme RibbonBar::theme() const {
    return d->currentTheme;
}

void RibbonBar::setTheme(RibbonTheme theme) {
    if (d->currentTheme == theme) {
        return;
    }
    
    d->currentTheme = theme;
    d->themeManager->setTheme(theme);
    updateTheme();
    
    emit themeChanged(theme);
    qCInfo(ribbonInterface) << "Ribbon theme changed to:" << static_cast<int>(theme);
}

void RibbonBar::setCustomTheme(const QJsonObject& themeData) {
    d->currentTheme = RibbonTheme::Custom;
    d->themeManager->setCustomTheme(themeData);
    updateTheme();
    
    emit themeChanged(RibbonTheme::Custom);
    qCInfo(ribbonInterface) << "Custom ribbon theme applied";
}

bool RibbonBar::animationsEnabled() const {
    return d->animationsEnabled;
}

void RibbonBar::setAnimationsEnabled(bool enabled) {
    d->animationsEnabled = enabled;
}

void RibbonBar::setAnimationDuration(int milliseconds) {
    d->animationDuration = milliseconds;
    d->minimizeAnimation->setDuration(milliseconds);
}

int RibbonBar::animationDuration() const {
    return d->animationDuration;
}

void RibbonBar::setTabHeight(int height) {
    d->tabHeight = height;
    d->tabWidget->setFixedHeight(height);
    updateLayout();
}

int RibbonBar::tabHeight() const {
    return d->tabHeight;
}

void RibbonBar::setGroupSpacing(int spacing) {
    d->groupSpacing = spacing;
    
    // Update spacing for all tabs
    for (RibbonTab* tab : d->tabs.values()) {
        tab->setGroupSpacing(spacing);
    }
}

int RibbonBar::groupSpacing() const {
    return d->groupSpacing;
}

void RibbonBar::saveLayout(const QString& filePath) {
    QJsonObject layout = exportLayout();
    QJsonDocument doc(layout);
    
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        qCInfo(ribbonInterface) << "Ribbon layout saved to:" << filePath;
    } else {
        qCWarning(ribbonInterface) << "Failed to save ribbon layout to:" << filePath;
    }
}

void RibbonBar::loadLayout(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(ribbonInterface) << "Failed to load ribbon layout from:" << filePath;
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isObject()) {
        importLayout(doc.object());
        qCInfo(ribbonInterface) << "Ribbon layout loaded from:" << filePath;
    } else {
        qCWarning(ribbonInterface) << "Invalid ribbon layout file:" << filePath;
    }
}

QJsonObject RibbonBar::exportLayout() const {
    QJsonObject layout;
    layout["version"] = "1.0";
    layout["theme"] = static_cast<int>(d->currentTheme);
    layout["minimized"] = d->minimized;
    layout["tabHeight"] = d->tabHeight;
    layout["groupSpacing"] = d->groupSpacing;
    layout["animationsEnabled"] = d->animationsEnabled;
    layout["animationDuration"] = d->animationDuration;
    
    // Export tabs
    QJsonArray tabsArray;
    for (const QString& tabId : d->tabOrder) {
        if (d->tabs.contains(tabId)) {
            RibbonTab* tab = d->tabs[tabId];
            QJsonObject tabObj;
            tabObj["id"] = tabId;
            tabObj["title"] = tab->title();
            tabObj["contextual"] = tab->isContextual();
            tabObj["context"] = tab->context();
            // TODO: Export groups and controls
            tabsArray.append(tabObj);
        }
    }
    layout["tabs"] = tabsArray;
    
    return layout;
}

void RibbonBar::importLayout(const QJsonObject& layout) {
    // Clear existing tabs
    while (tabCount() > 0) {
        removeTab(0);
    }
    
    // Import settings
    if (layout.contains("theme")) {
        setTheme(static_cast<RibbonTheme>(layout["theme"].toInt()));
    }
    if (layout.contains("minimized")) {
        setMinimized(layout["minimized"].toBool());
    }
    if (layout.contains("tabHeight")) {
        setTabHeight(layout["tabHeight"].toInt());
    }
    if (layout.contains("groupSpacing")) {
        setGroupSpacing(layout["groupSpacing"].toInt());
    }
    if (layout.contains("animationsEnabled")) {
        setAnimationsEnabled(layout["animationsEnabled"].toBool());
    }
    if (layout.contains("animationDuration")) {
        setAnimationDuration(layout["animationDuration"].toInt());
    }
    
    // Import tabs
    if (layout.contains("tabs")) {
        QJsonArray tabsArray = layout["tabs"].toArray();
        for (const QJsonValue& value : tabsArray) {
            QJsonObject tabObj = value.toObject();
            QString tabId = tabObj["id"].toString();
            QString title = tabObj["title"].toString();
            
            RibbonTab* tab = addTab(title, tabId);
            if (tabObj.contains("contextual")) {
                tab->setContextual(tabObj["contextual"].toBool());
            }
            if (tabObj.contains("context")) {
                tab->setContext(tabObj["context"].toString());
            }
            // TODO: Import groups and controls
        }
    }
}

void RibbonBar::addContextTab(const QString& title, const QString& context, const QString& id) {
    QString tabId = id.isEmpty() ? QUuid::createUuid().toString(QUuid::WithoutBraces) : id;
    
    auto tab = new RibbonTab(title, tabId, this);
    tab->setContextual(true);
    tab->setContext(context);
    
    d->contextTabs[tabId] = tab;
    
    qCInfo(ribbonInterface) << "Added context tab:" << title << "for context:" << context;
}

void RibbonBar::removeContextTab(const QString& id) {
    if (d->contextTabs.contains(id)) {
        RibbonTab* tab = d->contextTabs[id];
        d->contextTabs.remove(id);
        tab->deleteLater();
        
        qCInfo(ribbonInterface) << "Removed context tab with ID:" << id;
    }
}

void RibbonBar::showContextTabs(const QString& context) {
    for (RibbonTab* tab : d->contextTabs.values()) {
        if (tab->context() == context) {
            // Add to main tab widget if not already there
            if (d->tabWidget->indexOf(tab) == -1) {
                d->tabWidget->addTab(tab, tab->title());
            }
        }
    }
    
    qCInfo(ribbonInterface) << "Showing context tabs for:" << context;
}

void RibbonBar::hideContextTabs(const QString& context) {
    for (RibbonTab* tab : d->contextTabs.values()) {
        if (tab->context() == context) {
            int index = d->tabWidget->indexOf(tab);
            if (index != -1) {
                d->tabWidget->removeTab(index);
            }
        }
    }
    
    qCInfo(ribbonInterface) << "Hiding context tabs for:" << context;
}

void RibbonBar::showCustomizationDialog() {
    // TODO: Implement customization dialog
    emit customizationRequested();
}

void RibbonBar::resetToDefaults() {
    // TODO: Implement reset to defaults
    qCInfo(ribbonInterface) << "Resetting ribbon to defaults";
}

void RibbonBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    paintBackground(&painter);
    paintTabBackground(&painter);
}

void RibbonBar::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    d->layoutTimer->start();
}

void RibbonBar::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);

    // Handle clicks on tab bar area for minimization
    if (event->button() == Qt::LeftButton) {
        QRect tabBarRect = d->tabWidget->tabBar()->geometry();
        if (tabBarRect.contains(event->pos())) {
            // Click was on tab bar - let it handle normally
            return;
        }
    }
}

void RibbonBar::mouseDoubleClickEvent(QMouseEvent* event) {
    QWidget::mouseDoubleClickEvent(event);

    // Double-click on tab bar to toggle minimization
    QRect tabBarRect = d->tabWidget->tabBar()->geometry();
    if (tabBarRect.contains(event->pos())) {
        toggleMinimized();
    }
}

bool RibbonBar::eventFilter(QObject* object, QEvent* event) {
    if (object == d->tabWidget->tabBar() && event->type() == QEvent::MouseButtonDblClick) {
        toggleMinimized();
        return true;
    }

    return QWidget::eventFilter(object, event);
}

void RibbonBar::onTabChanged(int index) {
    if (index >= 0 && index < d->tabOrder.size()) {
        QString tabId = d->tabOrder[index];
        emit currentTabChanged(index, tabId);
        qCDebug(ribbonInterface) << "Current tab changed to:" << index << tabId;
    }
}

void RibbonBar::onTabBarDoubleClicked() {
    toggleMinimized();
}

void RibbonBar::onAnimationFinished() {
    updateLayout();
}

void RibbonBar::setupUI() {
    // Create main layout
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    d->mainLayout->setSpacing(0);

    // Create top layout for application button and quick access toolbar
    d->topLayout = new QHBoxLayout();
    d->topLayout->setContentsMargins(4, 2, 4, 2);
    d->topLayout->setSpacing(4);

    // Create application button
    d->applicationButton = new RibbonApplicationButton(this);
    d->topLayout->addWidget(d->applicationButton);

    // Create quick access toolbar
    d->quickAccessToolbar = new RibbonQuickAccessToolbar(this);
    d->topLayout->addWidget(d->quickAccessToolbar);

    d->topLayout->addStretch();

    // Create tab widget
    d->tabWidget = new QTabWidget(this);
    d->tabWidget->setTabPosition(QTabWidget::North);
    d->tabWidget->setFixedHeight(d->tabHeight);
    d->tabWidget->tabBar()->installEventFilter(this);

    // Add layouts to main layout
    d->mainLayout->addLayout(d->topLayout);
    d->mainLayout->addWidget(d->tabWidget);

    // Connect tab widget signals
    connect(d->tabWidget, &QTabWidget::currentChanged,
            this, &RibbonBar::onTabChanged);

    // Set minimum size
    setMinimumHeight(d->tabHeight + 30);
}

void RibbonBar::setupAnimations() {
    connect(d->minimizeAnimation, &QPropertyAnimation::finished,
            this, &RibbonBar::onAnimationFinished);
}

void RibbonBar::updateTheme() {
    // Apply theme styles
    setStyleSheet(d->themeManager->ribbonBarStyleSheet());
    d->tabWidget->setStyleSheet(d->themeManager->ribbonTabStyleSheet());

    // Update all tabs
    for (RibbonTab* tab : d->tabs.values()) {
        // Tab will update its own theme
        tab->update();
    }

    update();
}

void RibbonBar::updateLayout() {
    // Update layout based on current state
    if (d->minimized) {
        d->tabWidget->setFixedHeight(d->tabWidget->tabBar()->height());
    } else {
        d->tabWidget->setFixedHeight(d->tabHeight);
    }

    // Update minimum height
    int minHeight = d->topLayout->sizeHint().height() +
                   (d->minimized ? d->tabWidget->tabBar()->height() : d->tabHeight);
    setMinimumHeight(minHeight);

    adjustSize();
}

void RibbonBar::animateMinimize(bool minimize) {
    QRect startGeometry = d->tabWidget->geometry();
    QRect endGeometry = startGeometry;

    if (minimize) {
        endGeometry.setHeight(d->tabWidget->tabBar()->height());
    } else {
        endGeometry.setHeight(d->tabHeight);
    }

    d->minimizeAnimation->setStartValue(startGeometry);
    d->minimizeAnimation->setEndValue(endGeometry);
    d->minimizeAnimation->start();
}

void RibbonBar::paintBackground(QPainter* painter) {
    QRect rect = this->rect();

    // Create gradient background
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());

    switch (d->currentTheme) {
        case RibbonTheme::Light:
            gradient.setColorAt(0.0, QColor(250, 250, 250));
            gradient.setColorAt(1.0, QColor(240, 240, 240));
            break;
        case RibbonTheme::Dark:
            gradient.setColorAt(0.0, QColor(60, 60, 60));
            gradient.setColorAt(1.0, QColor(45, 45, 45));
            break;
        case RibbonTheme::Blue:
            gradient.setColorAt(0.0, QColor(227, 239, 255));
            gradient.setColorAt(1.0, QColor(199, 224, 255));
            break;
        default:
            gradient.setColorAt(0.0, d->themeManager->backgroundColor().lighter(105));
            gradient.setColorAt(1.0, d->themeManager->backgroundColor());
            break;
    }

    painter->fillRect(rect, gradient);

    // Draw border
    painter->setPen(QPen(d->themeManager->borderColor(), 1));
    painter->drawLine(rect.bottomLeft(), rect.bottomRight());
}

void RibbonBar::paintTabBackground(QPainter* painter) {
    if (d->minimized) {
        return;
    }

    QRect tabRect = d->tabWidget->geometry();
    tabRect.adjust(0, d->tabWidget->tabBar()->height(), 0, 0);

    // Fill tab content area
    painter->fillRect(tabRect, d->themeManager->backgroundColor());

    // Draw tab content border
    painter->setPen(QPen(d->themeManager->borderColor(), 1));
    painter->drawRect(tabRect.adjusted(0, 0, -1, -1));
}

// RibbonThemeManager Implementation (Singleton)
RibbonThemeManager* RibbonThemeManager::s_instance = nullptr;

RibbonThemeManager* RibbonThemeManager::instance() {
    if (!s_instance) {
        s_instance = new RibbonThemeManager(qApp);
    }
    return s_instance;
}

RibbonThemeManager::RibbonThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(RibbonTheme::Light)
    , m_followSystemTheme(false)
    , m_dpiScale(1.0)
    , m_systemThemeTimer(new QTimer(this))
{
    initializeDefaults();

    // Setup system theme detection
    m_systemThemeTimer->setSingleShot(true);
    m_systemThemeTimer->setInterval(100);
    connect(m_systemThemeTimer, &QTimer::timeout, this, &RibbonThemeManager::onSystemThemeChanged);

    // Load settings
    loadSettings();

    qCInfo(ribbonInterface) << "RibbonThemeManager initialized";
}

RibbonThemeManager::~RibbonThemeManager() = default;

void RibbonThemeManager::setTheme(RibbonTheme theme) {
    if (m_currentTheme == theme) {
        return;
    }

    m_currentTheme = theme;

    // Update color scheme
    if (m_colorScheme) {
        m_colorScheme->setTheme(theme);
    }

    updateStyleSheets();
    emit themeChanged(theme);

    qCInfo(ribbonInterface) << "Theme changed to:" << static_cast<int>(theme);
}

QColor RibbonThemeManager::backgroundColor() const {
    switch (m_currentTheme) {
        case RibbonTheme::Light:
            return QColor(245, 245, 245);
        case RibbonTheme::Dark:
            return QColor(45, 45, 45);
        case RibbonTheme::Blue:
            return QColor(227, 239, 255);
        case RibbonTheme::Silver:
            return QColor(240, 240, 240);
        case RibbonTheme::Black:
            return QColor(30, 30, 30);
        default:
            return QColor(245, 245, 245);
    }
}

QColor RibbonThemeManager::foregroundColor() const {
    switch (m_currentTheme) {
        case RibbonTheme::Light:
            return QColor(255, 255, 255);
        case RibbonTheme::Dark:
            return QColor(60, 60, 60);
        case RibbonTheme::Blue:
            return QColor(255, 255, 255);
        case RibbonTheme::Silver:
            return QColor(255, 255, 255);
        case RibbonTheme::Black:
            return QColor(45, 45, 45);
        default:
            return QColor(255, 255, 255);
    }
}

QColor RibbonThemeManager::accentColor() const {
    switch (m_currentTheme) {
        case RibbonTheme::Light:
            return QColor(0, 120, 215);
        case RibbonTheme::Dark:
            return QColor(0, 120, 215);
        case RibbonTheme::Blue:
            return QColor(43, 87, 154);
        case RibbonTheme::Silver:
            return QColor(75, 75, 75);
        case RibbonTheme::Black:
            return QColor(0, 120, 215);
        default:
            return QColor(0, 120, 215);
    }
}

QColor RibbonThemeManager::hoverColor() const {
    return accentColor().lighter(180);
}

QColor RibbonThemeManager::pressedColor() const {
    return accentColor().darker(120);
}

QColor RibbonThemeManager::borderColor() const {
    switch (m_currentTheme) {
        case RibbonTheme::Light:
            return QColor(171, 171, 171);
        case RibbonTheme::Dark:
            return QColor(100, 100, 100);
        case RibbonTheme::Blue:
            return QColor(158, 190, 245);
        case RibbonTheme::Silver:
            return QColor(165, 165, 165);
        case RibbonTheme::Black:
            return QColor(80, 80, 80);
        default:
            return QColor(171, 171, 171);
    }
}

QColor RibbonThemeManager::textColor() const {
    switch (m_currentTheme) {
        case RibbonTheme::Light:
            return QColor(68, 68, 68);
        case RibbonTheme::Dark:
            return QColor(255, 255, 255);
        case RibbonTheme::Blue:
            return QColor(21, 66, 139);
        case RibbonTheme::Silver:
            return QColor(68, 68, 68);
        case RibbonTheme::Black:
            return QColor(255, 255, 255);
        default:
            return QColor(68, 68, 68);
    }
}

QColor RibbonThemeManager::disabledTextColor() const {
    return textColor().lighter(150);
}

QFont RibbonThemeManager::defaultFont() const {
    return QFont("Segoe UI", 9);
}

QFont RibbonThemeManager::titleFont() const {
    return QFont("Segoe UI", 11, QFont::Bold);
}

QFont RibbonThemeManager::smallFont() const {
    return QFont("Segoe UI", 8);
}

int RibbonThemeManager::defaultSpacing() const {
    return 6;
}

int RibbonThemeManager::defaultMargin() const {
    return 4;
}

int RibbonThemeManager::buttonHeight() const {
    return 22;
}

int RibbonThemeManager::groupTitleHeight() const {
    return 18;
}

QString RibbonThemeManager::ribbonBarStyleSheet() const {
    return QString(
        "QWidget {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid %3;"
        "}"
    ).arg(backgroundColor().name())
     .arg(textColor().name())
     .arg(borderColor().name());
}

QString RibbonThemeManager::ribbonTabStyleSheet() const {
    return QString(
        "QTabWidget::pane {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "}"
        "QTabBar::tab {"
        "    background-color: %3;"
        "    color: %4;"
        "    padding: 4px 12px;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: %1;"
        "    border-bottom: 2px solid %5;"
        "}"
        "QTabBar::tab:hover {"
        "    background-color: %6;"
        "}"
    ).arg(foregroundColor().name())
     .arg(borderColor().name())
     .arg(backgroundColor().name())
     .arg(textColor().name())
     .arg(accentColor().name())
     .arg(hoverColor().name());
}

QString RibbonThemeManager::ribbonGroupStyleSheet() const {
    return QString(
        "QGroupBox {"
        "    background-color: %1;"
        "    border: 1px solid %2;"
        "    border-radius: 3px;"
        "    margin-top: 1ex;"
        "    font-weight: bold;"
        "    color: %3;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
    ).arg(foregroundColor().name())
     .arg(borderColor().name())
     .arg(textColor().name());
}

QString RibbonThemeManager::ribbonButtonStyleSheet() const {
    return QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: 1px solid transparent;"
        "    border-radius: 3px;"
        "    padding: 4px 8px;"
        "    min-height: 18px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %3;"
        "    border-color: %4;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %5;"
        "    border-color: %4;"
        "}"
        "QPushButton:checked {"
        "    background-color: %6;"
        "    border-color: %7;"
        "}"
        "QPushButton:disabled {"
        "    color: %8;"
        "    background-color: %9;"
        "}"
    ).arg(foregroundColor().name())
     .arg(textColor().name())
     .arg(hoverColor().name())
     .arg(borderColor().name())
     .arg(pressedColor().name())
     .arg(accentColor().lighter(180).name())
     .arg(accentColor().name())
     .arg(disabledTextColor().name())
     .arg(backgroundColor().darker(110).name());
}

void RibbonThemeManager::initializeDefaults() {
    // Initialize with default values
    m_currentTheme = RibbonTheme::Light;
    m_followSystemTheme = false;
    m_dpiScale = qApp->primaryScreen()->devicePixelRatio();
}

void RibbonThemeManager::updateStyleSheets() {
    // This would update all ribbon components with new styles
    // For now, just emit the signal
    emit colorsChanged();
}

void RibbonThemeManager::loadSettings() {
    QSettings settings;
    settings.beginGroup("RibbonTheme");

    int themeValue = settings.value("theme", static_cast<int>(RibbonTheme::Light)).toInt();
    m_currentTheme = static_cast<RibbonTheme>(themeValue);
    m_followSystemTheme = settings.value("followSystemTheme", false).toBool();

    settings.endGroup();

    if (m_followSystemTheme) {
        detectSystemTheme();
    }
}

void RibbonThemeManager::saveSettings() const {
    QSettings settings;
    settings.beginGroup("RibbonTheme");

    settings.setValue("theme", static_cast<int>(m_currentTheme));
    settings.setValue("followSystemTheme", m_followSystemTheme);

    settings.endGroup();
}

void RibbonThemeManager::detectSystemTheme() {
    // Detect system theme (simplified implementation)
    QPalette palette = qApp->palette();
    bool isDark = palette.color(QPalette::Window).lightness() < 128;

    RibbonTheme detectedTheme = isDark ? RibbonTheme::Dark : RibbonTheme::Light;
    if (detectedTheme != m_currentTheme) {
        setTheme(detectedTheme);
    }
}

void RibbonThemeManager::onSystemThemeChanged() {
    if (m_followSystemTheme) {
        detectSystemTheme();
    }
}
