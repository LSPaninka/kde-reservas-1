import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import Qt.labs.calendar 1.0
import App 1.0
import "." as Local

ApplicationWindow {
    id: root
    visible: true
    width: 880
    height: 820
    minimumWidth: 520
    minimumHeight: 600
    title: qsTr("Copérnico — Reserva de Oficina")

    color: Theme.background

    Connections {
        target: reservationModel
        function onErrorOccurred(message) {
            errorPopup.show(message)
        }
    }

    header: ToolBar {
        background: Rectangle { color: Theme.surface }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            spacing: 8

            ToolButton {
                text: "☰"
                font.pixelSize: 20
                onClicked: drawer.open()
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Item { Layout.fillWidth: true }

            ToolButton {
                text: qsTr("Hoy")
                onClicked: reservationModel.goToToday()
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            ToolButton {
                text: Theme.dark ? "☼" : "☾" // sun / moon glyphs
                onClicked: Theme.toggle()
                ToolTip.visible: hovered
                ToolTip.text: Theme.dark ? qsTr("Modo claro") : qsTr("Modo oscuro")
                contentItem: Text {
                    text: parent.text
                    color: Theme.textPrimary
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    Drawer {
        id: drawer
        width: Math.min(360, root.width * 0.85)
        height: root.height
        edge: Qt.LeftEdge

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            Label {
                text: qsTr("Calendario")
                font.pixelSize: 20
                font.bold: true
                color: Theme.textPrimary
            }

            RowLayout {
                Layout.fillWidth: true
                ToolButton {
                    text: "<"
                    onClicked: {
                        if (monthGrid.month === 0) {
                            monthGrid.month = 11
                            monthGrid.year -= 1
                        } else {
                            monthGrid.month -= 1
                        }
                    }
                    contentItem: Text { text: parent.text; color: Theme.textPrimary; font.pixelSize: 16; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: Qt.locale("es").standaloneMonthName(monthGrid.month + 1) + " " + monthGrid.year
                    color: Theme.textPrimary
                    font.pixelSize: 16
                    font.bold: true
                }
                ToolButton {
                    text: ">"
                    onClicked: {
                        if (monthGrid.month === 11) {
                            monthGrid.month = 0
                            monthGrid.year += 1
                        } else {
                            monthGrid.month += 1
                        }
                    }
                    contentItem: Text { text: parent.text; color: Theme.textPrimary; font.pixelSize: 16; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }

            DayOfWeekRow {
                Layout.fillWidth: true
                locale: Qt.locale("es")
                delegate: Label {
                    text: model.shortName
                    color: Theme.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    font.bold: true
                }
            }

            MonthGrid {
                id: monthGrid
                Layout.fillWidth: true
                locale: Qt.locale("es")
                month: {
                    var d = Date.fromLocaleString(Qt.locale(), reservationModel.currentDate, "yyyy-MM-dd")
                    return d.getMonth()
                }
                year: {
                    var d = Date.fromLocaleString(Qt.locale(), reservationModel.currentDate, "yyyy-MM-dd")
                    return d.getFullYear()
                }

                delegate: Rectangle {
                    property bool selected: {
                        var iso = Qt.formatDate(model.date, "yyyy-MM-dd")
                        return iso === reservationModel.currentDate
                    }
                    property bool today: {
                        var n = new Date()
                        return model.date.getFullYear() === n.getFullYear()
                            && model.date.getMonth() === n.getMonth()
                            && model.date.getDate() === n.getDate()
                    }
                    implicitHeight: 32
                    color: selected ? Theme.accent : "transparent"
                    border.color: today ? Theme.accent : "transparent"
                    border.width: today ? 1 : 0
                    radius: 4
                    Text {
                        anchors.centerIn: parent
                        text: model.day
                        color: selected
                            ? Theme.textOnAccent
                            : (model.month === monthGrid.month
                                ? Theme.textPrimary
                                : Theme.textSecondary)
                        font.bold: today
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            var iso = Qt.formatDate(model.date, "yyyy-MM-dd")
                            reservationModel.currentDate = iso
                        }
                    }
                }
            }

            Button {
                text: qsTr("Ir a hoy")
                Layout.fillWidth: true
                onClicked: {
                    reservationModel.goToToday()
                    var d = new Date()
                    monthGrid.month = d.getMonth()
                    monthGrid.year = d.getFullYear()
                }
            }

            Item { Layout.fillHeight: true }

            Label {
                text: qsTr("API REST: http://localhost:%1").arg(restPort)
                color: Theme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 8

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: "COPÉRNICO"
            color: Theme.textPrimary
            font.pixelSize: 56
            font.bold: true
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Oficina")
            color: Theme.textPrimary
            font.pixelSize: 40
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: reservationModel.displayDate
            color: Theme.textSecondary
            font.pixelSize: 24
            bottomPadding: 8
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: reservationModel
            cellWidth: Math.floor(width / 2)
            cellHeight: Math.min(160, Math.floor(height / 4))
            interactive: false
            clip: true

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight
                Local.SeatCell {
                    anchors.fill: parent
                    anchors.margins: 6
                    seatNumber: model.seat
                    reserved: model.reserved
                    firstName: model.firstName
                    lastName: model.lastName
                    onClicked: {
                        if (model.reserved) {
                            cancelPopup.openFor(model.seat, model.fullName)
                        } else {
                            reservationDialog.seatNumber = model.seat
                            reservationDialog.isoDate = reservationModel.currentDate
                            reservationDialog.open()
                        }
                    }
                }
            }
        }
    }

    Local.ReservationDialog {
        id: reservationDialog
        anchors.centerIn: parent
        onConfirmed: function(firstName, lastName) {
            reservationModel.reserve(seatNumber, firstName, lastName)
        }
    }

    Dialog {
        id: cancelPopup
        anchors.centerIn: parent
        modal: true
        property int seatNumber: 0
        property string fullName: ""
        title: qsTr("Cancelar reserva")
        standardButtons: Dialog.Yes | Dialog.No

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.border
            border.width: 1
            radius: 8
        }

        function openFor(seat, name) {
            seatNumber = seat
            fullName = name
            open()
        }

        contentItem: Label {
            text: qsTr("¿Liberar el lugar %1 (%2)?").arg(cancelPopup.seatNumber).arg(cancelPopup.fullName)
            color: Theme.textPrimary
            wrapMode: Text.Wrap
        }

        onAccepted: reservationModel.cancel(seatNumber)
    }

    Popup {
        id: errorPopup
        anchors.centerIn: parent
        modal: true
        property string message: ""
        function show(msg) { message = msg; open() }

        background: Rectangle {
            color: Theme.surface
            border.color: Theme.reserved
            border.width: 2
            radius: 8
        }

        contentItem: ColumnLayout {
            spacing: 12
            Label {
                text: qsTr("Error")
                font.bold: true
                font.pixelSize: 18
                color: Theme.reserved
            }
            Label {
                text: errorPopup.message
                color: Theme.textPrimary
                wrapMode: Text.Wrap
                Layout.preferredWidth: 280
            }
            Button {
                text: qsTr("Cerrar")
                Layout.alignment: Qt.AlignRight
                onClicked: errorPopup.close()
            }
        }
    }
}
