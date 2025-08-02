// RibbonCustomization.h - Ribbon Customization and Layout Management
#pragma once

#include "RibbonInterface.h"
#include <QDialog>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QListWidget>
#include <QTableWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QScrollArea>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <QMap>
#include <QList>
#include <memory>

// Forward declarations
class RibbonCustomizationDialog;
class RibbonLayoutEditor;
class RibbonCommandPalette;
class RibbonPreviewWidget;
class CustomizableRibbonTab;
class CustomizableRibbonGroup;

// Customization item types
enum class RibbonCustomizationType {
    Tab,
    Group,
    Control,
    Separator,
    Spacer
};

// Customization actions
enum class RibbonCustomizationAction {
    Add,
    Remove,
    Move,
    Rename,
    Configure,
    Reset
};

// Customization item data
struct RibbonCustomizationItem {
    QString id;
    QString name;
    QString description;
    QString icon;
    RibbonCustomizationType type;
    RibbonControlType controlType = RibbonControlType::Button;
    QJsonObject properties;
    QStringList children;
    bool visible = true;
    bool enabled = true;
    bool removable = true;
    bool renamable = true;
    
    RibbonCustomizationItem() = default;
    RibbonCustomizationItem(const QString& i, const QString& n, RibbonCustomizationType t)
        : id(i), name(n), type(t) {}
};

// Main ribbon customization dialog
class RibbonCustomizationDialog : public QDialog {
    Q_OBJECT

public:
    explicit RibbonCustomizationDialog(RibbonBar* ribbonBar, QWidget* parent = nullptr);
    ~RibbonCustomizationDialog() override;

    // Customization management
    void setRibbonBar(RibbonBar* ribbonBar);
    RibbonBar* ribbonBar() const;
    
    // Layout management
    QJsonObject exportLayout() const;
    void importLayout(const QJsonObject& layout);
    void resetToDefaults();
    
    // Preset management
    void savePreset(const QString& name);
    void loadPreset(const QString& name);
    void deletePreset(const QString& name);
    QStringList availablePresets() const;

public slots:
    void accept() override;
    void reject() override;
    void applyChanges();
    void previewChanges();
    void resetChanges();

signals:
    void layoutChanged(const QJsonObject& layout);
    void presetSaved(const QString& name);
    void presetLoaded(const QString& name);

private slots:
    void onTabSelectionChanged();
    void onGroupSelectionChanged();
    void onControlSelectionChanged();
    void onAddItemRequested();
    void onRemoveItemRequested();
    void onMoveItemRequested();
    void onRenameItemRequested();
    void onConfigureItemRequested();
    void onPreviewRequested();

private:
    struct CustomizationDialogPrivate;
    std::unique_ptr<CustomizationDialogPrivate> d;
    
    void setupUI();
    void setupTabsPage();
    void setupGroupsPage();
    void setupControlsPage();
    void setupPresetsPage();
    void setupPreviewPage();
    void updatePreview();
    void populateAvailableItems();
    void populateCurrentLayout();
    void validateLayout();
    void applyLayoutToRibbon();
};

// Ribbon layout editor widget
class RibbonLayoutEditor : public QWidget {
    Q_OBJECT

public:
    explicit RibbonLayoutEditor(QWidget* parent = nullptr);
    ~RibbonLayoutEditor() override;

    // Layout management
    void setLayout(const QJsonObject& layout);
    QJsonObject getLayout() const;
    void clearLayout();
    
    // Item management
    void addItem(const RibbonCustomizationItem& item, const QString& parentId = "");
    void removeItem(const QString& itemId);
    void moveItem(const QString& itemId, const QString& newParentId, int position = -1);
    void renameItem(const QString& itemId, const QString& newName);
    void configureItem(const QString& itemId, const QJsonObject& properties);
    
    // Selection
    QString selectedItemId() const;
    void setSelectedItem(const QString& itemId);
    RibbonCustomizationItem selectedItem() const;
    
    // Validation
    bool validateLayout() const;
    QStringList getValidationErrors() const;

signals:
    void itemSelected(const QString& itemId);
    void itemDoubleClicked(const QString& itemId);
    void itemMoved(const QString& itemId, const QString& oldParentId, const QString& newParentId);
    void layoutChanged();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    struct LayoutEditorPrivate;
    std::unique_ptr<LayoutEditorPrivate> d;
    
