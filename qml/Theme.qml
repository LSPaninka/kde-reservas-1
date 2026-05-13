pragma Singleton
import QtQuick 2.15

QtObject {
    id: theme

    property bool dark: false

    // Backgrounds
    property color background: dark ? "#1e1f22" : "#fafafa"
    property color surface:    dark ? "#2b2d31" : "#ffffff"
    property color border:     dark ? "#3f4147" : "#1f1f1f"

    // Text
    property color textPrimary:   dark ? "#f2f2f2" : "#111111"
    property color textSecondary: dark ? "#c0c0c0" : "#444444"
    property color textOnAccent:  "#ffffff"

    // States
    property color available:        dark ? "#1f7a3a" : "#2e9d4f"   // green
    property color availableHover:   dark ? "#26944b" : "#3cb863"
    property color reserved:         dark ? "#9b2828" : "#d23030"   // red
    property color reservedHover:    dark ? "#b03030" : "#e84545"

    property color accent: dark ? "#5a8ee8" : "#2962ff"

    function toggle() { dark = !dark }
}
