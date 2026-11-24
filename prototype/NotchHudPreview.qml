// Standalone preview for NotchHud.qml — no Workrave build required.
// Run with: qmlscene NotchHudPreview.qml   (or: qml NotchHudPreview.qml)

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: win
    width: 900
    height: 420
    visible: true
    title: "Workledge — Notch HUD prototype"
    color: "#101216"

    // Fake desktop background so the translucent pill is visible.
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1D2430" }
            GradientStop { position: 1.0; color: "#0B0D11" }
        }
    }

    Text {
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 92
        }
        text: "Simple notches, zero nonsense."
        color: "#4E5966"
        font.pixelSize: 13
    }

    NotchHud {
        id: hud
        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 24
        }

        onTakeBreak: status.text = "signal: takeBreak()"
        onPostpone: status.text = "signal: postpone()"
        onSettingsRequested: status.text = "signal: settingsRequested()"
    }

    Text {
        id: status
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 28
        }
        text: "Click the pill to expand"
        color: "#7DD3A7"
        font.pixelSize: 12
    }
}
