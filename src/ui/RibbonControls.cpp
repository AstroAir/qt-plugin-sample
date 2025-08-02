// RibbonControls.cpp - Implementation of Ribbon Controls Library
#include "RibbonControls.h"
#include "RibbonInterface.h"
#include <QApplication>
#include <QStylePainter>
#include <QStyleOption>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFontMetrics>
#include <QTextOption>
#include <QColorDialog>
#include <QFontDatabase>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(ribbonControls, "ui.ribbon.controls")

// RibbonControl Private Implementation
struct RibbonControl::RibbonControlPrivate {
    QString id;
    RibbonButtonState state = RibbonButtonState::Normal;
    bool animationsEnabled = true;
    RibbonControlAnimation animation = RibbonControlAnimation::Fade;
    QPropertyAnimation* stateAnimation = nullptr;
    QGraphicsEffect* effect = nullptr;
    
    explicit RibbonControlPrivate(RibbonControl* parent) {
        stateAnimation = new QPropertyAnimation(parent, "geometry", parent);
        stateAnimation->setDuration(150);
        stateAnimation->setEasingCurve(QEasingCurve::OutCubic);
    }
};

// RibbonControl Implementation
RibbonControl::RibbonControl(QWidget* parent)
    : QWidget(parent)
    , d(std::make_unique<RibbonControlPrivate>(this))
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover, true);
    
    // Apply theme
    updateTheme();
}

RibbonControl::~RibbonControl() = default;

QString RibbonControl::id() const {
    return d->id;
}

void RibbonControl::setId(const QString& id) {
    d->id = id;
}

bool RibbonControl::animationsEnabled() const {
    return d->animationsEnabled;
}

void RibbonControl::setAnimationsEnabled(bool enabled) {
    d->animationsEnabled = enabled;
}

RibbonControlAnimation RibbonControl::animation() const {
    return d->animation;
}

void RibbonControl::setAnimation(RibbonControlAnimation animation) {
    d->animation = animation;
}

void RibbonControl::setControlState(RibbonButtonState state) {
    if (d->state == state) {
        return;
    }
    
    RibbonButtonState oldState = d->state;
    Q_UNUSED(oldState) // Reserved for future animation transitions
    d->state = state;
    
    if (d->animationsEnabled) {
        startAnimation();
    }
    
    update();
    emit stateChanged(state);
}

RibbonButtonState RibbonControl::controlState() const {
    return d->state;
}

void RibbonControl::updateTheme() {
    // Apply theme-specific styling
    RibbonThemeManager* themeManager = RibbonThemeManager::instance();
    
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, themeManager->backgroundColor());
    palette.setColor(QPalette::WindowText, themeManager->textColor());
    palette.setColor(QPalette::Button, themeManager->backgroundColor());
    palette.setColor(QPalette::ButtonText, themeManager->textColor());
    setPalette(palette);
    
    setFont(themeManager->defaultFont());
    update();
}

void RibbonControl::enterEvent(QEnterEvent* event) {
    QWidget::enterEvent(event);
    if (isEnabled()) {
        setControlState(RibbonButtonState::Hovered);
    }
}

void RibbonControl::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
    if (isEnabled()) {
        setControlState(RibbonButtonState::Normal);
    }
}

void RibbonControl::mousePressEvent(QMouseEvent* event) {
    QWidget::mousePressEvent(event);
    if (event->button() == Qt::LeftButton && isEnabled()) {
        setControlState(RibbonButtonState::Pressed);
    }
}

void RibbonControl::mouseReleaseEvent(QMouseEvent* event) {
    QWidget::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton && isEnabled()) {
        if (rect().contains(event->pos())) {
            setControlState(RibbonButtonState::Hovered);
            emit clicked();
        } else {
            setControlState(RibbonButtonState::Normal);
        }
    }
}

void RibbonControl::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    update();
}

void RibbonControl::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    update();
}

