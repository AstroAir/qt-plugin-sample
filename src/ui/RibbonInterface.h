// RibbonInterface.h - Modern Ribbon Interface Framework
#pragma once

#include <QWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QSplitter>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QGraphicsDropShadowEffect>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QScreen>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <QMap>
#include <QList>
#include <memory>
#include "ui/RibbonControls.h"

// Forward declarations
class RibbonBar;
class RibbonTab;
class RibbonColorScheme;
class RibbonGroup;
class RibbonButton;
class RibbonGallery;
class RibbonQuickAccessToolbar;
class RibbonApplicationButton;
class RibbonThemeManager;

// Ribbon control types
enum class RibbonControlType {
    Button,
    SplitButton,
    DropdownButton,
    ToggleButton,
    Gallery,
    ComboBox,
    SpinBox,
    Slider,
    Separator,
    Label,
    Custom
};

// Note: RibbonButtonSize enum is defined in ui/RibbonControls.h

// Ribbon themes
enum class RibbonTheme {
    Light,
    Dark,
    Blue,
    Silver,
    Black,
    Custom
};

// Ribbon animation types
enum class RibbonAnimation {
    None,
    Fade,
    Slide,
    Expand,
    Bounce
};

// Ribbon control configuration
struct RibbonControlConfig {
    QString id;
    QString text;
    QString tooltip;
    QString icon;
    RibbonControlType type = RibbonControlType::Button;
    RibbonButtonSize size = RibbonButtonSize::Large;
    bool enabled = true;
    bool visible = true;
    bool checkable = false;
    bool checked = false;
    QStringList items; // For combo boxes, galleries
    QVariant data;
    QString shortcut;
    
    RibbonControlConfig() = default;
    RibbonControlConfig(const QString& i, const QString& t, const QString& ic = "")
        : id(i), text(t), icon(ic) {}
};

// Main ribbon bar widget
class RibbonBar : public QWidget {
    Q_OBJECT
    Q_PROPERTY(RibbonTheme theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(bool minimized READ isMinimized WRITE setMinimized NOTIFY minimizedChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled)

public:
    explicit RibbonBar(QWidget* parent = nullptr);
    ~RibbonBar() override;

    // Tab management
    RibbonTab* addTab(const QString& title, const QString& id = "");
    RibbonTab* insertTab(int index, const QString& title, const QString& id = "");
    void removeTab(int index);
    void removeTab(const QString& id);
    RibbonTab* tab(int index) const;
    RibbonTab* tab(const QString& id) const;
    int tabCount() const;
    
    // Current tab
    int currentTabIndex() const;
    RibbonTab* currentTab() const;
    void setCurrentTab(int index);
    void setCurrentTab(const QString& id);
    
    // Quick Access Toolbar
    RibbonQuickAccessToolbar* quickAccessToolbar() const;
    void setQuickAccessToolbarVisible(bool visible);
    bool isQuickAccessToolbarVisible() const;
    
    // Application button
    RibbonApplicationButton* applicationButton() const;
    void setApplicationButtonVisible(bool visible);
    bool isApplicationButtonVisible() const;
    
    // Minimization
    bool isMinimized() const;
    void setMinimized(bool minimized);
    void toggleMinimized();
    
    // Theme and styling
    RibbonTheme theme() const;
    void setTheme(RibbonTheme theme);
    void setCustomTheme(const QJsonObject& themeData);
    
    // Animations
    bool animationsEnabled() const;
    void setAnimationsEnabled(bool enabled);
    void setAnimationDuration(int milliseconds);
    int animationDuration() const;
    
    // Layout and sizing
    void setTabHeight(int height);
    int tabHeight() const;
    void setGroupSpacing(int spacing);
    int groupSpacing() const;
    
    // Customization
    void saveLayout(const QString& filePath);
    void loadLayout(const QString& filePath);
    QJsonObject exportLayout() const;
    void importLayout(const QJsonObject& layout);
    
    // Context tabs (contextual tabs that appear based on selection)
    void addContextTab(const QString& title, const QString& context, const QString& id = "");
    void removeContextTab(const QString& id);
    void showContextTabs(const QString& context);
    void hideContextTabs(const QString& context);

signals:
    void currentTabChanged(int index, const QString& id);
    void tabAdded(int index, const QString& id);
    void tabRemoved(int index, const QString& id);
    void minimizedChanged(bool minimized);
    void themeChanged(RibbonTheme theme);
    void customizationRequested();

public slots:
    void showCustomizationDialog();
    void resetToDefaults();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* object, QEvent* event) override;

private slots:
    void onTabChanged(int index);
    void onTabBarDoubleClicked();
    void onAnimationFinished();

private:
    struct RibbonBarPrivate;
    std::unique_ptr<RibbonBarPrivate> d;
    