    void setupUI();
    void populateTree();
    void updateTreeItem(const QString& itemId);
    QTreeWidgetItem* findTreeItem(const QString& itemId) const;
    QString getItemIdFromTreeItem(QTreeWidgetItem* item) const;
    bool canDropItem(const QString& itemId, const QString& targetId) const;
};

// Command palette for adding ribbon items
class RibbonCommandPalette : public QWidget {
    Q_OBJECT

public:
    explicit RibbonCommandPalette(QWidget* parent = nullptr);
    ~RibbonCommandPalette() override;

    // Command management
    void addCommand(const RibbonCustomizationItem& item);
    void removeCommand(const QString& commandId);
    void clearCommands();
    
    // Categories
    void addCategory(const QString& name, const QString& description = "");
    void removeCategory(const QString& name);
    void setCategoryCommands(const QString& category, const QList<RibbonCustomizationItem>& commands);
    
    // Search and filtering
    void setSearchText(const QString& text);
    QString searchText() const;
    void setFilterCategory(const QString& category);
    QString filterCategory() const;
    
    // Selection
    RibbonCustomizationItem selectedCommand() const;
    void setSelectedCommand(const QString& commandId);

signals:
    void commandSelected(const RibbonCustomizationItem& item);
    void commandDoubleClicked(const RibbonCustomizationItem& item);
    void commandDragStarted(const RibbonCustomizationItem& item);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void onSearchTextChanged();
    void onCategoryChanged();
    void onItemSelectionChanged();

private:
    struct CommandPalettePrivate;
    std::unique_ptr<CommandPalettePrivate> d;
    
    void setupUI();
    void populateCommands();
    void filterCommands();
    void updateCommandList();
    QListWidgetItem* createCommandItem(const RibbonCustomizationItem& item);
};

// Preview widget for ribbon customization
class RibbonPreviewWidget : public QWidget {
    Q_OBJECT

public:
    explicit RibbonPreviewWidget(QWidget* parent = nullptr);
    ~RibbonPreviewWidget() override;

    // Preview management
    void setPreviewLayout(const QJsonObject& layout);
    void clearPreview();
    void refreshPreview();
    
    // Theme preview
    void setPreviewTheme(RibbonTheme theme);
    RibbonTheme previewTheme() const;
    
    // Scale preview
    void setPreviewScale(qreal scale);
    qreal previewScale() const;

signals:
    void previewClicked(const QString& itemId);
    void previewDoubleClicked(const QString& itemId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct PreviewWidgetPrivate;
    std::unique_ptr<PreviewWidgetPrivate> d;
    
    void setupUI();
    void createPreviewRibbon();
    void updatePreviewRibbon();
    void paintPreviewBackground(QPainter* painter);
    void paintPreviewRibbon(QPainter* painter);
    QString getItemIdAt(const QPoint& pos) const;
};

// Customizable ribbon tab (for runtime customization)
class CustomizableRibbonTab : public RibbonTab {
    Q_OBJECT

public:
    explicit CustomizableRibbonTab(const QString& title, const QString& id = "", QWidget* parent = nullptr);
    ~CustomizableRibbonTab() override;

    // Customization support
    bool isCustomizable() const;
    void setCustomizable(bool customizable);
    
    // Runtime modification
    void addCustomGroup(const QString& title, const QString& id = "");
    void removeCustomGroup(const QString& id);
    void moveCustomGroup(const QString& id, int newPosition);
    void renameCustomGroup(const QString& id, const QString& newTitle);
    
    // Serialization
    QJsonObject exportCustomization() const;
    void importCustomization(const QJsonObject& customization);

signals:
    void customizationChanged();
    void groupCustomized(const QString& groupId, const QString& action);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onCustomizeRequested();
    void onAddGroupRequested();
    void onRemoveGroupRequested();
    void onRenameGroupRequested();

private:
    bool m_customizable;
    QJsonObject m_customization;
    