void RibbonControl::paintBackground(QPainter* painter, const QRect& rect) {
    RibbonThemeManager* themeManager = RibbonThemeManager::instance();
    
    QColor backgroundColor;
    switch (d->state) {
        case RibbonButtonState::Normal:
            backgroundColor = themeManager->backgroundColor();
            break;
        case RibbonButtonState::Hovered:
            backgroundColor = themeManager->hoverColor();
            break;
        case RibbonButtonState::Pressed:
            backgroundColor = themeManager->pressedColor();
            break;
        case RibbonButtonState::Disabled:
            backgroundColor = themeManager->backgroundColor().darker(110);
            break;
        case RibbonButtonState::Checked:
            backgroundColor = themeManager->accentColor().lighter(150);
            break;
    }
    
    // Create gradient
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0.0, backgroundColor.lighter(105));
    gradient.setColorAt(1.0, backgroundColor.darker(105));
    
    painter->fillRect(rect, gradient);
}

void RibbonControl::paintBorder(QPainter* painter, const QRect& rect) {
    if (d->state == RibbonButtonState::Normal) {
        return; // No border for normal state
    }
    
    RibbonThemeManager* themeManager = RibbonThemeManager::instance();
    
    QColor borderColor;
    switch (d->state) {
        case RibbonButtonState::Hovered:
            borderColor = themeManager->borderColor();
            break;
        case RibbonButtonState::Pressed:
            borderColor = themeManager->borderColor().darker(120);
            break;
        case RibbonButtonState::Checked:
            borderColor = themeManager->accentColor();
            break;
        default:
            borderColor = themeManager->borderColor();
            break;
    }
    
    painter->setPen(QPen(borderColor, 1));
    painter->drawRect(rect.adjusted(0, 0, -1, -1));
}

void RibbonControl::startAnimation() {
    if (!d->animationsEnabled) {
        return;
    }
    
    switch (d->animation) {
        case RibbonControlAnimation::Fade: {
            if (!d->effect) {
                d->effect = new QGraphicsOpacityEffect(this);
                setGraphicsEffect(d->effect);
            }
            auto opacityEffect = qobject_cast<QGraphicsOpacityEffect*>(d->effect);
            if (opacityEffect) {
                QPropertyAnimation* animation = new QPropertyAnimation(opacityEffect, "opacity", this);
                animation->setDuration(150);
                animation->setStartValue(0.7);
                animation->setEndValue(1.0);
                animation->start(QAbstractAnimation::DeleteWhenStopped);
            }
            break;
        }
        case RibbonControlAnimation::Glow: {
            if (!d->effect) {
                d->effect = new QGraphicsDropShadowEffect(this);
                setGraphicsEffect(d->effect);
            }
            auto shadowEffect = qobject_cast<QGraphicsDropShadowEffect*>(d->effect);
            if (shadowEffect) {
                shadowEffect->setBlurRadius(10);
                shadowEffect->setColor(RibbonThemeManager::instance()->accentColor());
                shadowEffect->setOffset(0, 0);
            }
            break;
        }
        default:
            break;
    }
}

void RibbonControl::stopAnimation() {
    if (d->effect) {
        setGraphicsEffect(nullptr);
        d->effect = nullptr;
    }
}

// RibbonButton Private Implementation
struct RibbonButton::RibbonButtonPrivate {
    QString text;
    QIcon icon;
    RibbonButtonSize buttonSize = RibbonButtonSize::Large;
    bool checkable = false;
    bool checked = false;
    QMenu* menu = nullptr;
    QKeySequence shortcut;
    
    // Layout calculations
    QRect iconRect;
    QRect textRect;
    QSize cachedSizeHint;
    bool sizeHintValid = false;
};

// RibbonButton Implementation
RibbonButton::RibbonButton(QWidget* parent)
    : RibbonControl(parent)
    , d(std::make_unique<RibbonButtonPrivate>())
{
    setupUI();
}

RibbonButton::RibbonButton(const QString& text, QWidget* parent)
    : RibbonControl(parent)
    , d(std::make_unique<RibbonButtonPrivate>())
{
    d->text = text;
    setupUI();
}

RibbonButton::RibbonButton(const QIcon& icon, const QString& text, QWidget* parent)
    : RibbonControl(parent)
    , d(std::make_unique<RibbonButtonPrivate>())
{
    d->icon = icon;
    d->text = text;
    setupUI();
}

RibbonButton::~RibbonButton() = default;

QString RibbonButton::text() const {
    return d->text;
}

