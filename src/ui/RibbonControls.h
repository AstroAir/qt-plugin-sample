// RibbonControls.h - Comprehensive Ribbon Controls Library
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QProgressBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QListWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QFrame>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QStackedLayout>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsOpacityEffect>
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QEnterEvent>
#include <QTimer>
#include <QPixmap>
#include <QIcon>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QGradient>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QVariant>
#include <memory>

// Forward declarations
class RibbonButton;
class RibbonSplitButton;
class RibbonDropdownButton;
class RibbonToggleButton;
class RibbonGallery;
class RibbonColorPicker;
class RibbonFontComboBox;
class RibbonSpinBox;
class RibbonSlider;
class RibbonSeparator;
class RibbonLabel;
class RibbonLineEdit;
class RibbonTextEdit;
class RibbonCheckBox;
class RibbonRadioButton;

// Ribbon button states
enum class RibbonButtonState {
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Checked
};

// Ribbon button sizes
enum class RibbonButtonSize {
    Small,      // Small button (16x16 icon)
    Medium,     // Medium button (24x24 icon)
    Large       // Large button (32x32 icon)
};

// Ribbon control animations
enum class RibbonControlAnimation {
    None,
    Fade,
    Glow,
    Bounce,
    Slide,
    Scale
};

// Base class for all ribbon controls
class RibbonControl : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled)
    Q_PROPERTY(RibbonControlAnimation animation READ animation WRITE setAnimation)

public:
    explicit RibbonControl(QWidget* parent = nullptr);
    ~RibbonControl() override;

    // Basic properties
    QString id() const;
    void setId(const QString& id);
    
    // Animations
    bool animationsEnabled() const;
    void setAnimationsEnabled(bool enabled);
    RibbonControlAnimation animation() const;
    void setAnimation(RibbonControlAnimation animation);
    
    // State management
    virtual void setControlState(RibbonButtonState state);
    RibbonButtonState controlState() const;
    
    // Theming
    virtual void updateTheme();

signals:
    void clicked();
    void stateChanged(RibbonButtonState state);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    
    virtual void paintBackground(QPainter* painter, const QRect& rect);
    virtual void paintBorder(QPainter* painter, const QRect& rect);
    virtual void startAnimation();
    virtual void stopAnimation();

private:
    struct RibbonControlPrivate;
    std::unique_ptr<RibbonControlPrivate> d;
};

// Modern ribbon button with multiple sizes and styles
class RibbonButton : public RibbonControl {
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(RibbonButtonSize buttonSize READ buttonSize WRITE setButtonSize)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)

public:
    explicit RibbonButton(QWidget* parent = nullptr);
    explicit RibbonButton(const QString& text, QWidget* parent = nullptr);
    explicit RibbonButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    ~RibbonButton() override;

    // Text and icon
    QString text() const;
    void setText(const QString& text);
    QIcon icon() const;
    void setIcon(const QIcon& icon);
    
    // Button size
    RibbonButtonSize buttonSize() const;
    void setButtonSize(RibbonButtonSize size);
    
    // Checkable state
    bool isCheckable() const;
    void setCheckable(bool checkable);
    bool isChecked() const;
    void setChecked(bool checked);
    
    // Menu support
    void setMenu(QMenu* menu);
    QMenu* menu() const;
    
    // Shortcuts
    void setShortcut(const QKeySequence& shortcut);
    QKeySequence shortcut() const;

public slots:
    void click();
    void toggle();

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    struct RibbonButtonPrivate;
    std::unique_ptr<RibbonButtonPrivate> d;
    
    void setupUI();
    void updateLayout();
    void paintButton(QPainter* painter);
    void paintIcon(QPainter* painter, const QRect& iconRect);
    void paintText(QPainter* painter, const QRect& textRect);
    QRect calculateIconRect() const;
    QRect calculateTextRect() const;
};

// Split button with dropdown arrow
class RibbonSplitButton : public RibbonButton {
    Q_OBJECT

public:
    explicit RibbonSplitButton(QWidget* parent = nullptr);
    explicit RibbonSplitButton(const QString& text, QWidget* parent = nullptr);
    ~RibbonSplitButton() override;

    // Dropdown menu
    void setDropdownMenu(QMenu* menu);
    QMenu* dropdownMenu() const;
    
