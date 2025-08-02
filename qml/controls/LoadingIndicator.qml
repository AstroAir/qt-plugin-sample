import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.15

Item {
    id: loadingIndicator
    
    // Properties
    property bool running: false
    property string type: "spinner" // spinner, dots, pulse, wave
    property color color: "#2196f3"
    property real size: 32
    property int animationDuration: 1000
    property string text: ""
    property bool showText: text !== ""
    
    // Size
    width: size
    height: size + (showText ? textLabel.height + 8 : 0)
    
    // Spinner type loading
    Rectangle {
        id: spinner
        anchors.centerIn: parent
        width: size
        height: size
        radius: size / 2
        color: "transparent"
        border.color: loadingIndicator.color
        border.width: 3
        visible: type === "spinner"
        
        // Gradient for spinner effect
        ConicalGradient {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.8; color: loadingIndicator.color }
                GradientStop { position: 1.0; color: "transparent" }
            }
            
            RotationAnimation on rotation {
                running: loadingIndicator.running && type === "spinner"
                loops: Animation.Infinite
                from: 0
                to: 360
                duration: animationDuration
                easing.type: Easing.Linear
            }
        }
    }
    
    // Dots type loading
    Row {
        id: dotsRow
        anchors.centerIn: parent
        spacing: size / 8
        visible: type === "dots"
        
        Repeater {
            model: 3
            
            Rectangle {
                width: size / 6
                height: size / 6
                radius: width / 2
                color: loadingIndicator.color
                
                SequentialAnimation on opacity {
                    running: loadingIndicator.running && type === "dots"
                    loops: Animation.Infinite
                    
                    PauseAnimation { duration: index * 200 }
                    PropertyAnimation { to: 0.3; duration: 400 }
                    PropertyAnimation { to: 1.0; duration: 400 }
                    PauseAnimation { duration: (2 - index) * 200 }
                }
            }
        }
    }
    
    // Pulse type loading
    Rectangle {
        id: pulseCircle
        anchors.centerIn: parent
        width: size
        height: size
        radius: size / 2
        color: loadingIndicator.color
        visible: type === "pulse"
        
        SequentialAnimation on scale {
            running: loadingIndicator.running && type === "pulse"
            loops: Animation.Infinite
            
            PropertyAnimation {
                to: 1.2
                duration: animationDuration / 2
                easing.type: Easing.OutCubic
            }
            PropertyAnimation {
                to: 1.0
                duration: animationDuration / 2
                easing.type: Easing.InCubic
            }
        }
        
        SequentialAnimation on opacity {
            running: loadingIndicator.running && type === "pulse"
            loops: Animation.Infinite
            
            PropertyAnimation {
                to: 0.3
                duration: animationDuration / 2
                easing.type: Easing.OutCubic
            }
            PropertyAnimation {
                to: 1.0
                duration: animationDuration / 2
                easing.type: Easing.InCubic
            }
        }
    }
    
    // Wave type loading
    Row {
        id: waveRow
        anchors.centerIn: parent
        spacing: size / 12
        visible: type === "wave"
        
        Repeater {
            model: 5
            
            Rectangle {
                width: size / 10
                height: size / 3
                radius: width / 2
                color: loadingIndicator.color
                
                SequentialAnimation on height {
                    running: loadingIndicator.running && type === "wave"
                    loops: Animation.Infinite
                    
                    PauseAnimation { duration: index * 100 }
                    PropertyAnimation {
                        to: size
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                    PropertyAnimation {
                        to: size / 3
                        duration: 300
                        easing.type: Easing.InCubic
                    }
                    PauseAnimation { duration: (4 - index) * 100 }
                }
            }
        }
    }
    
    // Loading text
    Text {
        id: textLabel
        anchors.top: parent.verticalCenter
        anchors.topMargin: size / 2 + 8
        anchors.horizontalCenter: parent.horizontalCenter
        text: loadingIndicator.text
        color: loadingIndicator.color
        font.pixelSize: size / 4
        font.weight: Font.Medium
        visible: showText
        
        // Text fade animation
        SequentialAnimation on opacity {
            running: loadingIndicator.running && showText
            loops: Animation.Infinite
            
            PropertyAnimation {
                to: 0.5
                duration: 800
                easing.type: Easing.InOutQuad
            }
            PropertyAnimation {
                to: 1.0
                duration: 800
                easing.type: Easing.InOutQuad
            }
        }
    }
    
    // Overlay for full-screen loading
    Rectangle {
        id: overlay
        anchors.fill: parent
        color: "transparent"
        visible: false
        
        property bool fullScreen: false
        
        onFullScreenChanged: {
            if (fullScreen) {
                parent = loadingIndicator.parent
                anchors.fill = parent
                color = Qt.rgba(0, 0, 0, 0.5)
                visible = true
            } else {
                visible = false
            }
        }
    }
    
    // Public methods
    function start() {
        running = true
    }
    
    function stop() {
        running = false
    }
    
    function setFullScreen(enabled) {
        overlay.fullScreen = enabled
    }
}
