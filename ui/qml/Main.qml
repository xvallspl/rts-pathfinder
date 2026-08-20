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

    readonly property color startColor: "#2e7d32"
    readonly property color targetColor: "#c62828"
    readonly property color pathColor: "#1565c0"
    readonly property color elevatedColor: "#424242"
    readonly property color groundColor: "#eeeeee"

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

            Item { Layout.fillWidth: true }

            RowLayout {
                spacing: 12
                visible: controller.hasMap

                Repeater {
                    model: [
                        { cellColor: window.startColor, label: qsTr("Start") },
                        { cellColor: window.targetColor, label: qsTr("Target") },
                        { cellColor: window.pathColor, label: qsTr("Path") },
                        { cellColor: window.elevatedColor, label: qsTr("Elevated") },
                        { cellColor: window.groundColor, label: qsTr("Ground") }
                    ]

                    RowLayout {
                        spacing: 4
                        Rectangle {
                            width: 14
                            height: 14
                            color: modelData.cellColor
                        }
                        Label { text: modelData.label }
                    }
                }
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

            // Visible only when the grid overflows the viewport. Explicitly
            // styled handles: the default translucent ones are illegible on
            // top of the grid cells.
            component GridScrollHandle: Rectangle {
                implicitWidth: 9
                implicitHeight: 9
                radius: 4.5
                color: "#555555"
                border.color: "#ffffff"
                border.width: 1
            }

            ScrollBar.vertical: ScrollBar {
                contentItem: GridScrollHandle {}
            }
            ScrollBar.horizontal: ScrollBar {
                contentItem: GridScrollHandle {}
            }

            delegate: Rectangle {
                implicitWidth: 20
                implicitHeight: 20
                color: isStart ? window.startColor
                     : isTarget ? window.targetColor
                     : isPath ? window.pathColor
                     : elevated ? window.elevatedColor
                     : window.groundColor
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            visible: controller.hasSolution
            // Pin the content width to the viewport so the text wraps and
            // scrolls vertically instead of running off the right edge.
            contentWidth: availableWidth

            Label {
                width: parent.width
                wrapMode: Text.Wrap
                text: controller.pathText
            }
        }
    }
}