    // Split behavior
    void setSplitPolicy(Qt::ToolButtonStyle policy);
    Qt::ToolButtonStyle splitPolicy() const;

signals:
    void dropdownClicked();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    QSize sizeHint() const override;

private:
    struct SplitButtonPrivate;
    std::unique_ptr<SplitButtonPrivate> d;
    
    void paintDropdownArrow(QPainter* painter, const QRect& arrowRect);
    QRect calculateDropdownRect() const;
    bool isDropdownArea(const QPoint& pos) const;
};

// Dropdown button (menu only)
class RibbonDropdownButton : public RibbonButton {
    Q_OBJECT

public:
    explicit RibbonDropdownButton(QWidget* parent = nullptr);
    explicit RibbonDropdownButton(const QString& text, QWidget* parent = nullptr);
    ~RibbonDropdownButton() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void showDropdownMenu();
};

// Toggle button with on/off states
class RibbonToggleButton : public RibbonButton {
    Q_OBJECT

public:
    explicit RibbonToggleButton(QWidget* parent = nullptr);
    explicit RibbonToggleButton(const QString& text, QWidget* parent = nullptr);
    ~RibbonToggleButton() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void paintToggleIndicator(QPainter* painter, const QRect& rect);
};

// Gallery control for displaying collections of items
class RibbonGallery : public RibbonControl {
    Q_OBJECT
    Q_PROPERTY(int itemSize READ itemSize WRITE setItemSize)
    Q_PROPERTY(int columns READ columns WRITE setColumns)
    Q_PROPERTY(bool scrollable READ isScrollable WRITE setScrollable)

public:
    struct GalleryItem {
        QString id;
        QString text;
        QIcon icon;
        QVariant data;
        bool enabled = true;
        
        GalleryItem() = default;
        GalleryItem(const QString& i, const QString& t, const QIcon& ic = QIcon())
            : id(i), text(t), icon(ic) {}
    };

    explicit RibbonGallery(QWidget* parent = nullptr);
    ~RibbonGallery() override;

    // Item management
    void addItem(const GalleryItem& item);
    void insertItem(int index, const GalleryItem& item);
    void removeItem(int index);
    void removeItem(const QString& id);
    void clear();
    
    GalleryItem item(int index) const;
    GalleryItem item(const QString& id) const;
    int itemCount() const;
    
    // Selection
    int currentIndex() const;
    void setCurrentIndex(int index);
    QString currentId() const;
    void setCurrentId(const QString& id);
    GalleryItem currentItem() const;
    
    // Layout
    int itemSize() const;
    void setItemSize(int size);
    int columns() const;
    void setColumns(int columns);
    
    // Scrolling
    bool isScrollable() const;
    void setScrollable(bool scrollable);
    
    // Categories
    void addCategory(const QString& name);
    void setCategoryItems(const QString& category, const QList<GalleryItem>& items);

signals:
    void itemClicked(int index, const QString& id);
    void itemDoubleClicked(int index, const QString& id);
    void currentItemChanged(int index, const QString& id);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    QSize sizeHint() const override;

private:
    struct GalleryPrivate;
    std::unique_ptr<GalleryPrivate> d;
    
    void setupUI();
    void updateLayout();
    void paintItems(QPainter* painter);
    void paintItem(QPainter* painter, const GalleryItem& item, const QRect& rect, bool selected);
    int itemAt(const QPoint& pos) const;
    QRect itemRect(int index) const;
};

// Color picker control
class RibbonColorPicker : public RibbonControl {
    Q_OBJECT
    Q_PROPERTY(QColor currentColor READ currentColor WRITE setCurrentColor)
    Q_PROPERTY(bool showNoColor READ showNoColor WRITE setShowNoColor)
    Q_PROPERTY(bool showMoreColors READ showMoreColors WRITE setShowMoreColors)

public:
    explicit RibbonColorPicker(QWidget* parent = nullptr);
    ~RibbonColorPicker() override;

    // Color management
    QColor currentColor() const;
    void setCurrentColor(const QColor& color);
    
    // Standard colors
    void setStandardColors(const QList<QColor>& colors);
    QList<QColor> standardColors() const;
    
    // Theme colors
    void setThemeColors(const QList<QColor>& colors);
    QList<QColor> themeColors() const;
    
