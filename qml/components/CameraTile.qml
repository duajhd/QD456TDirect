import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import AppItems 1.0
import "../theme"
Rectangle {
    id: root

    property int cameraIndex: 0
    property var cameraVm: null
    property var roiManager: null
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

            Item {
                id: roiOverlay
                anchors.fill: parent

                property real imageWidth: root.cameraVm ? root.cameraVm.width : 0
                property real imageHeight: root.cameraVm ? root.cameraVm.height : 0
                property real displayScale: imageWidth > 0 && imageHeight > 0
                                            ? Math.min(width / imageWidth, height / imageHeight)
                                            : 1
                property real displayWidth: imageWidth * displayScale
                property real displayHeight: imageHeight * displayScale
                property real imageOffsetX: (width - displayWidth) * 0.5
                property real imageOffsetY: (height - displayHeight) * 0.5

                function imageToViewX(x) { return imageOffsetX + x * displayScale }
                function imageToViewY(y) { return imageOffsetY + y * displayScale }
                function imageToViewW(w) { return w * displayScale }
                function imageToViewH(h) { return h * displayScale }

                Repeater {
                    model: root.roiManager ? root.roiManager.roiItems : []

                    delegate: Item {
                        required property var modelData

                        property bool isDetectionRoi: modelData &&
                                                      (modelData.roiType === "TopROI" ||
                                                       modelData.roiType === "DownROI")
                        property real viewCenterX: roiOverlay.imageToViewX(modelData ? modelData.centerX : 0)
                        property real viewCenterY: roiOverlay.imageToViewY(modelData ? modelData.centerY : 0)
                        property real viewWidth: roiOverlay.imageToViewW(modelData ? modelData.roiWidth : 0)
                        property real viewHeight: roiOverlay.imageToViewH(modelData ? modelData.roiHeight : 0)

                        visible: isDetectionRoi && roiOverlay.imageWidth > 0 && roiOverlay.imageHeight > 0
                        x: viewCenterX - viewWidth / 2
                        y: viewCenterY - viewHeight / 2
                        width: viewWidth
                        height: viewHeight
                        rotation: modelData ? modelData.angle : 0
                        transformOrigin: Item.Center

                        Rectangle {
                            anchors.fill: parent
                            color: "transparent"
                            border.width: 2
                            border.color: modelData ? modelData.color : "#1677ff"
                        }

                        Text {
                            x: 4
                            y: 4
                            text: modelData ? modelData.roiType : ""
                            color: "#ffffff"
                            font.pixelSize: 11
                            font.bold: true
                            style: Text.Outline
                            styleColor: "#111827"
                        }
                    }
                }
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

            Rectangle {
                anchors.left: parent.left
                anchors.bottom: parent.bottom
                anchors.margins: 10
                radius: 6
                color: "#99000000"
                implicitWidth: frameCountText.implicitWidth + 18
                implicitHeight: 26

                Text {
                    id: frameCountText
                    anchors.centerIn: parent
                    text: "Frame: " + (root.cameraVm ? root.cameraVm.rejectFrameCount : 0)
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Rectangle {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
                radius: 6
                color: "#99000000"
                implicitWidth: algorithmCountText.implicitWidth + 18
                implicitHeight: 26

                Text {
                    id: algorithmCountText
                    anchors.centerIn: parent
                    text: "Algo: " + (root.cameraVm ? root.cameraVm.algorithmFrameCount : 0)
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                }
            }

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                radius: 6
                color: "#99000000"
                implicitWidth: regionCountText.implicitWidth + 18
                implicitHeight: 26

                Text {
                    id: regionCountText
                    anchors.centerIn: parent
                    text: "TC:" + (root.cameraVm ? root.cameraVm.topConnectedCount : -1)
                          + " DC:" + (root.cameraVm ? root.cameraVm.downConnectedCount : -1)
                    color: "white"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }
    }
}
