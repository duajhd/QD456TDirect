import QtQuick
import QtQuick.Controls
import MyApp 1.0
Item {
    id: root
      clip: true
       z: 100

      property string imageSource: ""
      property var roiManager: null

      property bool imageReady: imageItem.status === Image.Ready

      property real imageOriginalWidth: (imageReady && imageItem.sourceSize.width > 0)
                                        ? imageItem.sourceSize.width
                                        : width

      property real imageOriginalHeight: (imageReady && imageItem.sourceSize.height > 0)
                                         ? imageItem.sourceSize.height
                                         : height

      property real displayScale: imageReady
                                  ? Math.min(width / imageOriginalWidth,
                                             height / imageOriginalHeight)
                                  : 1.0

      property real displayWidth: imageReady ? imageOriginalWidth * displayScale : width
      property real displayHeight: imageReady ? imageOriginalHeight * displayScale : height
      property real offsetX: (width - displayWidth) * 0.5
      property real offsetY: (height - displayHeight) * 0.5

      function imageToViewX(x) { return offsetX + x * displayScale }
      function imageToViewY(y) { return offsetY + y * displayScale }
      function viewToImageX(x) { return (x - offsetX) / displayScale }
      function viewToImageY(y) { return (y - offsetY) / displayScale }
      function imageToViewW(w) { return w * displayScale }
      function imageToViewH(h) { return h * displayScale }

      Rectangle {
          anchors.fill: parent
          color: "#1E1E1E"
      }

      Image {
          id: imageItem
          anchors.centerIn: parent
          source: root.imageSource
          fillMode: Image.PreserveAspectFit
          width: root.displayWidth
          height: root.displayHeight
          cache: false
          asynchronous: false

          onStatusChanged: {
              console.log("image status =", status,
                          "source =", source,
                          "sourceSize =", sourceSize.width, sourceSize.height)
          }
      }

      Text {
          anchors.centerIn: parent
          visible: imageItem.status === Image.Error
          text: "图像加载失败"
          color: "#C0C0C0"
          font.pixelSize: 24
      }
      MouseArea {
          anchors.fill: parent
          acceptedButtons: Qt.LeftButton
          onPressed: {
              if (root.roiManager)
                  root.roiManager.UnselectAll()
          }
      }
      Repeater {
          model: root.roiManager ? root.roiManager.roiList : []

            delegate: RoiItem {
                required property var modelData

                imageCanvas: root
                roiManager: root.roiManager
                roiData: modelData

                Component.onCompleted: {
                    console.log("RoiItem delegate created, modelData =", modelData)
                }
            }
      }

}
