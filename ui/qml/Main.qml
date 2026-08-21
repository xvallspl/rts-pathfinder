import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import RtsPathfinder

ApplicationWindow {
    id: window
    width: 800
    height: 680
    visible: true
    title: qsTr("RTS Battle Unit Path-Finding")

    readonly property color startColor: "#2e7d32"
    readonly property color targetColor: "#c62828"
    readonly property color pathColor: "#1565c0"
    readonly property color unitColor: "#ef6c00"
    readonly property color elevatedColor: "#424242"
    readonly property color groundColor: "#eeeeee"

    readonly property int cellSize: 20
    readonly property int cellSpacing: 1

    // The default translucent handles are illegible against both the grid
    // cells and the dark background, so every scroll area uses this instead.
    component StyledScrollHandle: Rectangle {
        implicitWidth: 9
        implicitHeight: 9
        radius: 4.5
        color: "#555555"
        border.color: "#ffffff"
        border.width: 1
    }

    SolverController {
        id: controller
    }

    Timer {
        interval: 1000
        repeat: true
        running: controller.isReplaying
        onTriggered: controller.advance()
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
        spacing: 10

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

            Label {
                visible: controller.tickCount > 0
                text: qsTr("Tick %1 / %2").arg(controller.currentTick)
                                          .arg(controller.tickCount - 1)
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: controller.statusMessage
        }

        // The legend gets its own row, centered above the grid it explains.
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12
            visible: controller.hasMap

            Repeater {
                model: [
                    { cellColor: window.startColor, label: qsTr("Start") },
                    { cellColor: window.targetColor, label: qsTr("Target") },
                    { cellColor: window.pathColor, label: qsTr("Path") },
                    { cellColor: window.unitColor, label: qsTr("Unit") },
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

        // Flexible middle area; the grid sits centered inside it and never
        // grows beyond its own content. Sized from the model's row and
        // column counts -- stable during replay, unlike contentWidth/Height,
        // which fluctuate as delegates come and go.
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TableView {
                id: tableView

                readonly property int idealWidth:
                    columns * (window.cellSize + window.cellSpacing) - window.cellSpacing
                readonly property int idealHeight:
                    rows * (window.cellSize + window.cellSpacing) - window.cellSpacing

                anchors.centerIn: parent
                width: Math.max(0, Math.min(idealWidth, parent.width))
                height: Math.max(0, Math.min(idealHeight, parent.height))
                clip: true
                columnSpacing: window.cellSpacing
                rowSpacing: window.cellSpacing
                model: controller.mapModel

                // Visible only when the grid overflows the viewport.
                ScrollBar.vertical: ScrollBar {
                    contentItem: StyledScrollHandle {}
                }
                ScrollBar.horizontal: ScrollBar {
                    contentItem: StyledScrollHandle {}
                }

                delegate: Rectangle {
                    implicitWidth: window.cellSize
                    implicitHeight: window.cellSize
                    color: isUnit ? window.unitColor
                         : isStart ? window.startColor
                         : isTarget ? window.targetColor
                         : isPath ? window.pathColor
                         : elevated ? window.elevatedColor
                         : window.groundColor
                }
            }
        }

        // Tall enough to show the whole path list whenever it fits in a
        // reasonable share of the window; beyond that it scrolls.
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(pathLabel.implicitHeight + 12,
                                             window.height * 0.4)
            visible: controller.hasSolution
            // Pin the content width to the viewport so the text wraps and
            // scrolls vertically instead of running off the right edge.
            contentWidth: availableWidth
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                contentItem: StyledScrollHandle {}
            }

            Label {
                id: pathLabel
                width: parent.width
                wrapMode: Text.Wrap
                text: controller.pathText
            }
        }
    }
}
