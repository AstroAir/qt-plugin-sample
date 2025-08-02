import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "../controls"

AnimatedCard {
    id: footer
    
    // Properties
    property bool darkMode: false
    
    // Signals
    signal installPluginRequested()
    signal pluginStoreRequested()
    signal createPluginRequested()
    signal helpRequested()
    signal aboutRequested()
    
    // Component styling
    Layout.fillWidth: true
    Layout.preferredHeight: 60
    hoverEnabled: false
    
    // Enhanced footer styling
    color: darkMode ? "#252525" : "#f8f9fa"
    
    contentItem: RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        
        // Primary Actions Section
        RowLayout {
            spacing: 12
            
            // Install Plugin Button
            CustomButton {
                text: "📦 Install Plugin"
                buttonType: "primary"
                font.weight: Font.Medium
                
                // Enhanced styling
                Material.elevation: 2
                
                onClicked: {
                    footer.installPluginRequested()
                    // Could open file dialog or installation wizard
                }
                
                // Subtle glow effect
                glowEffect: hovered
            }
            
            // Plugin Store Button
            CustomButton {
                text: "🏪 Plugin Store"
                buttonType: "secondary"
                font.weight: Font.Medium
                
                onClicked: {
                    footer.pluginStoreRequested()
                    // Open plugin marketplace/store
                }
                
                // Hover animation
                scale: hovered ? 1.05 : 1.0
                
                Behavior on scale {
                    PropertyAnimation {
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }
            }
            
            // Create Plugin Button
            CustomButton {
                text: "🛠️ Create Plugin"
                buttonType: "secondary"
                font.weight: Font.Medium
                
                onClicked: {
                    footer.createPluginRequested()
                    // Open plugin creation wizard
                }
                
                // Hover animation
                scale: hovered ? 1.05 : 1.0
                
                Behavior on scale {
                    PropertyAnimation {
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }
        
        // Spacer
        Item { Layout.fillWidth: true }
        
        // Secondary Actions Section
        RowLayout {
            spacing: 8
            
            // Quick Stats (compact view)
            Rectangle {
                Layout.preferredWidth: statsRow.width + 16
                Layout.preferredHeight: 32
                radius: 16
                color: darkMode ? "#404040" : "#e8f4fd"
                visible: footer.width > 600
                
                RowLayout {
                    id: statsRow
                    anchors.centerIn: parent
                    spacing: 12
                    
                    // Active plugins indicator
                    RowLayout {
                        spacing: 4
                        
                        StatusIndicator {
                            width: 6
                            height: 6
                            status: "active"
                            customColor: "#4caf50"
                        }
                        
                        Text {
                            text: "Active"
                            font.pixelSize: 11
                            color: darkMode ? "#ffffff" : "#0d6efd"
                            font.weight: Font.Medium
                        }
                    }
                    
                    // Separator
                    Rectangle {
                        width: 1
                        height: 12
                        color: darkMode ? "#606060" : "#c0c0c0"
                    }
                    
                    // System status
                    Text {
                        text: "Ready"
                        font.pixelSize: 11
                        color: darkMode ? "#ffffff" : "#0d6efd"
                        font.weight: Font.Medium
                    }
                }
            }
            
            // Separator
            Rectangle {
                width: 1
                height: parent.height * 0.6
                color: darkMode ? "#404040" : "#e0e0e0"
                visible: footer.width > 500
            }
            
            // Help and About Section
            RowLayout {
                spacing: 8
                
                // Help Button
                CustomButton {
                    text: "❓ Help"
                    buttonType: "secondary"
                    Material.foreground: darkMode ? "#b0b0b0" : "#757575"
                    
                    onClicked: {
                        footer.helpRequested()
                        // Open help documentation or tutorial
                    }
                    
                    ToolTip.visible: hovered
                    ToolTip.text: "Open Help Documentation"
                }
                
                // About Button
                CustomButton {
                    text: "ℹ️"
                    buttonType: "secondary"
                    Material.foreground: darkMode ? "#b0b0b0" : "#757575"
                    width: 36
                    
                    onClicked: {
                        footer.aboutRequested()
                        // Show about dialog
                    }
                    
                    ToolTip.visible: hovered
                    ToolTip.text: "About Plugin Manager"
                }
            }
        }
    }
    
    // Subtle border at the top
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 1
        color: darkMode ? "#404040" : "#e0e0e0"
        opacity: 0.5
    }
    
    // Enhanced visual feedback for actions
    Rectangle {
        id: actionFeedback
        anchors.fill: parent
        radius: parent.radius
        color: Material.accent
        opacity: 0
        
        PropertyAnimation {
            id: feedbackAnimation
            target: actionFeedback
            property: "opacity"
            from: 0.2
            to: 0
            duration: 300
            easing.type: Easing.OutCubic
        }
        
        function showFeedback() {
            feedbackAnimation.start()
        }
    }
    
    // Connect feedback to button clicks
    Component.onCompleted: {
        installPluginRequested.connect(actionFeedback.showFeedback)
        pluginStoreRequested.connect(actionFeedback.showFeedback)
        createPluginRequested.connect(actionFeedback.showFeedback)
    }
}
