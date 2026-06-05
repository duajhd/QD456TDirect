import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AppItems 1.0
import "../theme"
Rectangle {
    id: root

    property int cameraIndex: 0
    property var cameraVm: null
    property var stackViewRef: null

    radius: 12
    color: "#ffffff"
    border.color: "#e5e7eb"
    border.width: 1
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: cameraVm ? cameraVm.name : ("相机 " + (cameraIndex + 1))
                font.pixelSize: 17
                font.bold: true
                color: "#262626"
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                width: 10
                height: 10
                radius: 5
                color: "#52c41a"
            }

            Label {
                text: cameraVm ? cameraVm.statusText : "未连接"
                font.pixelSize: 13
                color: "#8c8c8c"
            }

            Button {
                text: "ROI编辑"
                font.pixelSize: 13

                background: Rectangle {
                    radius: 8
                    color: parent.down ? "#0958d9" : "#1677ff"
                }

                contentItem: Text {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: 13
                }

                onClicked: {
                    if (!stackViewRef || !cameraVm)
                        return

                    console.warn("Use the Dashboard toolbar ROI editor button for this build.")
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: "#111827"
            clip: true
            border.color: "#1f2937"

            CameraFrameItem {
                anchors.fill: parent
                cameraVm: root.cameraVm
            }


            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.margins: 10
                radius: 6
                color: "#6622c55e"
                implicitWidth: 52
                implicitHeight: 26

                Text {
                    anchors.centerIn: parent
                    text: "LIVE"
                    color: "white"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }
    }
}
