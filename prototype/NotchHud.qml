// Workledge — Notch/Pill HUD prototype
// Reusable QML component: collapsed pill expands into a small control panel.
// Data comes either from local fallback properties or from an injected `bridge`
// (same pattern as Workrave's StatusWindowBridge).

import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: root
    clip: true
    implicitWidth: collapsedWidth
    implicitHeight: collapsedHeight

    // ---- Public API -------------------------------------------------------
    property bool expanded: false
    property real collapsedWidth: 360
    property real collapsedHeight: 46
    property real expandedHeight: 236

    property string microTime: "04:58"
    property string restTime: "09:41"
    property string dailyTime: "2:47:12"
    property real microProgress: 0.72
    property real restProgress: 0.38
    property real dailyProgress: 0.61
    property string nextBreakLabel: "Micro break in"
    property string nextBreakTime: microTime

    // Optional C++ bridge. When null, demo data above is used.
    property var bridge: null

    signal takeBreak()
    signal postpone()
    signal settingsRequested()

    // ---- Bridge-aware values ---------------------------------------------
    readonly property string _microTime: (bridge && bridge.microTime) ? bridge.microTime : microTime
    readonly property string _restTime: (bridge && bridge.restTime) ? bridge.restTime : restTime
    readonly property string _dailyTime: (bridge && bridge.dailyTime) ? bridge.dailyTime : dailyTime
    readonly property real _microProgress: (bridge && bridge.microProgress !== undefined) ? bridge.microProgress : microProgress
    readonly property real _restProgress: (bridge && bridge.restProgress !== undefined) ? bridge.restProgress : restProgress
    readonly property real _dailyProgress: (bridge && bridge.dailyProgress !== undefined) ? bridge.dailyProgress : dailyProgress
    readonly property string _nextLabel: (bridge && bridge.nextBreakLabel) ? bridge.nextBreakLabel : nextBreakLabel
    readonly property string _nextTime: (bridge && bridge.nextBreakTime) ? bridge.nextBreakTime : nextBreakTime

    // ---- Collapse / expand ------------------------------------------------
    states: [
        State {
            name: "expanded"
            when: root.expanded
            PropertyChanges { target: root; height: root.expandedHeight }
        },
        State {
            name: "collapsed"
            when: !root.expanded
            PropertyChanges { target: root; height: root.collapsedHeight }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            properties: "height"
            duration: 200
            easing.type: Easing.OutCubic
        }
    }

    // ---- Glass-like surface ----------------------------------------------
    Rectangle {
        anchors.fill: parent
        radius: root.expanded ? 20 : root.collapsedHeight / 2
        color: "#E620242B"
        border.color: "#55FFFFFF"
        border.width: 1
        Behavior on radius { NumberAnimation { duration: 200; easing.type: Easing.OutCubic } }
    }

    // ---- Collapsed pill content ------------------------------------------
    Row {
        id: pill
        anchors {
            left: parent.left
            leftMargin: 16
            right: parent.right
            rightMargin: 14
            verticalCenter: parent.verticalCenter
        }
        spacing: 10
        opacity: root.expanded ? 0 : 1
        enabled: !root.expanded
        Behavior on opacity { NumberAnimation { duration: 120 } }

        Rectangle {
            width: 8
            height: 8
            radius: 4
            color: "#7DD3A7"
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root._nextLabel
            color: "#C9CED6"
            font.pixelSize: 12
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root._nextTime
            color: "#FFFFFF"
            font.pixelSize: 14
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }

        Rectangle {
            width: 1
            height: 18
            color: "#44FFFFFF"
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: "Mic " + root._microTime + "  ·  Rest " + root._restTime
            color: "#9AA2AD"
            font.pixelSize: 11
            elide: Text.ElideRight
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: root.expanded ? "▴" : "▾"
            color: "#C9CED6"
            font.pixelSize: 12
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // Click on the collapsed pill only; disabled while expanded so buttons work.
    MouseArea {
        anchors.fill: parent
        enabled: !root.expanded
        z: 20
        cursorShape: Qt.PointingHandCursor
        onClicked: root.expanded = true
    }

    // ---- Expanded panel content ------------------------------------------
    component TimerRow : Item {
        property string label: ""
        property string time: ""
        property real progress: 0
        property string accent: "#7DD3A7"

        width: parent.width
        height: 34

        Text {
            id: timerLabel
            anchors { left: parent.left; verticalCenter: parent.verticalCenter }
            text: label
            color: "#C9CED6"
            font.pixelSize: 11
        }

        Text {
            anchors { right: parent.right; verticalCenter: parent.verticalCenter }
            text: time
            color: "#FFFFFF"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                bottomMargin: 1
            }
            height: 3
            radius: 2
            color: "#33FFFFFF"

            Rectangle {
                width: parent.width * Math.min(1, Math.max(0, progress))
                height: parent.height
                radius: 2
                color: accent
            }
        }
    }

    Column {
        id: panel
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            leftMargin: 16
            rightMargin: 16
            topMargin: 52
        }
        spacing: 8
        opacity: root.expanded ? 1 : 0
        enabled: root.expanded
        Behavior on opacity { NumberAnimation { duration: 140 } }

        Text {
            text: "Workrave timers"
            color: "#7DD3A7"
            font.pixelSize: 11
            font.bold: true
        }

        TimerRow {
            label: "Micro pause"
            time: root._microTime
            progress: root._microProgress
            accent: "#7DD3A7"
        }

        TimerRow {
            label: "Rest break"
            time: root._restTime
            progress: root._restProgress
            accent: "#8AB4F8"
        }

        TimerRow {
            label: "Daily limit"
            time: root._dailyTime
            progress: root._dailyProgress
            accent: "#F0A87D"
        }

        Row {
            spacing: 8
            anchors.horizontalCenter: parent.horizontalCenter
            topPadding: 4

            Button {
                implicitWidth: 88
                text: "Break now"
                onClicked: root.takeBreak()
                background: Rectangle { radius: 8; color: "#7DD3A7" }
                contentItem: Text {
                    text: "Break now"
                    color: "#101216"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                implicitWidth: 70
                text: "Later"
                onClicked: root.postpone()
                background: Rectangle { radius: 8; color: "#35404A" }
                contentItem: Text {
                    text: "Later"
                    color: "#E6EAF0"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                implicitWidth: 78
                text: "Settings"
                onClicked: root.settingsRequested()
                background: Rectangle { radius: 8; color: "#35404A" }
                contentItem: Text {
                    text: "Settings"
                    color: "#E6EAF0"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Click the pill again to collapse"
            color: "#6E7883"
            font.pixelSize: 10
        }
    }

    // Small close/collapse handle in the top-right while expanded.
    MouseArea {
        anchors { top: parent.top; right: parent.right; margins: 8 }
        width: 22
        height: 22
        visible: root.expanded
        z: 30
        cursorShape: Qt.PointingHandCursor
        onClicked: root.expanded = false

        Rectangle {
            anchors.fill: parent
            radius: 11
            color: "#33FFFFFF"
            Text {
                anchors.centerIn: parent
                text: "×"
                color: "#E6EAF0"
                font.pixelSize: 14
            }
        }
    }
}
