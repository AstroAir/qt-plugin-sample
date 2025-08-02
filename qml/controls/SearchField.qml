import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Rectangle {
    id: searchContainer
    
    // Custom properties
    property alias text: textField.text
    property alias placeholderText: textField.placeholderText
    property bool darkMode: false
    property string searchIcon: "🔍"
    property bool animated: true
    
    // Styling
    color: darkMode ? "#404040" : "#f5f5f5"
    radius: height / 2
    border.color: textField.activeFocus ? Material.accent : "transparent"
    border.width: 2
    
    // Size
    height: 40
    
    // Focus animation
    scale: textField.activeFocus ? 1.02 : 1.0
    
    Behavior on scale {
        enabled: animated
        PropertyAnimation {
            duration: 200
            easing.type: Easing.OutCubic
        }
    }
    
    Behavior on border.color {
        ColorAnimation {
            duration: 200
            easing.type: Easing.OutCubic
        }
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 8
        
        // Search icon
        Text {
            text: searchIcon
            font.pixelSize: 16
            color: darkMode ? "#b0b0b0" : "#757575"
            
            // Icon animation on focus
            scale: textField.activeFocus ? 1.1 : 1.0
            
            Behavior on scale {
                PropertyAnimation {
                    duration: 200
                    easing.type: Easing.OutCubic
                }
            }
        }
        
        // Text field
        TextField {
            id: textField
            Layout.fillWidth: true
            background: Rectangle { color: "transparent" }
            color: darkMode ? "#ffffff" : "#212121"
            font.pixelSize: 14
            selectByMouse: true
            
            // Enhanced placeholder styling
            placeholderTextColor: darkMode ? "#808080" : "#999999"
            
            // Text change animation
            onTextChanged: {
                if (animated) {
                    textChangeAnimation.start()
                }
                searchContainer.textChanged()
            }
            
            // Focus handling
            onActiveFocusChanged: {
                if (activeFocus) {
                    searchContainer.focused()
                } else {
                    searchContainer.unfocused()
                }
            }
            
            // Key handling
            Keys.onReturnPressed: searchContainer.searchRequested()
            Keys.onEnterPressed: searchContainer.searchRequested()
            Keys.onEscapePressed: {
                text = ""
                focus = false
            }
        }
        
        // Clear button
        CustomButton {
            visible: textField.text.length > 0
            text: "✕"
            buttonType: "secondary"
            flat: true
            Material.foreground: darkMode ? "#b0b0b0" : "#757575"
            width: 24
            height: 24
            
            onClicked: {
                textField.text = ""
                textField.forceActiveFocus()
                searchContainer.cleared()
            }
            
            // Fade in/out animation
            opacity: textField.text.length > 0 ? 1.0 : 0.0
            
            Behavior on opacity {
                PropertyAnimation {
                    duration: 200
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
    
    // Text change animation
    SequentialAnimation {
        id: textChangeAnimation
        PropertyAnimation {
            target: searchContainer
            property: "scale"
            from: 1.0
            to: 1.02
            duration: 100
            easing.type: Easing.OutQuad
        }
        PropertyAnimation {
            target: searchContainer
            property: "scale"
            from: 1.02
            to: 1.0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }
    
    // Signals
    signal textChanged()
    signal searchRequested()
    signal cleared()
    signal focused()
    signal unfocused()
    
    // Public methods
    function clear() {
        textField.text = ""
    }
    
    function focus() {
        textField.forceActiveFocus()
    }
    
    function selectAll() {
        textField.selectAll()
    }
}
