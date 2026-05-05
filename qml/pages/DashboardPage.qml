import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../components"
import "../theme"

Page {
    id: root
    property var stackViewRef: null

    function openRoiEditor() {
        console.log("openRoiEditor clicked", "stackViewRef =", stackViewRef)

        if (!stackViewRef) {
            console.warn("Cannot open ROI editor: stackViewRef is null")
            return
        }

        var index = cameraSelectBox.currentIndex
        if (index < 0)
            index = 0

        var roiManager = mainViewModel.getRoiManager(index)
        if (!roiManager)
            console.warn("Opening ROI editor with null roiManager for camera", index)

        stackViewRef.push(roiEditorPageComponent, {
            cameraIndex: index,
            cameraVm: mainViewModel.getCamera(index),
            roiManager: roiManager,
            stackViewRef: stackViewRef
        })
    }

    Component {
        id: roiEditorPageComponent

        Page {
            id: roiPage

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
                roiPage.imageSource = fileUrl.toString()
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
                            if (roiPage.stackViewRef)
                                roiPage.stackViewRef.pop()
                        }
                    }

                    Label {
                        text: "ROI编辑 - 相机" + (roiPage.cameraIndex + 1)
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
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "加载图片"
                            onClicked: imageDialog.open()
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "添加"
                            onClicked: roiPage.addRoi()
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
                                if (roiPage.roiManager) {
                                    roiPage.roiManager.ClearAllRois()
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
                    imageSource: roiPage.imageSource
                    roiManager: roiPage.roiManager
                }
            }

            FileDialog {
                id: imageDialog
                title: "加载图片"
                fileMode: FileDialog.OpenFile
                nameFilters: ["图像文件 (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)", "所有文件 (*)"]
                onAccepted: roiPage.loadImage(selectedFile)
            }

            FileDialog {
                id: saveDialog
                title: "保存ROI"
                fileMode: FileDialog.SaveFile
                nameFilters: ["JSON文件 (*.json)"]
                onAccepted: {
                    if (!roiPage.roiManager)
                        return
                    var path = roiPage.jsonPath(selectedFile)
                    statusLabel.text = roiPage.roiManager.SaveToJson(path)
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
                    if (!roiPage.roiManager)
                        return
                    var path = roiPage.localFilePath(selectedFile)
                    statusLabel.text = roiPage.roiManager.LoadFromJson(path)
                                     ? "已读取: " + path
                                     : "读取失败: " + path
                }
            }
        }
    }

    background: Rectangle {
        color: "#f5f7fa"
    }

    header: Rectangle {
        height: 64
        color: "#ffffff"
        border.color: "#ececec"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18

            Label {
                text: "四相机检测系统"
                font.pixelSize: 24
                font.bold: true
                color: "#262626"
            }

            Rectangle {
                width: 1
                height: 24
                color: "#e5e7eb"
            }

            Label {
                text: "Phase 2 - High Performance Viewer + ROI Editor"
                font.pixelSize: 14
                color: "#8c8c8c"
            }

            Item {
                Layout.fillWidth: true
            }

            Rectangle {
                radius: 8
                color: "#f0f5ff"
                border.color: "#d6e4ff"
                implicitWidth: 190
                implicitHeight: 34

                Row {
                    anchors.centerIn: parent
                    spacing: 8

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: "#52c41a"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "SceneGraph Rendering"
                        color: "#1677ff"
                        font.pixelSize: 13
                        font.bold: true
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 88
            radius: 10
            color: "#ffffff"
            border.color: "#f0f0f0"
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 18

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 4

                    Label {
                        text: "相机参数设置"
                        font.pixelSize: 17
                        font.bold: true
                        color: "#262626"
                    }

                    Label {
                        text: "配置曝光、增益、剔除阈值"
                        font.pixelSize: 12
                        color: "#8c8c8c"
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 1
                    Layout.preferredHeight: 36
                    Layout.alignment: Qt.AlignVCenter
                    color: "#f0f0f0"
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Label {
                        text: "选择相机"
                        font.pixelSize: 12
                        color: "#595959"
                    }

                    ComboBox {
                        id: cameraSelectBox
                        Layout.preferredWidth: 220
                        Layout.preferredHeight: 32

                        model: [
                            "相机1 - SN001",
                            "相机2 - SN002",
                            "相机3 - SN003",
                            "相机4 - SN004"
                        ]

                        background: Rectangle {
                            radius: 6
                            color: "#ffffff"
                            border.width: 1
                            border.color: cameraSelectBox.hovered ? "#4096ff" : "#d9d9d9"
                        }

                        contentItem: Text {
                            text: cameraSelectBox.displayText
                            color: "#262626"
                            font.pixelSize: 14
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: 12
                            rightPadding: 30
                            elide: Text.ElideRight
                        }

                        indicator: Text {
                            x: cameraSelectBox.width - width - 12
                            y: (cameraSelectBox.height - height) / 2
                            text: "⌄"
                            color: "#8c8c8c"
                            font.pixelSize: 16
                        }
                    }
                }

                function antInputBg(control) {
                    return control.activeFocus ? "#1677ff"
                         : control.hovered ? "#4096ff"
                         : "#d9d9d9"
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Label {
                        text: "曝光"
                        font.pixelSize: 12
                        color: "#595959"
                    }

                    TextField {
                        id: exposureField
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        text: "5000"
                        selectByMouse: true
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        padding: 0
                        leftPadding: 11
                        rightPadding: 11
                        topPadding: 0
                        bottomPadding: 0
                        color: "#262626"
                        font.pixelSize: 14
                        verticalAlignment: TextInput.AlignVCenter

                        background: Rectangle {
                            radius: 6
                            color: "#ffffff"
                            border.width: 1
                            border.color: root.antInputBg(exposureField)
                        }
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Label {
                        text: "增益"
                        font.pixelSize: 12
                        color: "#595959"
                    }

                    TextField {
                        id: gainField
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        text: "1.0"
                        selectByMouse: true
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        padding: 0
                        leftPadding: 11
                        rightPadding: 11
                        topPadding: 0
                        bottomPadding: 0
                        color: "#262626"
                        font.pixelSize: 14
                        verticalAlignment: TextInput.AlignVCenter

                        background: Rectangle {
                            radius: 6
                            color: "#ffffff"
                            border.width: 1
                            border.color: root.antInputBg(gainField)
                        }
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Label {
                        text: "剔除阈值"
                        font.pixelSize: 12
                        color: "#595959"
                    }

                    TextField {
                        id: rejectThresholdField
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 32
                        text: "0.80"
                        selectByMouse: true
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                        padding: 0
                        leftPadding: 11
                        rightPadding: 11
                        topPadding: 0
                        bottomPadding: 0
                        color: "#262626"
                        font.pixelSize: 14
                        verticalAlignment: TextInput.AlignVCenter

                        background: Rectangle {
                            radius: 6
                            color: "#ffffff"
                            border.width: 1
                            border.color: root.antInputBg(rejectThresholdField)
                        }
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Label {
                        text: ""
                        font.pixelSize: 12
                    }

                    Button {
                        id: confirmButton
                        Layout.preferredWidth: 104
                        Layout.preferredHeight: 32
                        text: "确认设置"
                        padding: 0

                        background: Rectangle {
                            radius: 6
                            color: confirmButton.down ? "#0958d9"
                                 : confirmButton.hovered ? "#4096ff"
                                 : "#1677ff"
                        }

                        contentItem: Text {
                            text: confirmButton.text
                            color: "#ffffff"
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                ColumnLayout {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 6

                    Label {
                        text: ""
                        font.pixelSize: 12
                    }

                    Button {
                        id: editRoiButton
                        Layout.preferredWidth: 96
                        Layout.preferredHeight: 32
                        text: "编辑ROI"
                        padding: 0

                        background: Rectangle {
                            radius: 6
                            color: editRoiButton.down ? "#389e0d"
                                 : editRoiButton.hovered ? "#73d13d"
                                 : "#52c41a"
                        }

                        contentItem: Text {
                            text: editRoiButton.text
                            color: "#ffffff"
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: root.openRoiEditor()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Item {
                    id: rejectAllCheckBox
                    property bool checked: false

                    Layout.preferredWidth: 86
                    Layout.preferredHeight: 32
                    Layout.alignment: Qt.AlignVCenter

                    Row {
                        anchors.centerIn: parent
                        spacing: 8

                        Rectangle {
                            id: checkboxBox
                            width: 16
                            height: 16
                            radius: 4
                            anchors.verticalCenter: parent.verticalCenter

                            color: rejectAllCheckBox.checked ? "#1677ff" : "#ffffff"
                            border.width: 1
                            border.color: rejectAllCheckBox.checked ? "#1677ff"
                                       : checkboxMouseArea.containsMouse ? "#4096ff"
                                       : "#d9d9d9"

                            Text {
                                anchors.centerIn: parent
                                text: "✓"
                                visible: rejectAllCheckBox.checked
                                color: "#ffffff"
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }

                        Text {
                            text: "全部剔除"
                            color: "#262626"
                            font.pixelSize: 14
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    MouseArea {
                        id: checkboxMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            rejectAllCheckBox.checked = !rejectAllCheckBox.checked
                        }
                    }
                }
            }
        }
        GridLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            rowSpacing: 14
            columnSpacing: 14

            Repeater {
                model: 4

                delegate: CameraTile {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    cameraIndex: index
                    cameraVm: mainViewModel.getCamera(index)
                    stackViewRef: root.stackViewRef
                }
            }
        }
    }
}
