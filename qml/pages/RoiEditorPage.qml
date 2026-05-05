import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../components"

Page {
    id: root

    property int cameraIndex: 0
    property var cameraVm: null
    property var roiManager: null
    property var stackViewRef: null
    property string imageSource: ""

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
        statusLabel.text = "已加载图片: " + localFilePath(fileUrl)
    }

    function addRoi() {
        if (!roiManager)
            return

        if (!imageCanvas.imageReady) {
            statusLabel.text = "请先加载图片"
            return
        }

        var cx = imageCanvas.viewToImageX(imageCanvas.width / 2)
        var cy = imageCanvas.viewToImageY(imageCanvas.height / 2)

        roiManager.AddRoi(selectedRoiType(), cx, cy, 160, 100, 0, selectedRoiColor())
        statusLabel.text = "已添加 " + selectedRoiType()
    }

    background: Rectangle {
        color: "#202124"
    }

    header: Rectangle {
        height: 56
        color: "#ffffff"
        border.color: "#e5e7eb"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 14
            anchors.rightMargin: 14
            spacing: 10

            Button {
                text: "返回"
                onClicked: {
                    if (root.stackViewRef)
                        root.stackViewRef.pop()
                }
            }

            Label {
                text: "ROI编辑 - 相机" + (root.cameraIndex + 1)
                font.pixelSize: 18
                font.bold: true
                color: "#262626"
            }

            Item { Layout.fillWidth: true }

            Label {
                id: statusLabel
                text: ""
                font.pixelSize: 13
                color: "#595959"
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            color: "#2b2b2b"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 14
                spacing: 12

                Label {
                    text: "ROI类型"
                    color: "#ffffff"
                    font.pixelSize: 14
                    font.bold: true
                }

                ComboBox {
                    id: roiTypeBox
                    Layout.fillWidth: true
                    textRole: "label"

                    model: ListModel {
                        id: roiTypeModel
                        ListElement { label: "TopROI"; roiType: "TopROI"; roiColor: "#1677ff" }
                        ListElement { label: "DownROI"; roiType: "DownROI"; roiColor: "#22c55e" }
                    }

                    delegate: ItemDelegate {
                        width: roiTypeBox.width
                        contentItem: Row {
                            spacing: 8
                            Rectangle {
                                width: 12
                                height: 12
                                radius: 6
                                color: model.roiColor
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: model.label
                                color: "#262626"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }

                Button {
                    Layout.fillWidth: true
                    text: "加载图片"
                    onClicked: imageDialog.open()
                }

                Button {
                    Layout.fillWidth: true
                    text: "添加"
                    onClicked: root.addRoi()
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#3f3f46"
                }

                Button {
                    Layout.fillWidth: true
                    text: "保存ROI"
                    onClicked: saveDialog.open()
                }

                Button {
                    Layout.fillWidth: true
                    text: "读取ROI"
                    onClicked: loadDialog.open()
                }

                Button {
                    Layout.fillWidth: true
                    text: "清空ROI"
                    onClicked: {
                        if (root.roiManager) {
                            root.roiManager.ClearAllRois()
                            statusLabel.text = "已清空ROI"
                        }
                    }
                }

                Item { Layout.fillHeight: true }
            }
        }

        ImageCanvas {
            id: imageCanvas
            Layout.fillWidth: true
            Layout.fillHeight: true
            imageSource: root.imageSource
            roiManager: root.roiManager
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
            if (!root.roiManager)
                return
            var path = root.jsonPath(selectedFile)
            statusLabel.text = root.roiManager.SaveToJson(path)
                             ? "已保存: " + path
                             : "保存失败: " + path
        }
    }

    FileDialog {
        id: loadDialog
        title: "读取ROI"
        fileMode: FileDialog.OpenFile
        nameFilters: ["JSON文件 (*.json)"]
        onAccepted: {
            if (!root.roiManager)
                return
            var path = root.localFilePath(selectedFile)
            statusLabel.text = root.roiManager.LoadFromJson(path)
                             ? "已读取: " + path
                             : "读取失败: " + path
        }
    }
}
