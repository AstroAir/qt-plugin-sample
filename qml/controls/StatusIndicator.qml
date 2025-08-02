import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: indicator
    
    // Custom properties
    property string status: "active" // active, inactive, error, warning, loading
    property bool animated: true
    property real pulseSpeed: 1000
    property string customColor: ""
    property bool showText: false
    property string statusText: ""
    
    // Size properties
    width: 12
    height: 12
    radius: width / 2
    
    // Color based on status
    color: {
        if (customColor !== "") return customColor
        
        switch(status) {
            case "active": return "#4caf50"
            case "inactive": return "#9e9e9e"
            case "error": return "#f44336"
            case "warning": return "#ff9800"
            case "loading": return "#2196f3"
            default: return "#9e9e9e"
        }
    }
    
    // Border for better visibility
    border.color: Qt.darker(color, 1.2)
    border.width: 1
    
    // Pulsing animation for active/loading states
    SequentialAnimation on opacity {
        running: animated && (status === "active" || status === "loading")
        loops: Animation.Infinite
        
        PropertyAnimation {
            to: 0.3
            duration: pulseSpeed
            easing.type: Easing.InOutQuad
        }
        PropertyAnimation {
            to: 1.0
            duration: pulseSpeed
            easing.type: Easing.InOutQuad
        }
    }
    
    // Rotation animation for loading state
    RotationAnimation on rotation {
        running: animated && status === "loading"
        loops: Animation.Infinite
        from: 0
        to: 360
        duration: 2000
        easing.type: Easing.Linear
    }
    
    // Inner circle for loading state
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 0.6
        height: parent.height * 0.6
        radius: width / 2
        color: "white"
        visible: status === "loading"
        opacity: 0.8
    }
    
    // Status text label
    Text {
        id: statusLabel
        anchors.left: parent.right
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        
        text: statusText !== "" ? statusText : status.charAt(0).toUpperCase() + status.slice(1)
        font.pixelSize: 12
        font.weight: Font.Medium
        color: parent.color
        visible: showText
    }
    
    // Tooltip for status information
    ToolTip {
        id: tooltip
        visible: mouseArea.containsMouse
        text: getStatusDescription()
        delay: 500
        
        function getStatusDescription() {
            switch(status) {
                case "active": return "Active and running"
                case "inactive": return "Inactive or disabled"
                case "error": return "Error occurred"
                case "warning": return "Warning condition"
                case "loading": return "Loading or processing"
                default: return "Unknown status"
            }
        }
    }
    
    // Mouse area for tooltip
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }
    
    // Scale animation on hover
    scale: mouseArea.containsMouse ? 1.2 : 1.0
    
    Behavior on scale {
        PropertyAnimation {
            duration: 150
            easing.type: Easing.OutCubic
        }
    }
    
    // Glow effect for error and warning states
    Rectangle {
        anchors.centerIn: parent
        width: parent.width * 2
        height: parent.height * 2
        radius: width / 2
        color: parent.color
        opacity: (status === "error" || status === "warning") ? 0.3 : 0
        visible: opacity > 0
        
        Behavior on opacity {
            PropertyAnimation {
                duration: 300
                easing.type: Easing.OutCubic
            }
        }
    }
}
