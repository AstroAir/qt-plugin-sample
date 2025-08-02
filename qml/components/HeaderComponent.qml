import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15
import "../controls"

AnimatedCard {
    id: header
    
    // Properties
    property bool darkMode: false
    property var pluginRegistry
    property string appTitle: "Plugin Manager"
    property string appSubtitle: "Manage and configure your plugins"
    
    // Signals
    signal refreshClicked()
    signal settingsClicked()
    signal darkModeToggled(bool enabled)
    
    // Header styling
    Layout.fillWidth: true
    Layout.preferredHeight: 80
    hoverEnabled: false
    
    contentItem: RowLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        
        // App Icon and Title Section
        RowLayout {
            spacing: 12
            
            // Enhanced App Icon
            Rectangle {
                width: 40
                height: 40
                radius: 20
                
                gradient: Gradient {
                    GradientStop { position: 0.0; color: Material.accent }
                    GradientStop { position: 1.0; color: Qt.darker(Material.accent, 1.3) }
                }
                
                // Icon shadow
                layer.enabled: true
                layer.effect: DropShadow {
                    horizontalOffset: 0
                    verticalOffset: 2
                    radius: 4
                    samples: 8
                    color: "#40000000"
                }
                
                Text {
                    anchors.centerIn: parent
                    text: "P"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }
                
                // Subtle pulse animation
                SequentialAnimation on scale {
                    running: true
                    loops: Animation.Infinite
                    PropertyAnimation { to: 1.05; duration: 2000; easing.type: Easing.InOutSine }
                    PropertyAnimation { to: 1.0; duration: 2000; easing.type: Easing.InOutSine }
                }
            }
            
            // Title and Subtitle
            Column {
                spacing: 2
                
                Text {
                    text: appTitle
                    color: darkMode ? "#ffffff" : "#212121"
                    font.pixelSize: 24
                    font.weight: Font.Bold
                }
                
                Text {
                    text: appSubtitle
                    color: darkMode ? "#b0b0b0" : "#757575"
                    font.pixelSize: 14
                    opacity: 0.8
                }
            }
        }
        
        // Spacer
        Item { Layout.fillWidth: true }
        
        // Action Buttons Section
        RowLayout {
            spacing: 12
            
            // Refresh Button with Animation
            CustomButton {
                id: refreshButton
                text: "Refresh"
                buttonType: "primary"
                glowEffect: refreshAnimation.running
                
                onClicked: {
                    refreshAnimation.start()
                    header.refreshClicked()
                    if (pluginRegistry) {
                        pluginRegistry.refreshPlugins()
                    }
                }
                
                // Refresh icon rotation animation
                RotationAnimation {
                    id: refreshAnimation
                    target: refreshButton
                    property: "rotation"
                    from: 0
                    to: 360
                    duration: 500
                    easing.type: Easing.OutCubic
                }
            }
            
            // Settings Button
            CustomButton {
                text: "Settings"
                buttonType: "secondary"
                
                onClicked: header.settingsClicked()
                
                // Hover glow effect
                glowEffect: hovered
            }
            
            // Enhanced Dark Mode Toggle
            Rectangle {
                width: 120
                height: 36
                radius: 18
                color: darkMode ? "#404040" : "#e0e0e0"
                border.color: darkMode ? "#606060" : "#c0c0c0"
                border.width: 1
                
                // Smooth color transition
                Behavior on color {
                    ColorAnimation {
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 4
                    
                    // Sun icon
                    Text {
                        text: "☀️"
                        font.pixelSize: 16
                        opacity: darkMode ? 0.5 : 1.0
                        
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: 300
                                easing.type: Easing.OutCubic
                            }
                        }
                    }
                    
                    // Toggle switch
                    Switch {
                        id: darkModeSwitch
                        Layout.fillWidth: true
                        checked: darkMode
                        
                        onCheckedChanged: {
                            header.darkModeToggled(checked)
                        }
                        
                        // Custom switch styling
                        Material.accent: Material.Blue
                    }
                    
                    // Moon icon
                    Text {
                        text: "🌙"
                        font.pixelSize: 16
                        opacity: darkMode ? 1.0 : 0.5
                        
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: 300
                                easing.type: Easing.OutCubic
                            }
                        }
                    }
                }
                
                // Click area for entire toggle
                MouseArea {
                    anchors.fill: parent
                    onClicked: darkModeSwitch.checked = !darkModeSwitch.checked
                }
            }
        }
    }
}