void RibbonButton::setText(const QString& text) {
    if (d->text == text) {
        return;
    }
    
    d->text = text;
    d->sizeHintValid = false;
    updateLayout();
    update();
}

QIcon RibbonButton::icon() const {
    return d->icon;
}

void RibbonButton::setIcon(const QIcon& icon) {
    d->icon = icon;
    d->sizeHintValid = false;
    updateLayout();
    update();
}

RibbonButtonSize RibbonButton::buttonSize() const {
    return d->buttonSize;
}

void RibbonButton::setButtonSize(RibbonButtonSize size) {
    if (d->buttonSize == size) {
        return;
    }
    
    d->buttonSize = size;
    d->sizeHintValid = false;
    updateLayout();
    update();
}

bool RibbonButton::isCheckable() const {
    return d->checkable;
}

void RibbonButton::setCheckable(bool checkable) {
    d->checkable = checkable;
    if (!checkable) {
        setChecked(false);
    }
}

bool RibbonButton::isChecked() const {
    return d->checked;
}

void RibbonButton::setChecked(bool checked) {
    if (!d->checkable || d->checked == checked) {
        return;
    }
    
    d->checked = checked;
    setControlState(checked ? RibbonButtonState::Checked : RibbonButtonState::Normal);
    emit toggled(checked);
}

void RibbonButton::setMenu(QMenu* menu) {
    d->menu = menu;
}

QMenu* RibbonButton::menu() const {
    return d->menu;
}

void RibbonButton::setShortcut(const QKeySequence& shortcut) {
    d->shortcut = shortcut;
    setToolTip(d->text + (shortcut.isEmpty() ? "" : " (" + shortcut.toString() + ")"));
}

QKeySequence RibbonButton::shortcut() const {
    return d->shortcut;
}

void RibbonButton::click() {
    if (!isEnabled()) {
        return;
    }
    
    if (d->checkable) {
        toggle();
    }
    
    if (d->menu) {
        QPoint menuPos = mapToGlobal(QPoint(0, height()));
        d->menu->exec(menuPos);
    }
    
    emit clicked();
}

void RibbonButton::toggle() {
    if (d->checkable) {
        setChecked(!d->checked);
    }
}

void RibbonButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    paintButton(&painter);
}

void RibbonButton::mousePressEvent(QMouseEvent* event) {
    RibbonControl::mousePressEvent(event);
    
    if (event->button() == Qt::LeftButton) {
        click();
    }
}

void RibbonButton::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
        click();
        event->accept();
        return;
    }
    
    RibbonControl::keyPressEvent(event);
}

QSize RibbonButton::sizeHint() const {
    if (d->sizeHintValid) {
        return d->cachedSizeHint;
    }
    
    QFontMetrics fm(font());
    QSize iconSize;
    QSize textSize;
    
    // Calculate icon size
    switch (d->buttonSize) {
        case RibbonButtonSize::Small:
            iconSize = QSize(16, 16);
            break;
        case RibbonButtonSize::Medium:
            iconSize = QSize(24, 24);
            break;
        case RibbonButtonSize::Large:
            iconSize = QSize(32, 32);
            break;
    }
    
    // Calculate text size
    if (!d->text.isEmpty()) {
        if (d->buttonSize == RibbonButtonSize::Small) {
            textSize = fm.size(Qt::TextSingleLine, d->text);
        } else {
            // For medium and large buttons, text can wrap
            QRect textRect = fm.boundingRect(QRect(0, 0, iconSize.width() + 20, 1000), 
                                           Qt::AlignCenter | Qt::TextWordWrap, d->text);
            textSize = textRect.size();
        }
    }
    
    // Calculate total size
    QSize totalSize;
    if (d->buttonSize == RibbonButtonSize::Small) {
        // Icon and text side by side
        totalSize.setWidth(iconSize.width() + textSize.width() + 12);
        totalSize.setHeight(qMax(iconSize.height(), textSize.height()) + 8);
    } else {
        // Icon above text
        totalSize.setWidth(qMax(iconSize.width(), textSize.width()) + 12);
        totalSize.setHeight(iconSize.height() + textSize.height() + 12);
    }
    
    // Minimum size constraints
    totalSize = totalSize.expandedTo(QSize(32, 22));
    
    d->cachedSizeHint = totalSize;
    d->sizeHintValid = true;
    
    return totalSize;
}

