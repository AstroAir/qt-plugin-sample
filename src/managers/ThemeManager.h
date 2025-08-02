// ThemeManager.h - Enhanced theme management system
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QColor>
#include <QFont>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QPixmap>
#include <QGradient>
#include <QPropertyAnimation>
#include <QGraphicsEffect>
#include <QSystemTrayIcon>
#include <memory>

class ThemePreview;
class ColorScheme;
class ThemeEditor;

class ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(bool darkModeEnabled READ isDarkModeEnabled WRITE setDarkModeEnabled NOTIFY darkModeChanged)
    Q_PROPERTY(QStringList availableThemes READ availableThemes NOTIFY availableThemesChanged)
    Q_PROPERTY(bool animationsEnabled READ animationsEnabled WRITE setAnimationsEnabled NOTIFY animationsEnabledChanged)
    Q_PROPERTY(double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)

public:
    enum ThemeType {
        Light,
        Dark,
        Auto,
        Custom
    };

    enum ColorRole {
        Primary,
        Secondary,
        Accent,
        Background,
        Surface,
        Error,
        Warning,
        Success,
        Info,
        TextPrimary,
        TextSecondary,
        TextDisabled,
        Border,
        Divider,
        Shadow,
        Highlight
    };

    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager() override;

    // Theme management
    QString currentTheme() const;
    void setCurrentTheme(const QString& themeName);
    QStringList availableThemes() const;
    ThemeType currentThemeType() const;

    // Dark mode
    bool isDarkModeEnabled() const;
    void setDarkModeEnabled(bool enabled);
    void toggleDarkMode();

    // Animations
    bool animationsEnabled() const;
    void setAnimationsEnabled(bool enabled);

    // Opacity
    double opacity() const;
    void setOpacity(double opacity);

    // Accent color
    QString accentColor() const;
    void setAccentColor(const QString& color);

    // Theme operations
    bool loadTheme(const QString& themeName);
    bool saveTheme(const QString& themeName, const QJsonObject& themeData);
    bool deleteTheme(const QString& themeName);
    bool exportTheme(const QString& themeName, const QString& filePath);
    bool importTheme(const QString& filePath);
    bool duplicateTheme(const QString& sourceName, const QString& newName);

    // Theme properties
    QColor getColor(ColorRole role) const;
    QColor getColor(const QString& colorName) const;
    QFont getFont(const QString& fontName) const;
    QString getStyleSheet() const;
    QString getStyleSheet(const QString& component) const;
    QJsonObject getCurrentThemeData() const;
    QPixmap getIcon(const QString& iconName) const;
    QGradient getGradient(const QString& gradientName) const;

    // Color schemes
    QStringList availableColorSchemes() const;
    void applyColorScheme(const QString& schemeName);
    ColorScheme* createColorScheme(const QString& name);

    // Theme validation
    bool isValidTheme(const QJsonObject& themeData) const;
    QStringList validateTheme(const QJsonObject& themeData) const;

    // Preview
    ThemePreview* createPreview(const QString& themeName) const;
    QPixmap generateThemePreview(const QString& themeName, const QSize& size = QSize(200, 150)) const;

    // Legacy compatibility
    Q_INVOKABLE void applyTheme(const QString& theme);
    Q_INVOKABLE QString getThemeStyleSheet(const QString& theme) const;

signals:
    void currentThemeChanged(const QString& themeName);
    void darkModeChanged(bool enabled);
    void availableThemesChanged();
    void themeLoaded(const QString& themeName);
    void themeError(const QString& error);
    void animationsEnabledChanged(bool enabled);
    void opacityChanged(double opacity);
    void accentColorChanged(const QString& color);
    void colorSchemeChanged(const QString& scheme);
    void themeApplied(const QString& theme); // Legacy compatibility

public slots:
    void refreshThemes();
    void resetToDefault();
    void applySystemTheme();
    void createCustomTheme();
    void editTheme(const QString& themeName);
    void previewTheme(const QString& themeName);

private slots:
    void onThemeFileChanged(const QString& path);
    void onSystemThemeChanged();
    void onAnimationFinished();

private:
    void initializeThemes();
    void loadBuiltInThemes();
    void loadAvailableThemes(); // Legacy compatibility
    QString loadStyleSheet(const QString& theme) const; // Legacy compatibility
    void scanThemeDirectory();
    void applyTheme(const QJsonObject& themeData);
    void updateApplicationPalette();
    void updateApplicationStyleSheet();
    void updateSystemTrayIcon();
    QString generateStyleSheet(const QJsonObject& themeData);
    QString generateComponentStyleSheet(const QString& component, const QJsonObject& themeData);
    QJsonObject createDefaultTheme();
    QJsonObject createDarkTheme();
    QJsonObject createMaterialTheme();
    QJsonObject createFlatTheme();
    QJsonObject createGlassTheme();
    void setupAnimations();
    void animateThemeTransition();

    QString m_currentTheme;
    QStringList m_availableThemes; // Legacy compatibility
    ThemeType m_currentThemeType;
    bool m_darkModeEnabled;
    bool m_animationsEnabled;
    double m_opacity;
    QString m_accentColor;
    QHash<QString, QJsonObject> m_themes;
    QHash<QString, ColorScheme*> m_colorSchemes;
    QJsonObject m_currentThemeData;
    QString m_themesDirectory;
    std::unique_ptr<QFileSystemWatcher> m_fileWatcher;
    std::unique_ptr<QSettings> m_settings;

    QPropertyAnimation* m_transitionAnimation;
};