    void setupUI();
    void setupAnimations();
    void updateTheme();
    void updateLayout();
    void animateMinimize(bool minimize);
    void paintBackground(QPainter* painter);
    void paintTabBackground(QPainter* painter);
};

// Ribbon tab widget
class RibbonTab : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(bool contextual READ isContextual WRITE setContextual)

public:
    explicit RibbonTab(const QString& title, const QString& id = "", QWidget* parent = nullptr);
    ~RibbonTab() override;

    // Basic properties
    QString title() const;
    void setTitle(const QString& title);
    QString id() const;
    void setId(const QString& id);
    
    // Contextual tabs
    bool isContextual() const;
    void setContextual(bool contextual);
    QString context() const;
    void setContext(const QString& context);
    
    // Group management
    RibbonGroup* addGroup(const QString& title, const QString& id = "");
    RibbonGroup* insertGroup(int index, const QString& title, const QString& id = "");
    void removeGroup(int index);
    void removeGroup(const QString& id);
    RibbonGroup* group(int index) const;
    RibbonGroup* group(const QString& id) const;
    int groupCount() const;
    
    // Layout
    void setGroupSpacing(int spacing);
    int groupSpacing() const;
    void setMargins(int left, int top, int right, int bottom);
    QMargins margins() const;

signals:
    void titleChanged(const QString& title);
    void groupAdded(int index, const QString& id);
    void groupRemoved(int index, const QString& id);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct RibbonTabPrivate;
    std::unique_ptr<RibbonTabPrivate> d;
    
    void setupUI();
    void updateLayout();
};

// Ribbon group widget
class RibbonGroup : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(bool collapsible READ isCollapsible WRITE setCollapsible)
    Q_PROPERTY(bool collapsed READ isCollapsed WRITE setCollapsed NOTIFY collapsedChanged)

public:
    explicit RibbonGroup(const QString& title, const QString& id = "", QWidget* parent = nullptr);
    ~RibbonGroup() override;

    // Basic properties
    QString title() const;
    void setTitle(const QString& title);
    QString id() const;
    void setId(const QString& id);
    
    // Collapsing
    bool isCollapsible() const;
    void setCollapsible(bool collapsible);
    bool isCollapsed() const;
    void setCollapsed(bool collapsed);
    void toggleCollapsed();
    
    // Control management
    QWidget* addControl(const RibbonControlConfig& config);
    QWidget* insertControl(int index, const RibbonControlConfig& config);
    void removeControl(int index);
    void removeControl(const QString& id);
    QWidget* control(int index) const;
    QWidget* control(const QString& id) const;
    int controlCount() const;
    
    // Layout options
    void addSeparator();
    void addStretch();
    void setControlSpacing(int spacing);
    int controlSpacing() const;
    
    // Quick actions
    RibbonButton* addButton(const QString& text, const QString& icon = "", const QString& id = "");
    RibbonButton* addLargeButton(const QString& text, const QString& icon = "", const QString& id = "");
    RibbonButton* addSmallButton(const QString& text, const QString& icon = "", const QString& id = "");
    QComboBox* addComboBox(const QString& id = "");
    QLabel* addLabel(const QString& text, const QString& id = "");