    void setupCustomizationMenu();
    void showGroupCustomizationDialog(const QString& groupId);
};

// Customizable ribbon group (for runtime customization)
class CustomizableRibbonGroup : public RibbonGroup {
    Q_OBJECT

public:
    explicit CustomizableRibbonGroup(const QString& title, const QString& id = "", QWidget* parent = nullptr);
    ~CustomizableRibbonGroup() override;

    // Customization support
    bool isCustomizable() const;
    void setCustomizable(bool customizable);
    
    // Runtime modification
    void addCustomControl(const RibbonControlConfig& config);
    void removeCustomControl(const QString& id);
    void moveCustomControl(const QString& id, int newPosition);
    void configureCustomControl(const QString& id, const RibbonControlConfig& config);
    
    // Serialization
    QJsonObject exportCustomization() const;
    void importCustomization(const QJsonObject& customization);

signals:
    void customizationChanged();
    void controlCustomized(const QString& controlId, const QString& action);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onCustomizeRequested();
    void onAddControlRequested();
    void onRemoveControlRequested();
    void onConfigureControlRequested();

private:
    bool m_customizable;
    QJsonObject m_customization;
    
    void setupCustomizationMenu();
    void showControlCustomizationDialog(const QString& controlId);
};

// Utility functions for ribbon customization
namespace RibbonCustomizationUtils {
    // Layout validation
    bool validateRibbonLayout(const QJsonObject& layout);
    QStringList getRibbonLayoutErrors(const QJsonObject& layout);
    
    // Layout conversion
    QJsonObject ribbonToLayout(const RibbonBar* ribbon);
    void layoutToRibbon(RibbonBar* ribbon, const QJsonObject& layout);
    
    // Item management
    RibbonCustomizationItem createTabItem(const QString& id, const QString& name);
    RibbonCustomizationItem createGroupItem(const QString& id, const QString& name);
    RibbonCustomizationItem createControlItem(const QString& id, const QString& name, RibbonControlType type);
    
    // Default layouts
    QJsonObject getDefaultRibbonLayout();
    QJsonObject getMinimalRibbonLayout();
    QJsonObject getFullRibbonLayout();
    
    // Preset management
    void saveLayoutPreset(const QString& name, const QJsonObject& layout);
    QJsonObject loadLayoutPreset(const QString& name);
    void deleteLayoutPreset(const QString& name);
    QStringList getAvailablePresets();
    
    // Import/Export
    bool exportLayoutToFile(const QJsonObject& layout, const QString& filePath);
    QJsonObject importLayoutFromFile(const QString& filePath);
    
    // Migration
    QJsonObject migrateLayout(const QJsonObject& oldLayout, const QString& fromVersion, const QString& toVersion);
    bool isLayoutCompatible(const QJsonObject& layout, const QString& version);
};

// Advanced ribbon designer for visual customization
class RibbonDesigner : public QWidget {
    Q_OBJECT

public:
    explicit RibbonDesigner(QWidget* parent = nullptr);
    ~RibbonDesigner() override;

    // Design operations
    void setRibbonBar(RibbonBar* ribbonBar);
    RibbonBar* ribbonBar() const;
    void enterDesignMode();
    void exitDesignMode();
    bool isInDesignMode() const;

    // Visual editing
    void selectElement(const QString& elementId);
    QString selectedElement() const;
    void highlightElement(const QString& elementId);
    void clearHighlight();

    // Drag and drop support
    void enableDragDrop(bool enable);
    bool isDragDropEnabled() const;
    void setDropIndicatorVisible(bool visible);
    bool isDropIndicatorVisible() const;

    // Grid and alignment
    void setGridVisible(bool visible);
    bool isGridVisible() const;
    void setGridSize(int size);
    int gridSize() const;
    void setSnapToGrid(bool snap);
    bool snapToGrid() const;

