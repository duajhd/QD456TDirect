import QtQuick
import QtQuick.Window
import QtQuick.Controls
import MyApp 1.0

Window {
    id: root
    width: 1200
    height: 800
    title: "ROI Editor"
    modality: Qt.NonModal

    property string imageSource: ""

    Rectangle {
        anchors.fill: parent
        color: "#202124"

        Row {
            anchors.fill: parent

            Rectangle {
                width: 220
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                color: "#2B2B2B"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10

                    Button {
                        text: "关闭"
                        onClicked: root.close()
                    }

                    Button {
                        text: "添加蓝色ROI"
                        onClicked: {
                              console.log("Add ROI clicked")

                              var cx = imageCanvas.imageReady
                                       ? imageCanvas.viewToImageX(imageCanvas.width / 2)
                                       : imageCanvas.width / 2

                              var cy = imageCanvas.imageReady
                                       ? imageCanvas.viewToImageY(imageCanvas.height / 2)
                                       : imageCanvas.height / 2

                              var roi = roiManager.AddRoi("rect", cx, cy, 160, 100, 0, "#00AEEF")
                              console.log("roi =", roi, "cx =", cx, "cy =", cy)
                          }
                    }

                    Button {
                        text: "保存ROI"
                        onClicked: roiManager.SaveToJson("D:/roi_data.json")
                    }
                }
            }

            ImageCanvas {
                  id: imageCanvas
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width - 220
                imageSource: root.imageSource
            }
        }
    }
}
