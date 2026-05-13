import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import App 1.0

Dialog {
    id: dialog
    modal: true
    focus: true
    title: qsTr("Reservar lugar %1").arg(seatNumber)

    property int seatNumber: 0
    property string isoDate: ""

    signal confirmed(string firstName, string lastName)

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAboutToShow: {
        firstField.text = ""
        lastField.text = ""
        firstField.forceActiveFocus()
    }

    onAccepted: {
        if (firstField.text.trim() === "" || lastField.text.trim() === "")
            return
        dialog.confirmed(firstField.text.trim(), lastField.text.trim())
    }

    background: Rectangle {
        color: Theme.surface
        border.color: Theme.border
        border.width: 1
        radius: 8
    }

    header: Label {
        text: dialog.title
        padding: 16
        font.pixelSize: 18
        font.bold: true
        color: Theme.textPrimary
    }

    contentItem: ColumnLayout {
        spacing: 12

        Label {
            text: qsTr("Fecha: %1").arg(dialog.isoDate)
            color: Theme.textSecondary
        }

        Label {
            text: qsTr("Nombre")
            color: Theme.textPrimary
        }
        TextField {
            id: firstField
            Layout.fillWidth: true
            placeholderText: qsTr("Nombre")
            color: Theme.textPrimary
        }

        Label {
            text: qsTr("Apellido")
            color: Theme.textPrimary
        }
        TextField {
            id: lastField
            Layout.fillWidth: true
            placeholderText: qsTr("Apellido")
            color: Theme.textPrimary
            onAccepted: dialog.accept()
        }
    }
}
