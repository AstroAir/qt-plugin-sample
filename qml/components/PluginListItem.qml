import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "../controls"

AnimatedCard {
    id: pluginItem
    
    // Properties
    property bool darkMode: false
    property var pluginRegistry
    property var model
    
    // Signals
    signal enableToggled(string pluginName, bool enabled)
    signal configureRequested(string pluginName)
    signal detailsRequested(string pluginName)
    signal removeRequested(string pluginName)
    
    // Item styling
    width: parent.width - 20
    height: 120
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
    
    contentItem: RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // Enhanced Plugin Icon with Status
        Rectangle {
            Layout.preferredWidth: 64
            Layout.preferredHeight: 64
            radius: 32
            
            // Enhanced gradient based on plugin status
            gradient: Gradient {
                GradientStop { 
                    position: 0.0
                    color: model && model.enabled ? Material.accent : "#9e9e9e"
                }
                GradientStop { 
                    position: 1.0
                    color: model && model.enabled ? 
                        Qt.darker(Material.accent, 1.2) : Qt.darker("#9e9e9e", 1.2)
                }
            }
            
            // Icon shadow
            layer.enabled: true
            layer.effect: DropShadow {
                horizontalOffset: 0
                verticalOffset: 2
                radius: 6
                samples: 12
                color: "#40000000"
            }
            
            // Plugin initial or icon
            Text {
                anchors.centerIn: parent
                text: model && model.name ? model.name.charAt(0).toUpperCase() : "P"
                color: "white"
                font.pixelSize: 24
                font.weight: Font.Bold
            }
            
            // Enhanced Status Indicator
            StatusIndicator {
                width: 20
                height: 20
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.rightMargin: -2
                anchors.bottomMargin: -2
                
                status: {
                    if (!model) return "inactive"
                    if (model.enabled) return "active"
                    if (model.hasError) return "error"
                    return "inactive"
                }
                
                // White border for better visibility
                border.color: darkMode ? "#2d2d2d" : "#ffffff"
                border.width: 2
            }
        }
        
        // Enhanced Plugin Information
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6
            
            // Plugin Name and Version Row
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Text {
                    text: model && model.name ? model.name : "Unknown Plugin"
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    color: darkMode ? "#ffffff" : "#212121"
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
                
                // Enhanced Version Badge
                Rectangle {
                    Layout.preferredWidth: versionText.width + 16
                    Layout.preferredHeight: 24
                    radius: 12
                    
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: darkMode ? "#404040" : "#e8f4fd" }
                        GradientStop { position: 1.0; color: darkMode ? "#353535" : "#d1e7dd" }
                    }
                    
                    Text {
                        id: versionText
                        anchors.centerIn: parent
                        text: "v" + (model && model.version ? model.version : "1.0")
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        color: darkMode ? "#ffffff" : "#0d6efd"
                    }
                }
            }
            
            // Enhanced Description
            Text {
                text: model && model.description ? model.description : "No description available"
                font.pixelSize: 14
                color: darkMode ? "#b0b0b0" : "#757575"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                maximumLineCount: 2
                elide: Text.ElideRight
                lineHeight: 1.2
            }
            
            // Enhanced Metadata Row
            RowLayout {
                Layout.fillWidth: true
                spacing: 20
                
                // Author info
                RowLayout {
                    spacing: 6
                    
                    Text {
                        text: "👤"
                        font.pixelSize: 12
                        color: darkMode ? "#b0b0b0" : "#757575"
                    }
                    
                    Text {
                        text: model && model.author ? model.author : "Unknown Author"
                        font.pixelSize: 12
                        color: darkMode ? "#b0b0b0" : "#757575"
                        font.weight: Font.Medium
                    }
                }
                
                // Category info
                RowLayout {
                    spacing: 6
                    
                    Text {
                        text: "📁"
                        font.pixelSize: 12
                        color: darkMode ? "#b0b0b0" : "#757575"
                    }
                    
                    Text {
                        text: model && model.category ? model.category : "General"
                        font.pixelSize: 12
                        color: darkMode ? "#b0b0b0" : "#757575"
                        font.weight: Font.Medium
                    }
                }
                
                Item { Layout.fillWidth: true }
                
                // Enhanced Performance Indicators
                RowLayout {
                    spacing: 12
                    
                    // CPU Usage
                    RowLayout {
                        spacing: 4
                        
                        StatusIndicator {
                            width: 8
                            height: 8
                            status: {
                                if (!model || !model.cpuUsage) return "inactive"
                                if (model.cpuUsage > 50) return "error"
                                if (model.cpuUsage > 20) return "warning"
                                return "active"
                            }
                        }
                        
                        Text {
                            text: "CPU: " + (model && model.cpuUsage ? model.cpuUsage.toFixed(1) : "0.0") + "%"
                            font.pixelSize: 10
                            color: darkMode ? "#b0b0b0" : "#757575"
                            font.weight: Font.Medium
                        }
                    }
                    
                    // Memory Usage
                    RowLayout {
                        spacing: 4
                        
                        StatusIndicator {
                            width: 8
                            height: 8
                            status: {
                                if (!model || !model.memoryUsage) return "inactive"
                                if (model.memoryUsage > 100) return "error"
                                if (model.memoryUsage > 50) return "warning"
                                return "active"
                            }
                        }
                        
                        Text {
                            text: "RAM: " + (model && model.memoryUsage ? model.memoryUsage.toFixed(0) : "0") + "MB"
                            font.pixelSize: 10
                            color: darkMode ? "#b0b0b0" : "#757575"
                            font.weight: Font.Medium
                        }
                    }
                }
            }
        }
        
        // Enhanced Action Buttons
        ColumnLayout {
            Layout.preferredWidth: 140
            spacing: 8
            
            // Primary Enable/Disable Button
            CustomButton {
                Layout.fillWidth: true
                text: model && model.enabled ? "Disable" : "Enable"
                buttonType: model && model.enabled ? "danger" : "primary"
                
                onClicked: {
                    if (model) {
                        pluginItem.enableToggled(model.name, !model.enabled)
                        if (pluginRegistry) {
                            if (model.enabled) {
                                pluginRegistry.disablePlugin(model.name)
                            } else {
                                pluginRegistry.enablePlugin(model.name)
                            }
                        }
                    }
                }
            }
            
            // Secondary Actions Row
            RowLayout {
                Layout.fillWidth: true
                spacing: 4
                
                // Configure Button
                CustomButton {
                    Layout.fillWidth: true
                    text: "⚙️"
                    buttonType: "secondary"
                    enabled: model && model.enabled
                    
                    ToolTip.visible: hovered
                    ToolTip.text: "Configure Plugin"
                    
                    onClicked: {
                        if (model) {
                            pluginItem.configureRequested(model.name)
                        }
                    }
                }
                
                // Details Button
                CustomButton {
                    Layout.fillWidth: true
                    text: "📊"
                    buttonType: "secondary"
                    
                    ToolTip.visible: hovered
                    ToolTip.text: "View Details"
                    
                    onClicked: {
                        if (model) {
                            pluginItem.detailsRequested(model.name)
                        }
                    }
                }
                
                // Remove Button
                CustomButton {
                    Layout.fillWidth: true
                    text: "🗑️"
                    buttonType: "secondary"
                    Material.foreground: "#f44336"
                    
                    ToolTip.visible: hovered
                    ToolTip.text: "Remove Plugin"
                    
                    onClicked: {
                        if (model) {
                            pluginItem.removeRequested(model.name)
                            if (pluginRegistry) {
                                pluginRegistry.uninstallPlugin(model.name)
                            }
                        }
                    }
                }
            }
        }
    }
}
