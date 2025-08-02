import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

Button {
    id: control
    
    // Custom properties
    property string buttonType: "primary" // primary, secondary, accent, danger
    property bool animated: true
    property real animationDuration: 200
    property bool glowEffect: false
    
    // Enhanced styling based on type
    Material.background: {
        switch(buttonType) {
            case "primary": return Material.accent
            case "secondary": return "transparent"
            case "accent": return Material.accent
            case "danger": return "#f44336"
            default: return Material.accent
        }
    }
    
    Material.foreground: {
        switch(buttonType) {
            case "primary": return "white"
            case "secondary": return Material.accent
            case "accent": return "white"
            case "danger": return "white"
            default: return "white"
        }
    }
    
    flat: buttonType === "secondary"
    font.weight: Font.Medium
    
    // Enhanced hover and press effects
    scale: mouseArea.pressed ? 0.98 : (mouseArea.containsMouse ? 1.02 : 1.0)
    
    Behavior on scale {
        enabled: animated
        PropertyAnimation {
            duration: animationDuration
            easing.type: Easing.OutCubic
        }
    }
    
    // Custom mouse area for better control
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: control.clicked()
        
        // Ripple effect on click
        Rectangle {
            id: ripple
            anchors.centerIn: parent
            width: 0
            height: 0
            radius: width / 2
            color: control.Material.foreground
            opacity: 0.2
            visible: false
            
            SequentialAnimation {
                id: rippleAnimation
                PropertyAnimation {
                    target: ripple
                    properties: "width,height"
                    from: 0
                    to: Math.max(control.width, control.height) * 2
                    duration: 300
                    easing.type: Easing.OutCubic
                }
                PropertyAnimation {
                    target: ripple
                    property: "opacity"
                    from: 0.2
                    to: 0
                    duration: 200
                }
                onStarted: ripple.visible = true
                onFinished: ripple.visible = false
            }
        }
        
        onPressed: {
            if (animated) {
                rippleAnimation.start()
            }
        }
    }
    
    // Glow effect for special buttons
    Rectangle {
        anchors.fill: parent
        radius: control.radius
        color: "transparent"
        border.color: control.Material.accent
        border.width: glowEffect ? 2 : 0
        opacity: glowEffect ? 0.6 : 0
        
        Behavior on opacity {
            PropertyAnimation {
                duration: animationDuration
                easing.type: Easing.OutCubic
            }
        }
    }
}
