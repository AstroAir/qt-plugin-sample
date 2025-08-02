import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtGraphicalEffects 1.15
import "components"
import "controls"

ApplicationWindow {
    id: root
    width: 1000
    height: 700
    title: "Plugin Manager"

    property var pluginManager
    property var pluginRegistry
    property var themeManager
    property bool darkMode: false

    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: Material.Blue
    Material.primary: Material.Blue

    // Enhanced background with theme support
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: darkMode ? "#1e1e1e" : "#fafafa"
            }
            GradientStop {
                position: 1.0
                color: darkMode ? "#121212" : "#f5f5f5"
            }
        }

        // Subtle texture overlay
        Rectangle {
            anchors.fill: parent
            opacity: 0.02
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.5; color: darkMode ? "#ffffff" : "#000000" }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        // Header Component
        HeaderComponent {
            darkMode: root.darkMode
            pluginRegistry: root.pluginRegistry

            onRefreshClicked: {
                if (pluginRegistry) {
                    pluginRegistry.refreshPlugins()
                }
            }

            onSettingsClicked: {
                // Open settings dialog
                console.log("Settings clicked")
            }

            onDarkModeToggled: function(enabled) {
                root.darkMode = enabled
            }
        }

        // Search and Filter Component
        SearchFilterComponent {
            darkMode: root.darkMode
            pluginRegistry: root.pluginRegistry

            onSearchTextChanged: function(text) {
                if (pluginRegistry) {
                    pluginRegistry.filterText = text
                }
            }

            onCategoryChanged: function(category) {
                console.log("Category filter changed:", category)
                // Apply category filter logic
            }

            onStatusChanged: function(status) {
                console.log("Status filter changed:", status)
                // Apply status filter logic
            }

            onSortRequested: function(sortType) {
                console.log("Sort requested:", sortType)
                // Apply sorting logic
            }
        }

        // Enhanced Plugin List with Cards
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: pluginListView
                model: pluginRegistry
                spacing: 12

                delegate: PluginListItem {
                    darkMode: root.darkMode
                    pluginRegistry: root.pluginRegistry
                    model: modelData || model

                    onEnableToggled: function(pluginName, enabled) {
                        console.log("Plugin", pluginName, enabled ? "enabled" : "disabled")
                    }

                    onConfigureRequested: function(pluginName) {
                        console.log("Configure requested for:", pluginName)
                        // Open configuration dialog
                    }

                    onDetailsRequested: function(pluginName) {
                        console.log("Details requested for:", pluginName)
                        // Show plugin details
                    }

                    onRemoveRequested: function(pluginName) {
                        console.log("Remove requested for:", pluginName)
                        // Confirm and remove plugin
                    }
                }


            }
        }

        // Status Bar Component
        StatusBarComponent {
            darkMode: root.darkMode
            pluginRegistry: root.pluginRegistry
            systemStatus: "ready"
        }

        // Quick Actions Footer Component
        QuickActionsFooter {
            darkMode: root.darkMode

            onInstallPluginRequested: {
                console.log("Install plugin requested")
                // Open file dialog to install plugin
            }

            onPluginStoreRequested: {
                console.log("Plugin store requested")
                // Open plugin store
            }

            onCreatePluginRequested: {
                console.log("Create plugin requested")
                // Open plugin creation wizard
            }

            onHelpRequested: {
                console.log("Help requested")
                // Open help documentation
            }

            onAboutRequested: {
                console.log("About requested")
                // Show about dialog
            }
        }
    }
}
