// RibbonThemes.h - Modern Ribbon Themes and Styling System
#pragma once

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QGradient>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QConicalGradient>
#include <QPalette>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <memory>
#include "ui/RibbonInterface.h"

// Forward declarations
class RibbonColorScheme;
class RibbonStyleSheet;
class RibbonAnimationManager;

// Note: RibbonTheme enum is defined in ui/RibbonInterface.h

// Color roles for ribbon theming
enum class RibbonColorRole {
    Background,
    Foreground,
    Accent,
    Hover,
    Pressed,
    Border,
    Text,
    DisabledText,
    Selection,
    Highlight,
    Shadow,
    TabBackground,
    TabBorder,
    GroupBackground,
    GroupBorder,
    ButtonBackground,
    ButtonBorder,
    MenuBackground,
    MenuBorder,
    ToolTipBackground,
    ToolTipText
};

// Animation settings
struct RibbonAnimationSettings {
    bool enabled = true;
    int duration = 250;
    QEasingCurve::Type easingCurve = QEasingCurve::OutCubic;
    bool fadeEnabled = true;
    bool slideEnabled = true;
    bool scaleEnabled = false;
    bool glowEnabled = true;
    
    RibbonAnimationSettings() = default;
};

// Font settings
struct RibbonFontSettings {
    QFont defaultFont;
    QFont titleFont;
    QFont smallFont;
    QFont largeFont;
    int defaultSize = 9;
    int titleSize = 11;
    int smallSize = 8;
    int largeSize = 12;
    QString fontFamily = "Segoe UI";
    
    RibbonFontSettings() {
        updateFonts();
    }
    
    void updateFonts() {
        defaultFont = QFont(fontFamily, defaultSize);
        titleFont = QFont(fontFamily, titleSize, QFont::Bold);
        smallFont = QFont(fontFamily, smallSize);
        largeFont = QFont(fontFamily, largeSize);
    }
};

// Metrics and spacing
struct RibbonMetrics {
    int defaultSpacing = 6;
    int defaultMargin = 4;
    int buttonHeight = 22;
    int largeButtonHeight = 66;
    int groupTitleHeight = 18;
    int tabHeight = 100;
    int ribbonHeight = 120;
    int borderWidth = 1;
    int cornerRadius = 3;
    int shadowBlur = 4;
    int shadowOffset = 1;
    
    RibbonMetrics() = default;
};

// Color scheme for ribbon themes
class RibbonColorScheme : public QObject {
    Q_OBJECT

public:
    explicit RibbonColorScheme(QObject* parent = nullptr);
    explicit RibbonColorScheme(RibbonTheme theme, QObject* parent = nullptr);
    ~RibbonColorScheme() override;

    // Theme management
    RibbonTheme theme() const;
    void setTheme(RibbonTheme theme);
    
    // Color access
    QColor color(RibbonColorRole role) const;
    void setColor(RibbonColorRole role, const QColor& color);
    
    // Predefined colors
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor accentColor() const;
    QColor hoverColor() const;
    QColor pressedColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    QColor disabledTextColor() const;
    
    // Gradients
    QLinearGradient backgroundGradient(const QRect& rect) const;
    QLinearGradient buttonGradient(const QRect& rect, bool pressed = false) const;
    QLinearGradient tabGradient(const QRect& rect, bool active = false) const;
    
    // Brushes and pens
    QBrush backgroundBrush() const;
    QBrush foregroundBrush() const;
    QPen borderPen() const;
    QPen textPen() const;
    
    // Serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    
    // Presets
    void loadLightTheme();
    void loadDarkTheme();
    void loadBlueTheme();
    void loadSilverTheme();
    void loadBlackTheme();
    void loadHighContrastTheme();

signals:
    void colorChanged(RibbonColorRole role, const QColor& color);
    void themeChanged(RibbonTheme theme);

private:
    RibbonTheme m_theme;
    QMap<RibbonColorRole, QColor> m_colors;
    
    void initializeColors();
    void loadThemeColors(RibbonTheme theme);
};

// Style sheet generator for ribbon components
class RibbonStyleSheet : public QObject {
    Q_OBJECT

public:
    explicit RibbonStyleSheet(QObject* parent = nullptr);
    ~RibbonStyleSheet() override;

    // Style sheet generation
    QString ribbonBarStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    QString ribbonTabStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    QString ribbonGroupStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    QString ribbonButtonStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    QString ribbonControlStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    QString ribbonMenuStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    QString ribbonToolTipStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics) const;
    
    // Complete style sheet
    QString completeStyleSheet(const RibbonColorScheme* colorScheme, const RibbonMetrics& metrics, const RibbonFontSettings& fonts) const;
    
    // Custom style sheets
    void addCustomStyleSheet(const QString& selector, const QString& styleSheet);
    void removeCustomStyleSheet(const QString& selector);
    QString customStyleSheet(const QString& selector) const;
    void clearCustomStyleSheets();

