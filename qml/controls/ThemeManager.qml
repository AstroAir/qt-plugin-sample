pragma Singleton
import QtQuick 2.15
import QtQuick.Controls.Material 2.15

QtObject {
    id: theme
    
    // Theme state
    property bool darkMode: false
    
    // Enhanced Color Palette
    readonly property QtObject colors: QtObject {
        // Primary colors
        property color primary: darkMode ? "#2196f3" : "#1976d2"
        property color primaryVariant: darkMode ? "#1976d2" : "#0d47a1"
        property color secondary: darkMode ? "#03dac6" : "#018786"
        
        // Surface colors
        property color background: darkMode ? "#121212" : "#fafafa"
        property color backgroundGradientStart: darkMode ? "#1e1e1e" : "#fafafa"
        property color backgroundGradientEnd: darkMode ? "#121212" : "#f5f5f5"
        property color surface: darkMode ? "#2d2d2d" : "#ffffff"
        property color surfaceVariant: darkMode ? "#404040" : "#f5f5f5"
        
        // Text colors
        property color onSurface: darkMode ? "#ffffff" : "#212121"
        property color onSurfaceVariant: darkMode ? "#b0b0b0" : "#757575"
        property color onSurfaceSecondary: darkMode ? "#808080" : "#999999"
        
        // Status colors
        property color success: "#4caf50"
        property color warning: "#ff9800"
        property color error: "#f44336"
        property color info: "#2196f3"
        
        // Interactive colors
        property color accent: Material.accent
        property color hover: darkMode ? "#404040" : "#f0f0f0"
        property color pressed: darkMode ? "#505050" : "#e0e0e0"
        property color focus: Material.accent
        
        // Border colors
        property color border: darkMode ? "#404040" : "#e0e0e0"
        property color borderFocus: Material.accent
        property color divider: darkMode ? "#404040" : "#e0e0e0"
        
        // Shadow colors
        property color shadow: darkMode ? "#000000" : "#40000000"
        property color shadowLight: darkMode ? "#000000" : "#20000000"
    }
    
    // Typography System
    readonly property QtObject typography: QtObject {
        // Font families
        property string primaryFont: "Segoe UI"
        property string monoFont: "Consolas"
        
        // Font sizes
        property int displayLarge: 32
        property int displayMedium: 28
        property int displaySmall: 24
        property int headlineLarge: 22
        property int headlineMedium: 20
        property int headlineSmall: 18
        property int titleLarge: 16
        property int titleMedium: 14
        property int titleSmall: 12
        property int bodyLarge: 14
        property int bodyMedium: 12
        property int bodySmall: 11
        property int labelLarge: 12
        property int labelMedium: 11
        property int labelSmall: 10
        
        // Font weights
        property int light: Font.Light
        property int normal: Font.Normal
        property int medium: Font.Medium
        property int bold: Font.Bold
        
        // Line heights
        property real lineHeightTight: 1.2
        property real lineHeightNormal: 1.4
        property real lineHeightLoose: 1.6
    }
    
    // Spacing System
    readonly property QtObject spacing: QtObject {
        property int xs: 4
        property int sm: 8
        property int md: 12
        property int lg: 16
        property int xl: 20
        property int xxl: 24
        property int xxxl: 32
        
        // Component-specific spacing
        property int cardPadding: lg
        property int buttonPadding: md
        property int listItemSpacing: md
        property int sectionSpacing: xl
    }
    
    // Border Radius System
    readonly property QtObject radius: QtObject {
        property int none: 0
        property int sm: 4
        property int md: 8
        property int lg: 12
        property int xl: 16
        property int full: 999
        
        // Component-specific radius
        property int card: lg
        property int button: md
        property int input: md
        property int badge: full
    }
    
    // Shadow System
    readonly property QtObject shadows: QtObject {
        property int none: 0
        property int sm: 2
        property int md: 4
        property int lg: 8
        property int xl: 12
        
        // Shadow offsets
        property int offsetSm: 1
        property int offsetMd: 2
        property int offsetLg: 4
        
        // Shadow samples
        property int samplesSm: 8
        property int samplesMd: 12
        property int samplesLg: 16
    }
    
    // Animation System
    readonly property QtObject animations: QtObject {
        property int fast: 150
        property int normal: 200
        property int slow: 300
        property int slower: 500
        
        // Easing curves
        property int easeOut: Easing.OutCubic
        property int easeIn: Easing.InCubic
        property int easeInOut: Easing.InOutCubic
        property int easeOutBack: Easing.OutBack
        
        // Scale factors
        property real scaleHover: 1.02
        property real scalePress: 0.98
        property real scaleActive: 1.05
    }
    
    // Component Styles
    readonly property QtObject components: QtObject {
        // Card styles
        readonly property QtObject card: QtObject {
            property color background: theme.colors.surface
            property color border: theme.colors.border
            property int borderWidth: darkMode ? 0 : 1
            property int radius: theme.radius.card
            property int shadowRadius: theme.shadows.md
            property int shadowOffset: theme.shadows.offsetMd
            property color shadowColor: theme.colors.shadow
        }
        
        // Button styles
        readonly property QtObject button: QtObject {
            property int radius: theme.radius.button
            property int paddingHorizontal: theme.spacing.lg
            property int paddingVertical: theme.spacing.sm
            property int fontSize: theme.typography.bodyLarge
            property int fontWeight: theme.typography.medium
        }
        
        // Input styles
        readonly property QtObject input: QtObject {
            property color background: theme.colors.surfaceVariant
            property color border: theme.colors.border
            property color borderFocus: theme.colors.borderFocus
            property int borderWidth: 2
            property int radius: theme.radius.input
            property int padding: theme.spacing.md
        }
    }
    
    // Theme switching
    function toggleTheme() {
        darkMode = !darkMode
    }
    
    function setDarkMode(enabled) {
        darkMode = enabled
    }
    
    // Utility functions
    function getStatusColor(status) {
        switch(status) {
            case "success":
            case "active":
            case "enabled":
                return colors.success
            case "warning":
                return colors.warning
            case "error":
            case "danger":
                return colors.error
            case "info":
            case "loading":
                return colors.info
            default:
                return colors.onSurfaceVariant
        }
    }
    
    function getContrastColor(backgroundColor) {
        // Simple contrast calculation
        var r = parseInt(backgroundColor.toString().substr(1, 2), 16)
        var g = parseInt(backgroundColor.toString().substr(3, 2), 16)
        var b = parseInt(backgroundColor.toString().substr(5, 2), 16)
        var brightness = (r * 299 + g * 587 + b * 114) / 1000
        return brightness > 128 ? "#000000" : "#ffffff"
    }
}
