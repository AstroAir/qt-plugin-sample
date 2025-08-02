import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Rectangle {
    id: card
    
    // Custom properties
    property bool darkMode: false
    property bool hoverEnabled: true
    property real animationDuration: 200
    property real hoverScale: 1.02
    property real shadowRadius: 8
    property real shadowOffset: 2
    property alias contentItem: contentLoader.sourceComponent
    
    // Card styling
    color: darkMode ? "#2d2d2d" : "#ffffff"
    radius: 12
    
    // Drop shadow effect
    layer.enabled: true
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: shadowOffset
        radius: shadowRadius
        samples: 16
        color: darkMode ? "#000000" : "#40000000"
        transparentBorder: true
    }
    
    // Hover animation
    scale: mouseArea.containsMouse && hoverEnabled ? hoverScale : 1.0
    
    Behavior on scale {
        PropertyAnimation {
            duration: animationDuration
            easing.type: Easing.OutCubic
        }
    }
    
    // Enhanced shadow on hover
    Behavior on layer.effect.radius {
        PropertyAnimation {
            duration: animationDuration
            easing.type: Easing.OutCubic
        }
    }
    
    // Mouse interaction
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: card.hoverEnabled
        
        onEntered: {
            if (hoverEnabled) {
                card.layer.effect.radius = shadowRadius * 1.5
                card.layer.effect.verticalOffset = shadowOffset * 1.5
            }
        }
        
        onExited: {
            if (hoverEnabled) {
                card.layer.effect.radius = shadowRadius
                card.layer.effect.verticalOffset = shadowOffset
            }
        }
        
        onClicked: card.clicked()
    }
    
    // Content loader for flexible content
    Loader {
        id: contentLoader
        anchors.fill: parent
        anchors.margins: 16
    }
    
    // Signals
    signal clicked()
    signal entered()
    signal exited()
    
    // Connect mouse area signals
    Component.onCompleted: {
        mouseArea.entered.connect(card.entered)
        mouseArea.exited.connect(card.exited)
        mouseArea.clicked.connect(card.clicked)
    }
    
    // Subtle border for light theme
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: "transparent"
        border.color: darkMode ? "transparent" : "#e0e0e0"
        border.width: darkMode ? 0 : 1
        opacity: 0.5
    }
    
    // Loading state overlay
    Rectangle {
        id: loadingOverlay
        anchors.fill: parent
        radius: parent.radius
        color: card.color
        opacity: 0
        visible: opacity > 0
        
        property bool loading: false
        
        Behavior on opacity {
            PropertyAnimation {
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
        
        onLoadingChanged: {
            opacity = loading ? 0.8 : 0
        }
        
        // Loading spinner
        BusyIndicator {
            anchors.centerIn: parent
            running: loadingOverlay.loading
            Material.accent: darkMode ? "#ffffff" : "#2196f3"
        }
    }
    
    // Public methods
    function showLoading() {
        loadingOverlay.loading = true
    }
    
    function hideLoading() {
        loadingOverlay.loading = false
    }
}