signals:
    void titleChanged(const QString& title);
    void collapsedChanged(bool collapsed);
    void controlAdded(int index, const QString& id);
    void controlRemoved(int index, const QString& id);
    void controlClicked(const QString& id);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onControlClicked();

private:
    struct RibbonGroupPrivate;
    std::unique_ptr<RibbonGroupPrivate> d;
    
    void setupUI();
    void updateLayout();
    void paintGroupFrame(QPainter* painter);
    void paintGroupTitle(QPainter* painter);
    QWidget* createControl(const RibbonControlConfig& config);
};

// Quick Access Toolbar
class RibbonQuickAccessToolbar : public QWidget {
    Q_OBJECT

public:
    explicit RibbonQuickAccessToolbar(QWidget* parent = nullptr);
    ~RibbonQuickAccessToolbar() override;

    // Button management
    void addAction(QAction* action);
    void removeAction(QAction* action);
    void addSeparator();
    void clear();
    
    // Customization
    void setCustomizable(bool customizable);
    bool isCustomizable() const;
    void showCustomizationMenu();

signals:
    void customizationRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    struct QuickAccessToolbarPrivate;
    std::unique_ptr<QuickAccessToolbarPrivate> d;
    
    void setupUI();
};

// Application Button (Office Button)
class RibbonApplicationButton : public QPushButton {
    Q_OBJECT

public:
    explicit RibbonApplicationButton(QWidget* parent = nullptr);
    ~RibbonApplicationButton() override;

    // Menu management
    void setApplicationMenu(QMenu* menu);
    QMenu* applicationMenu() const;
    
    // Recent files
    void addRecentFile(const QString& filePath);
    void removeRecentFile(const QString& filePath);
    void clearRecentFiles();
    QStringList recentFiles() const;

signals:
    void recentFileClicked(const QString& filePath);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct ApplicationButtonPrivate;
    std::unique_ptr<ApplicationButtonPrivate> d;
    
    void setupUI();
    void updateRecentFilesMenu();
};

// Ribbon Theme Manager
class RibbonThemeManager : public QObject {
    Q_OBJECT

public:
    static RibbonThemeManager* instance();

    // Theme management
    void setTheme(RibbonTheme theme);
    RibbonTheme currentTheme() const;
    void setCustomTheme(const QJsonObject& themeData);
    QJsonObject customTheme() const;

    // Color schemes
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor accentColor() const;
    QColor hoverColor() const;
    QColor pressedColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    QColor disabledTextColor() const;

    // Fonts
    QFont defaultFont() const;
    QFont titleFont() const;
    QFont smallFont() const;

    // Metrics
    int defaultSpacing() const;
    int defaultMargin() const;
    int buttonHeight() const;
    int groupTitleHeight() const;

    // Style sheets
    QString ribbonBarStyleSheet() const;
    QString ribbonTabStyleSheet() const;
    QString ribbonGroupStyleSheet() const;
    QString ribbonButtonStyleSheet() const;

    // Settings
    void loadSettings();
    void saveSettings() const;
    void detectSystemTheme();

signals:
    void themeChanged(RibbonTheme theme);
    void colorsChanged();

private slots:
    void onSystemThemeChanged();

private:
    explicit RibbonThemeManager(QObject* parent = nullptr);
    ~RibbonThemeManager() override;

    static RibbonThemeManager* s_instance;

    RibbonTheme m_currentTheme;
    RibbonColorScheme* m_colorScheme;
    bool m_followSystemTheme;
    qreal m_dpiScale;
    QTimer* m_systemThemeTimer;

    void initializeDefaults();
    void updateStyleSheets();
};

// Utility macros for ribbon creation
#define RIBBON_TAB(ribbon, title) ribbon->addTab(title)
#define RIBBON_GROUP(tab, title) tab->addGroup(title)
#define RIBBON_BUTTON(group, text, icon) group->addButton(text, icon)
#define RIBBON_LARGE_BUTTON(group, text, icon) group->addLargeButton(text, icon)
#define RIBBON_SMALL_BUTTON(group, text, icon) group->addSmallButton(text, icon)
