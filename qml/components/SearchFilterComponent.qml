import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "../controls"

AnimatedCard {
    id: searchFilter
    
    // Properties
    property bool darkMode: false
    property var pluginRegistry
    property alias searchText: searchField.text
    property alias categoryFilter: categoryCombo.currentText
    property alias statusFilter: statusCombo.currentText
    
    // Signals
    signal searchTextChanged(string text)
    signal categoryChanged(string category)
    signal statusChanged(string status)
    signal sortRequested(string sortType)
    
    // Component styling
    Layout.fillWidth: true
    Layout.preferredHeight: 70
    hoverEnabled: false
    
    contentItem: RowLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16
        
        // Enhanced Search Field
        SearchField {
            id: searchField
            Layout.fillWidth: true
            darkMode: searchFilter.darkMode
            placeholderText: "Search plugins by name, description, or author..."
            
            onTextChanged: {
                searchFilter.searchTextChanged(text)
                if (pluginRegistry) {
                    pluginRegistry.filterText = text
                }
            }
            
            onSearchRequested: {
                // Trigger immediate search
                if (pluginRegistry) {
                    pluginRegistry.refreshPlugins()
                }
            }
        }
        
        // Enhanced Category Filter
        ComboBox {
            id: categoryCombo
            Layout.preferredWidth: 160
            
            model: [
                "All Categories",
                "UI Plugins", 
                "Service Plugins", 
                "Network Plugins", 
                "Data Providers", 
                "Utilities",
                "Development Tools",
                "Security",
                "Analytics"
            ]
            
            Material.accent: Material.Blue
            
            // Enhanced styling
            background: Rectangle {
                color: darkMode ? "#404040" : "#f5f5f5"
                radius: 8
                border.color: categoryCombo.activeFocus ? Material.accent : "transparent"
                border.width: 2
                
                Behavior on border.color {
                    ColorAnimation {
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }
            }
            
            contentItem: Text {
                text: categoryCombo.displayText
                font.pixelSize: 14
                font.weight: Font.Medium
                color: darkMode ? "#ffffff" : "#212121"
                verticalAlignment: Text.AlignVCenter
                leftPadding: 12
            }
            
            onCurrentTextChanged: {
                searchFilter.categoryChanged(currentText)
                // Apply category filter logic here
            }
            
            // Custom dropdown styling
            popup: Popup {
                y: categoryCombo.height + 4
                width: categoryCombo.width
                implicitHeight: contentItem.implicitHeight
                padding: 4
                
                background: Rectangle {
                    color: darkMode ? "#2d2d2d" : "#ffffff"
                    radius: 8
                    border.color: darkMode ? "#404040" : "#e0e0e0"
                    border.width: 1
                    
                    // Drop shadow
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0
                        verticalOffset: 4
                        radius: 8
                        samples: 16
                        color: darkMode ? "#000000" : "#40000000"
                    }
                }
                
                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: categoryCombo.popup.visible ? categoryCombo.delegateModel : null
                    currentIndex: categoryCombo.highlightedIndex
                    
                    ScrollIndicator.vertical: ScrollIndicator { }
                }
            }
        }
        
        // Enhanced Status Filter
        ComboBox {
            id: statusCombo
            Layout.preferredWidth: 130
            
            model: [
                "All Status",
                "Enabled", 
                "Disabled", 
                "Error",
                "Loading",
                "Updating"
            ]
            
            Material.accent: Material.Blue
            
            // Consistent styling with category combo
            background: Rectangle {
                color: darkMode ? "#404040" : "#f5f5f5"
                radius: 8
                border.color: statusCombo.activeFocus ? Material.accent : "transparent"
                border.width: 2
                
                Behavior on border.color {
                    ColorAnimation {
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }
            }
            
            contentItem: RowLayout {
                spacing: 8
                
                // Status indicator
                StatusIndicator {
                    status: {
                        switch(statusCombo.currentText) {
                            case "Enabled": return "active"
                            case "Disabled": return "inactive"
                            case "Error": return "error"
                            case "Loading": return "loading"
                            default: return "inactive"
                        }
                    }
                    visible: statusCombo.currentText !== "All Status"
                    width: 8
                    height: 8
                }
                
                Text {
                    text: statusCombo.displayText
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    color: darkMode ? "#ffffff" : "#212121"
                    Layout.fillWidth: true
                }
            }
            
            onCurrentTextChanged: {
                searchFilter.statusChanged(currentText)
                // Apply status filter logic here
            }
        }
        
        // Enhanced Sort Button with Menu
        CustomButton {
            text: "Sort ▼"
            buttonType: "secondary"
            Layout.preferredWidth: 80
            
            onClicked: sortMenu.open()
            
            Menu {
                id: sortMenu
                y: parent.height + 4
                
                // Enhanced menu styling
                background: Rectangle {
                    color: darkMode ? "#2d2d2d" : "#ffffff"
                    radius: 8
                    border.color: darkMode ? "#404040" : "#e0e0e0"
                    border.width: 1
                    
                    layer.enabled: true
                    layer.effect: DropShadow {
                        horizontalOffset: 0
                        verticalOffset: 4
                        radius: 8
                        samples: 16
                        color: darkMode ? "#000000" : "#40000000"
                    }
                }
                
                MenuItem {
                    text: "📝 Name (A-Z)"
                    onTriggered: searchFilter.sortRequested("name_asc")
                }
                MenuItem {
                    text: "📝 Name (Z-A)"
                    onTriggered: searchFilter.sortRequested("name_desc")
                }
                MenuSeparator {}
                MenuItem {
                    text: "🕒 Recently Added"
                    onTriggered: searchFilter.sortRequested("date_added")
                }
                MenuItem {
                    text: "📊 Most Used"
                    onTriggered: searchFilter.sortRequested("usage")
                }
                MenuItem {
                    text: "⚡ Performance"
                    onTriggered: searchFilter.sortRequested("performance")
                }
                MenuSeparator {}
                MenuItem {
                    text: "✅ Enabled First"
                    onTriggered: searchFilter.sortRequested("enabled_first")
                }
            }
        }
    }
}