private:
    QMap<QString, QString> m_customStyleSheets;
    
    QString colorToString(const QColor& color) const;
    QString gradientToString(const QLinearGradient& gradient) const;
    QString fontToString(const QFont& font) const;
};

// Animation manager for ribbon effects
class RibbonAnimationManager : public QObject {
    Q_OBJECT

public:
    explicit RibbonAnimationManager(QObject* parent = nullptr);
    ~RibbonAnimationManager() override;

    // Animation settings
    RibbonAnimationSettings settings() const;
    void setSettings(const RibbonAnimationSettings& settings);
    
    // Animation control
    void setAnimationsEnabled(bool enabled);
    bool animationsEnabled() const;
    void setAnimationDuration(int duration);
    int animationDuration() const;
    
    // Effect animations
    void animateFadeIn(QWidget* widget, int duration = -1);
    void animateFadeOut(QWidget* widget, int duration = -1);
    void animateSlideIn(QWidget* widget, Qt::Orientation direction, int duration = -1);
    void animateSlideOut(QWidget* widget, Qt::Orientation direction, int duration = -1);
    void animateScale(QWidget* widget, qreal scale, int duration = -1);
    void animateGlow(QWidget* widget, const QColor& color, int duration = -1);
    
    // State animations
    void animateHover(QWidget* widget);
    void animatePress(QWidget* widget);
    void animateRelease(QWidget* widget);
    void animateCheck(QWidget* widget, bool checked);
    
    // Group animations
    void animateGroupExpand(QWidget* group, int duration = -1);
    void animateGroupCollapse(QWidget* group, int duration = -1);
    void animateTabSwitch(QWidget* fromTab, QWidget* toTab, int duration = -1);

signals:
    void animationStarted(QWidget* widget, const QString& animationType);
    void animationFinished(QWidget* widget, const QString& animationType);

private slots:
    void onAnimationFinished();

private:
    RibbonAnimationSettings m_settings;
    QMap<QWidget*, QPropertyAnimation*> m_activeAnimations;
    
    QPropertyAnimation* createAnimation(QWidget* widget, const QByteArray& property, const QVariant& startValue, const QVariant& endValue, int duration);
    void cleanupAnimation(QWidget* widget);
};

// Note: RibbonThemeManager class is defined in ui/RibbonInterface.h

// Theme configuration dialog
class RibbonThemeDialog : public QDialog {
    Q_OBJECT

public:
    explicit RibbonThemeDialog(QWidget* parent = nullptr);
    ~RibbonThemeDialog();

    // Theme selection
    RibbonTheme selectedTheme() const;
    void setSelectedTheme(RibbonTheme theme);
    
    // Custom theme editing
    void setCustomTheme(const QJsonObject& themeData);
    QJsonObject customTheme() const;

public slots:
    void accept();
    void reject();
    void resetToDefaults();
    void importTheme();
    void exportTheme();

signals:
    void themeChanged(RibbonTheme theme);
    void customThemeChanged(const QJsonObject& themeData);

private slots:
    void onThemeSelectionChanged();
    void onColorChanged();
    void onFontChanged();
    void onMetricChanged();
    void onAnimationChanged();
    void onPreviewRequested();

private:
    struct ThemeDialogPrivate;
    std::unique_ptr<ThemeDialogPrivate> d;
    
    void setupUI();
    void setupThemeSelection();
    void setupColorCustomization();
    void setupFontCustomization();
    void setupMetricsCustomization();
    void setupAnimationSettings();
    void setupPreview();
    void updatePreview();
    void loadThemeSettings();
    void saveThemeSettings();
};

// Utility functions for theme management
namespace RibbonThemeUtils {
    // Color utilities
    QColor lighten(const QColor& color, int factor = 150);
    QColor darken(const QColor& color, int factor = 150);
    QColor blend(const QColor& color1, const QColor& color2, qreal ratio = 0.5);
    QColor adjustAlpha(const QColor& color, int alpha);
    
    // Gradient utilities
    QLinearGradient createGradient(const QColor& startColor, const QColor& endColor, Qt::Orientation orientation = Qt::Vertical);
    QRadialGradient createRadialGradient(const QColor& centerColor, const QColor& edgeColor, const QPointF& center, qreal radius);
    
    // Font utilities
    QFont scaleFont(const QFont& font, qreal scale);
    QFont adjustFontWeight(const QFont& font, QFont::Weight weight);
    
    // System integration
    bool isSystemDarkMode();
    QColor getSystemAccentColor();
    QString getSystemFontFamily();
    
    // Theme validation
    bool validateThemeData(const QJsonObject& themeData);
    QStringList getThemeValidationErrors(const QJsonObject& themeData);
    
    // Theme conversion
    QJsonObject themeToJson(RibbonTheme theme);
    RibbonTheme themeFromString(const QString& themeName);
    QString themeToString(RibbonTheme theme);
}