    // Recent colors
    void addRecentColor(const QColor& color);
    void clearRecentColors();
    QList<QColor> recentColors() const;
    
    // Options
    bool showNoColor() const;
    void setShowNoColor(bool show);
    bool showMoreColors() const;
    void setShowMoreColors(bool show);

signals:
    void colorChanged(const QColor& color);
    void noColorSelected();
    void moreColorsRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    struct ColorPickerPrivate;
    std::unique_ptr<ColorPickerPrivate> d;
    
    void setupUI();
    void paintColorGrid(QPainter* painter);
    void paintColorCell(QPainter* painter, const QColor& color, const QRect& rect, bool selected);
    QColor colorAt(const QPoint& pos) const;
    void showColorDialog();
};

// Font combo box with preview
class RibbonFontComboBox : public QComboBox {
    Q_OBJECT
    Q_PROPERTY(bool showPreview READ showPreview WRITE setShowPreview)

public:
    explicit RibbonFontComboBox(QWidget* parent = nullptr);
    ~RibbonFontComboBox() override;

    // Preview
    bool showPreview() const;
    void setShowPreview(bool show);
    
    // Font filtering
    void setFontFilters(QFontComboBox::FontFilters filters);
    QFontComboBox::FontFilters fontFilters() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct FontComboBoxPrivate;
    std::unique_ptr<FontComboBoxPrivate> d;
    
    void setupUI();
    void updateFontList();
};

// Enhanced spin box for ribbon
class RibbonSpinBox : public QSpinBox {
    Q_OBJECT

public:
    explicit RibbonSpinBox(QWidget* parent = nullptr);
    ~RibbonSpinBox() override;

    // Quick increment buttons
    void setQuickIncrements(const QList<int>& increments);
    QList<int> quickIncrements() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct SpinBoxPrivate;
    std::unique_ptr<SpinBoxPrivate> d;
    
    void setupUI();
};

// Enhanced slider for ribbon
class RibbonSlider : public QSlider {
    Q_OBJECT
    Q_PROPERTY(bool showValue READ showValue WRITE setShowValue)
    Q_PROPERTY(bool showTicks READ showTicks WRITE setShowTicks)

public:
    explicit RibbonSlider(Qt::Orientation orientation, QWidget* parent = nullptr);
    ~RibbonSlider() override;

    // Value display
    bool showValue() const;
    void setShowValue(bool show);
    
    // Tick marks
    bool showTicks() const;
    void setShowTicks(bool show);
    
    // Custom labels
    void setCustomLabels(const QMap<int, QString>& labels);
    QMap<int, QString> customLabels() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct SliderPrivate;
    std::unique_ptr<SliderPrivate> d;
    
    void setupUI();
    void paintCustomLabels(QPainter* painter);
};

// Ribbon separator
class RibbonSeparator : public QFrame {
    Q_OBJECT

public:
    explicit RibbonSeparator(Qt::Orientation orientation = Qt::Vertical, QWidget* parent = nullptr);
    ~RibbonSeparator() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Qt::Orientation m_orientation;
};

// Enhanced label for ribbon
class RibbonLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap)

public:
    explicit RibbonLabel(QWidget* parent = nullptr);
    explicit RibbonLabel(const QString& text, QWidget* parent = nullptr);
    ~RibbonLabel() override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

// Enhanced line edit for ribbon
class RibbonLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit RibbonLineEdit(QWidget* parent = nullptr);
    explicit RibbonLineEdit(const QString& text, QWidget* parent = nullptr);
    ~RibbonLineEdit() override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private:
    void setupUI();
};

// Enhanced check box for ribbon
class RibbonCheckBox : public QCheckBox {
    Q_OBJECT

public:
    explicit RibbonCheckBox(QWidget* parent = nullptr);
    explicit RibbonCheckBox(const QString& text, QWidget* parent = nullptr);
    ~RibbonCheckBox() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
};

// Enhanced radio button for ribbon
class RibbonRadioButton : public QRadioButton {
    Q_OBJECT

public:
    explicit RibbonRadioButton(QWidget* parent = nullptr);
    explicit RibbonRadioButton(const QString& text, QWidget* parent = nullptr);
    ~RibbonRadioButton() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void setupUI();
};
