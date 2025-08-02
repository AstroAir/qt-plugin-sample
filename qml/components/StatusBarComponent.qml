import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15
import "../controls"

AnimatedCard {
    id: statusBar
    
    // Properties
    property bool darkMode: false
    property var pluginRegistry
    property string systemStatus: "ready" // ready, loading, error, warning
    property real totalMemoryUsage: 0
    property int totalPlugins: 0
    property int enabledPlugins: 0
    
    // Component styling
    Layout.fillWidth: true
    Layout.preferredHeight: 50
    hoverEnabled: false
    
    // Custom shadow direction (upward)
    layer.effect: DropShadow {
        horizontalOffset: 0
        verticalOffset: -1
        radius: 4
        samples: 8
        color: darkMode ? "#000000" : "#20000000"
        transparentBorder: true
    }
    
    contentItem: RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 20
        
        // Plugin Statistics Section
        RowLayout {
            spacing: 20
            
            // Total Plugins Count
            RowLayout {
                spacing: 8
                
                StatusIndicator {
                    width: 8
                    height: 8
                    status: "active"
                    customColor: Material.accent
                }
                
                Text {
                    text: "Total Plugins: " + (pluginRegistry ? pluginRegistry.count : totalPlugins)
                    color: darkMode ? "#ffffff" : "#212121"
                    font.pixelSize: 14
                    font.weight: Font.Medium
                }
            }
            
            // Enabled Plugins Count
            RowLayout {
                spacing: 8
                
                StatusIndicator {
                    width: 8
                    height: 8
                    status: "active"
                    customColor: "#4caf50"
                }
                
                Text {
                    text: "Enabled: " + (pluginRegistry ? pluginRegistry.enabledCount : enabledPlugins)
                    color: darkMode ? "#ffffff" : "#212121"
                    font.pixelSize: 14
                }
            }
            
            // Disabled Plugins Count
            RowLayout {
                spacing: 8
                
                StatusIndicator {
                    width: 8
                    height: 8
                    status: "inactive"
                    customColor: "#9e9e9e"
                }
                
                Text {
                    text: "Disabled: " + ((pluginRegistry ? pluginRegistry.count : totalPlugins) - (pluginRegistry ? pluginRegistry.enabledCount : enabledPlugins))
                    color: darkMode ? "#ffffff" : "#212121"
                    font.pixelSize: 14
                }
            }
        }
        
        // Separator
        Rectangle {
            width: 1
            height: parent.height * 0.6
            color: darkMode ? "#404040" : "#e0e0e0"
        }
        
        // System Resources Section
        RowLayout {
            spacing: 20
            
            // Memory Usage
            RowLayout {
                spacing: 8
                
                StatusIndicator {
                    width: 8
                    height: 8
                    status: {
                        var memUsage = pluginRegistry ? pluginRegistry.totalMemoryUsage : totalMemoryUsage
                        if (memUsage > 500) return "error"
                        if (memUsage > 200) return "warning"
                        return "active"
                    }
                    customColor: "#ff9800"
                }
                
                Text {
                    text: "Memory: " + (pluginRegistry ? pluginRegistry.totalMemoryUsage : totalMemoryUsage).toFixed(1) + " MB"
                    color: darkMode ? "#ffffff" : "#212121"
                    font.pixelSize: 14
                }
            }
            
            // CPU Usage (if available)
            RowLayout {
                spacing: 8
                visible: pluginRegistry && pluginRegistry.totalCpuUsage !== undefined
                
                StatusIndicator {
                    width: 8
                    height: 8
                    status: {
                        if (!pluginRegistry) return "inactive"
                        var cpuUsage = pluginRegistry.totalCpuUsage || 0
                        if (cpuUsage > 70) return "error"
                        if (cpuUsage > 40) return "warning"
                        return "active"
                    }
                    customColor: "#2196f3"
                }
                
                Text {
                    text: "CPU: " + (pluginRegistry ? (pluginRegistry.totalCpuUsage || 0).toFixed(1) : "0.0") + "%"
                    color: darkMode ? "#ffffff" : "#212121"
                    font.pixelSize: 14
                }
            }
        }
        
        // Spacer
        Item { Layout.fillWidth: true }
        
        // System Status Section
        RowLayout {
            spacing: 8
            
            // Animated Status Indicator
            StatusIndicator {
                width: 12
                height: 12
                status: {
                    switch(systemStatus) {
                        case "ready": return "active"
                        case "loading": return "loading"
                        case "error": return "error"
                        case "warning": return "warning"
                        default: return "active"
                    }
                }
                animated: true
            }
            
            // Status Text
            Text {
                text: {
                    switch(systemStatus) {
                        case "ready": return "System Ready"
                        case "loading": return "Loading..."
                        case "error": return "System Error"
                        case "warning": return "Warning"
                        default: return "System Ready"
                    }
                }
                color: {
                    switch(systemStatus) {
                        case "ready": return "#4caf50"
                        case "loading": return "#2196f3"
                        case "error": return "#f44336"
                        case "warning": return "#ff9800"
                        default: return "#4caf50"
                    }
                }
                font.pixelSize: 14
                font.weight: Font.Medium
            }
        }
        
        // Quick Actions (if space allows)
        RowLayout {
            spacing: 8
            visible: statusBar.width > 800
            
            // Refresh indicator
            CustomButton {
                text: "🔄"
                buttonType: "secondary"
                width: 32
                height: 32
                
                ToolTip.visible: hovered
                ToolTip.text: "Refresh Status"
                
                onClicked: {
                    if (pluginRegistry) {
                        pluginRegistry.refreshPlugins()
                    }
                }
            }
            
            // Settings quick access
            CustomButton {
                text: "⚙️"
                buttonType: "secondary"
                width: 32
                height: 32
                
                ToolTip.visible: hovered
                ToolTip.text: "Quick Settings"
                
                onClicked: {
                    // Open quick settings
                }
            }
        }
    }
    
    // Update properties when pluginRegistry changes
    Connections {
        target: pluginRegistry
        function onCountChanged() {
            totalPlugins = pluginRegistry.count
        }
        function onEnabledCountChanged() {
            enabledPlugins = pluginRegistry.enabledCount
        }
        function onTotalMemoryUsageChanged() {
            totalMemoryUsage = pluginRegistry.totalMemoryUsage
        }
    }
}