    // Zoom and view
    void setZoomLevel(double zoom);
    double zoomLevel() const;
    void zoomIn();
    void zoomOut();
    void zoomToFit();
    void resetZoom();

signals:
    void designModeEntered();
    void designModeExited();
    void elementSelected(const QString& elementId);
    void elementMoved(const QString& elementId, const QPoint& newPosition);
    void elementResized(const QString& elementId, const QSize& newSize);
    void elementsDropped(const QStringList& elementIds, const QString& targetId);
    void propertyChanged(const QString& elementId, const QString& property, const QVariant& value);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    RibbonBar* m_ribbonBar;
    bool m_designMode;
    bool m_dragDropEnabled;
    bool m_gridVisible;
    bool m_snapToGrid;
    bool m_dropIndicatorVisible;
    int m_gridSize;
    double m_zoomLevel;
    QString m_selectedElement;
    QString m_highlightedElement;
    QPoint m_dragStartPos;
    QPoint m_dropIndicatorPos;

    void drawGrid(QPainter* painter);
    void drawDropIndicator(QPainter* painter);
    void drawElementHighlight(QPainter* painter, const QString& elementId);
    QString elementAtPosition(const QPoint& pos) const;
    QRect getElementRect(const QString& elementId) const;
    QPoint snapToGridPoint(const QPoint& point) const;
};

// Theme customizer for ribbon appearance
class ThemeCustomizer : public QWidget {
    Q_OBJECT

public:
    explicit ThemeCustomizer(QWidget* parent = nullptr);
    ~ThemeCustomizer() override;

    // Theme management
    void setCurrentTheme(const RibbonTheme& theme);
    RibbonTheme getCurrentTheme() const;
    void applyTheme(const RibbonTheme& theme);
    void resetTheme();

    // Color customization
    void setAccentColor(const QColor& color);
    QColor accentColor() const;
    void setBackgroundColor(const QColor& color);
    QColor backgroundColor() const;
    void setTextColor(const QColor& color);
    QColor textColor() const;
    void setHighlightColor(const QColor& color);
    QColor highlightColor() const;

    // Font customization
    void setFont(const QFont& font);
    QFont font() const;
    void setFontSize(int size);
    int fontSize() const;
    void setBold(bool bold);
    bool isBold() const;
    void setItalic(bool italic);
    bool isItalic() const;

    // Style customization
    void setCornerRadius(int radius);
    int cornerRadius() const;
    void setBorderWidth(int width);
    int borderWidth() const;
    void setShadowEnabled(bool enabled);
    bool isShadowEnabled() const;
    void setAnimationsEnabled(bool enabled);
    bool areAnimationsEnabled() const;

    // Icon customization
    void setIconTheme(const QString& theme);
    QString iconTheme() const;
    void setIconSize(const QSize& size);
    QSize iconSize() const;
    void setIconStyle(const QString& style);
    QString iconStyle() const;

signals:
    void themeChanged(const RibbonTheme& theme);
    void colorChanged(const QString& colorRole, const QColor& color);
    void fontChanged(const QFont& font);
    void styleChanged(const QString& property, const QVariant& value);
    void iconThemeChanged(const QString& theme);

private slots:
    void onColorChanged();
    void onFontChanged();
    void onStyleChanged();
    void onIconChanged();

private:
    RibbonTheme m_currentTheme;

    // UI components
    QTabWidget* m_tabWidget;
    QWidget* m_colorsTab;
    QWidget* m_fontsTab;
    QWidget* m_stylesTab;
    QWidget* m_iconsTab;

    // Color controls
    QPushButton* m_accentColorButton;
    QPushButton* m_backgroundColorButton;
    QPushButton* m_textColorButton;
    QPushButton* m_highlightColorButton;

    // Font controls
    QComboBox* m_fontFamilyCombo;
    QSpinBox* m_fontSizeSpinBox;
    QCheckBox* m_boldCheckBox;
    QCheckBox* m_italicCheckBox;

    // Style controls
    QSpinBox* m_cornerRadiusSpinBox;
    QSpinBox* m_borderWidthSpinBox;
    QCheckBox* m_shadowCheckBox;
    QCheckBox* m_animationsCheckBox;

    // Icon controls
    QComboBox* m_iconThemeCombo;
    QComboBox* m_iconSizeCombo;
    QComboBox* m_iconStyleCombo;

    void setupUI();
    void setupColorsTab();
    void setupFontsTab();
    void setupStylesTab();
    void setupIconsTab();
    void updateUIFromTheme();
    void updateThemeFromUI();
    void showColorDialog(QPushButton* button, const QColor& currentColor);
};
