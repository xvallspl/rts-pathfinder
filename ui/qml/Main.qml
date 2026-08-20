import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import RtsPathfinder

ApplicationWindow {
    id: window
    width: 800
    height: 640
    visible: true
    title: qsTr("RTS Battle Unit Path-Finding")

    SolverController {
        id: controller
    }

    FileDialog {
        id: openDialog
        title: qsTr("Open Map")
        nameFilters: [qsTr("Tilemap JSON (*.json)"), qsTr("All files (*)")]
        onAccepted: controller.loadMap(selectedFile)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        RowLayout {
            spacing: 8

            Button {
                text: qsTr("Open Map...")
                onClicked: openDialog.open()
            }

            Button {
                text: qsTr("Solve")
                enabled: controller.hasMap
                onClicked: controller.solve()
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: controller.statusMessage
        }

        TableView {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            columnSpacing: 1
            rowSpacing: 1
            model: controller.mapModel

            delegate: Rectangle {
                implicitWidth: 20
                implicitHeight: 20
                color: isStart ? "#2e7d32"
                     : isTarget ? "#c62828"
                     : isPath ? "#1565c0"
                     : elevated ? "#424242"
                     : "#eeeeee"
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            visible: controller.hasSolution

            Label {
                width: parent.width
                wrapMode: Text.Wrap
                text: controller.pathText
            }
        }
    }
}
