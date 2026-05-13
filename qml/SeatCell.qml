import QtQuick 2.15
import QtQuick.Controls 2.15
import App 1.0

Rectangle {
    id: cell

    property int seatNumber: 0
    property bool reserved: false
    property string firstName: ""
    property string lastName: ""

    signal clicked()

    implicitWidth: 220
    implicitHeight: 130

    color: reserved
        ? (mouseArea.containsMouse ? Theme.reservedHover : Theme.reserved)
        : (mouseArea.containsMouse ? Theme.availableHover : Theme.available)

    border.color: Theme.border
    border.width: 2
    radius: 6

    Behavior on color { ColorAnimation { duration: 120 } }

    Column {
        anchors.centerIn: parent
        spacing: 4
        width: parent.width - 16

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: reserved ? cell.firstName : qsTr("Disponible")
            font.pixelSize: 22
            font.bold: true
            color: Theme.textOnAccent
            elide: Text.ElideRight
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: reserved ? cell.lastName : ""
            visible: reserved
            font.pixelSize: 20
            color: Theme.textOnAccent
            elide: Text.ElideRight
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Lugar %1").arg(cell.seatNumber)
            visible: !reserved
            font.pixelSize: 14
            color: Theme.textOnAccent
            opacity: 0.85
        }
    }

    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 6
        width: 26
        height: 18
        radius: 9
        color: Qt.rgba(0, 0, 0, 0.25)
        Text {
            anchors.centerIn: parent
            text: cell.seatNumber
            color: "#ffffff"
            font.pixelSize: 11
            font.bold: true
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: cell.clicked()
    }
}
