import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import MyApp 1.0

Window {
    id: root
    width: 1200
    height: 800
    title: "ROI编辑"
    modality: Qt.NonModal

    property string imageSource: ""
    property var roiManager: null

    function selectedRoiColor() {
        if (roiTypeBox.currentIndex < 0)
            return "#1677ff"
        return roiTypeModel.get(roiTypeBox.currentIndex).roiColor
    }

    function selectedRoiType() {
        if (roiTypeBox.currentIndex < 0)
            return "TopROI"
        return roiTypeModel.get(roiTypeBox.currentIndex).roiType
    }

    function localFilePath(fileUrl) {
        var text = decodeURIComponent(fileUrl.toString())
        if (text.indexOf("file:///") === 0)
            return text.substring(8)
        if (text.indexOf("file://") === 0)
            return text.substring(7)
        return text
    }

    function jsonPath(fileUrl) {
        var path = localFilePath(fileUrl)
        if (path.length > 0 && path.toLowerCase().slice(-5) !== ".json")
            path += ".json"
        return path
    }

    function loadImage(fileUrl) {
        root.imageSource = fileUrl.toString()
    }

    function addRoi() {
        if (!roiManager)
            return

        if (!imageCanvas.imageReady)
            return

        var cx = imageCanvas.viewToImageX(imageCanvas.width / 2)
        var cy = imageCanvas.viewToImageY(imageCanvas.height / 2)

        roiManager.AddRoi(selectedRoiType(), cx, cy, 160, 100, 0, selectedRoiColor())
    }

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

                    ComboBox {
                        id: roiTypeBox
                        width: parent.width
                        textRole: "label"

                        model: ListModel {
                            id: roiTypeModel
                            ListElement { label: "TopROI"; roiType: "TopROI"; roiColor: "#1677ff" }
                            ListElement { label: "DownROI"; roiType: "DownROI"; roiColor: "#22c55e" }
                        }
                    }

                    Button {
                        text: "加载图片"
                        onClicked: imageDialog.open()
                    }

                    Button {
                        text: "添加"
                        onClicked: root.addRoi()
                    }

                    Button {
                        text: "保存ROI"
                        onClicked: saveDialog.open()
                    }

                    Button {
                        text: "读取ROI"
                        onClicked: loadDialog.open()
                    }
                }
            }

            ImageCanvas {
                  id: imageCanvas
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: parent.width - 220
                imageSource: root.imageSource
                roiManager: root.roiManager
            }
        }
    }

    FileDialog {
        id: imageDialog
        title: "加载图片"
        fileMode: FileDialog.OpenFile
        nameFilters: ["图像文件 (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)", "所有文件 (*)"]
        onAccepted: root.loadImage(selectedFile)
    }

    FileDialog {
        id: saveDialog
        title: "保存ROI"
        fileMode: FileDialog.SaveFile
        nameFilters: ["JSON文件 (*.json)"]
        onAccepted: {
            if (root.roiManager)
                root.roiManager.SaveToJson(root.jsonPath(selectedFile))
        }
    }

    FileDialog {
        id: loadDialog
        title: "读取ROI"
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON文件 (*.json)"]
        onAccepted: {
            if (root.roiManager)
                root.roiManager.LoadFromJson(root.localFilePath(selectedFile))
        }
    }
}