QSize RibbonButton::minimumSizeHint() const {
    return sizeHint();
}

void RibbonButton::setupUI() {
    setMinimumSize(32, 22);
    updateLayout();
}

void RibbonButton::updateLayout() {
    d->iconRect = calculateIconRect();
    d->textRect = calculateTextRect();
    d->sizeHintValid = false;
    updateGeometry();
}

void RibbonButton::paintButton(QPainter* painter) {
    QRect rect = this->rect();
    
    // Paint background
    paintBackground(painter, rect);
    
    // Paint border
    paintBorder(painter, rect);
    
    // Paint icon
    if (!d->icon.isNull()) {
        paintIcon(painter, d->iconRect);
    }
    
    // Paint text
    if (!d->text.isEmpty()) {
        paintText(painter, d->textRect);
    }
    
    // Paint focus indicator
    if (hasFocus()) {
        QStyleOptionFocusRect option;
        option.initFrom(this);
        option.rect = rect.adjusted(1, 1, -1, -1);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter, this);
    }
}

void RibbonButton::paintIcon(QPainter* painter, const QRect& iconRect) {
    if (d->icon.isNull() || iconRect.isEmpty()) {
        return;
    }
    
    QIcon::Mode mode = QIcon::Normal;
    if (!isEnabled()) {
        mode = QIcon::Disabled;
    } else if (controlState() == RibbonButtonState::Pressed) {
        mode = QIcon::Selected;
    }
    
    QIcon::State state = d->checked ? QIcon::On : QIcon::Off;
    QPixmap pixmap = d->icon.pixmap(iconRect.size(), mode, state);
    
    painter->drawPixmap(iconRect, pixmap);
}

void RibbonButton::paintText(QPainter* painter, const QRect& textRect) {
    if (d->text.isEmpty() || textRect.isEmpty()) {
        return;
    }
    
    QPen textPen;
    if (isEnabled()) {
        textPen.setColor(RibbonThemeManager::instance()->textColor());
    } else {
        textPen.setColor(RibbonThemeManager::instance()->disabledTextColor());
    }
    
    painter->setPen(textPen);
    painter->setFont(font());
    
    int flags = Qt::AlignCenter;
    if (d->buttonSize != RibbonButtonSize::Small) {
        flags |= Qt::TextWordWrap;
    }
    
    painter->drawText(textRect, flags, d->text);
}

QRect RibbonButton::calculateIconRect() const {
    if (d->icon.isNull()) {
        return QRect();
    }
    
    QSize iconSize;
    switch (d->buttonSize) {
        case RibbonButtonSize::Small:
            iconSize = QSize(16, 16);
            break;
        case RibbonButtonSize::Medium:
            iconSize = QSize(24, 24);
            break;
        case RibbonButtonSize::Large:
            iconSize = QSize(32, 32);
            break;
    }
    
    QRect rect = this->rect();
    QRect iconRect;
    
    if (d->buttonSize == RibbonButtonSize::Small) {
        // Icon on the left
        iconRect = QRect(6, (rect.height() - iconSize.height()) / 2, 
                        iconSize.width(), iconSize.height());
    } else {
        // Icon at the top center
        iconRect = QRect((rect.width() - iconSize.width()) / 2, 6,
                        iconSize.width(), iconSize.height());
    }
    
    return iconRect;
}

QRect RibbonButton::calculateTextRect() const {
    if (d->text.isEmpty()) {
        return QRect();
    }
    
    QRect rect = this->rect();
    QRect textRect;
    
    if (d->buttonSize == RibbonButtonSize::Small) {
        // Text to the right of icon
        int iconWidth = d->icon.isNull() ? 0 : 22;
        textRect = QRect(iconWidth, 0, rect.width() - iconWidth - 6, rect.height());
    } else {
        // Text below icon
        int iconHeight = d->icon.isNull() ? 0 : 38;
        textRect = QRect(6, iconHeight, rect.width() - 12, rect.height() - iconHeight - 6);
    }
    
    return textRect;
}
