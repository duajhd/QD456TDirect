import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import "../components"
import "../theme"

Page {
    id: root
    property var stackViewRef: null
    property var cameraOptions: [
        cameraOptionText(0),
        cameraOptionText(1),
        cameraOptionText(2),
        cameraOptionText(3)
    ]

    function cameraOptionText(index) {
        var camera = mainViewModel.getCamera(index)
        if (!camera)
            return "相机" + (index + 1)
        return camera.name + " - " + camera.serialNumber
    }

    function syncCameraParameterFields() {
        var index = cameraSelectBox.currentIndex
        if (index < 0)
            index = 0

        var camera = mainViewModel.getCamera(index)
        if (!camera)
            return

        exposureField.text = String(camera.exposureTime)
        gainField.text = String(camera.gain)
    }

    function applyCameraSettings() {
        var index = cameraSelectBox.currentIndex
        if (index < 0)
            index = 0

        var exposure = Number(exposureField.text)
        var gain = Number(gainField.text)
        var dropThres = Math.round(Number(rejectThresholdField.text))

        if (isNaN(exposure) || exposure <= 0 || isNaN(gain) || gain < 0 || isNaN(dropThres) || dropThres < 0) {
            console.warn("Invalid camera settings", exposureField.text, gainField.text, rejectThresholdField.text)
            return
        }

        var ok = mainViewModel.SetCameraParameter(index, exposure, gain, dropThres)
        console.log("SetCameraParameter", "camera =", index, "exposure =", exposure, "gain =", gain, "dropThres =", dropThres, "ok =", ok)
    }

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
        if (!roiManager) {
            console.warn("Opening ROI editor with null roiManager for camera", index)
            return
        }

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
            property real offsetBaseX: 512
            property real offsetBaseY: 512
            property real sidePanelWidth: 340
            property bool pageAlive: true

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

            function fieldNumber(field, fallbackValue) {
                var value = Number(field.text)
                return isNaN(value) ? fallbackValue : value
            }

            function halconParamMap() {
                return {
                    topThresholdMin: fieldNumber(topThresholdMinField, 0),
                    topThresholdMax: fieldNumber(topThresholdMaxField, 240),
                    downThresholdMin: fieldNumber(downThresholdMinField, 0),
                    downThresholdMax: fieldNumber(downThresholdMaxField, 240),
                    topRatioMin: fieldNumber(topRatioMinField, 0),
                    topRatioMax: fieldNumber(topRatioMaxField, 3),
                    topHeightMin: fieldNumber(topHeightMinField, 1),
                    topHeightMax: fieldNumber(topHeightMaxField, 20),
                    topWidthMin: fieldNumber(topWidthMinField, 30),
                    topWidthMax: fieldNumber(topWidthMaxField, 300),
                    downRatioMin: fieldNumber(downRatioMinField, 3),
                    downRatioMax: fieldNumber(downRatioMaxField, 40),
                    downHeightMin: fieldNumber(downHeightMinField, 90),
                    downHeightMax: fieldNumber(downHeightMaxField, 2000),
                    downWidthMin: fieldNumber(downWidthMinField, 1),
                    downWidthMax: fieldNumber(downWidthMaxField, 15)
                }
            }

            function setHalconParamFields(params) {
                if (!params)
                    return
                if (!topThresholdMinField || !topThresholdMaxField ||
                    !downThresholdMinField || !downThresholdMaxField ||
                    !topRatioMinField || !topRatioMaxField ||
                    !topHeightMinField || !topHeightMaxField ||
                    !topWidthMinField || !topWidthMaxField ||
                    !downRatioMinField || !downRatioMaxField ||
                    !downHeightMinField || !downHeightMaxField ||
                    !downWidthMinField || !downWidthMaxField)
                    return

                topThresholdMinField.text = String(params.topThresholdMin !== undefined ? params.topThresholdMin : 0)
                topThresholdMaxField.text = String(params.topThresholdMax !== undefined ? params.topThresholdMax : 240)
                downThresholdMinField.text = String(params.downThresholdMin !== undefined ? params.downThresholdMin : 0)
                downThresholdMaxField.text = String(params.downThresholdMax !== undefined ? params.downThresholdMax : 240)
                topRatioMinField.text = String(params.topRatioMin !== undefined ? params.topRatioMin : 0)
                topRatioMaxField.text = String(params.topRatioMax !== undefined ? params.topRatioMax : 3)
                topHeightMinField.text = String(params.topHeightMin !== undefined ? params.topHeightMin : 1)
                topHeightMaxField.text = String(params.topHeightMax !== undefined ? params.topHeightMax : 20)
                topWidthMinField.text = String(params.topWidthMin !== undefined ? params.topWidthMin : 30)
                topWidthMaxField.text = String(params.topWidthMax !== undefined ? params.topWidthMax : 300)
                downRatioMinField.text = String(params.downRatioMin !== undefined ? params.downRatioMin : 3)
                downRatioMaxField.text = String(params.downRatioMax !== undefined ? params.downRatioMax : 40)
                downHeightMinField.text = String(params.downHeightMin !== undefined ? params.downHeightMin : 90)
                downHeightMaxField.text = String(params.downHeightMax !== undefined ? params.downHeightMax : 2000)
                downWidthMinField.text = String(params.downWidthMin !== undefined ? params.downWidthMin : 1)
                downWidthMaxField.text = String(params.downWidthMax !== undefined ? params.downWidthMax : 15)
            }

            function createTopOffsetRoi() {
                if (!roiManager)
                    return

                var angle = fieldNumber(topRotationField, 0)
                var topOffsetX = fieldNumber(topOffsetXField, 0)
                var topOffsetY = fieldNumber(topOffsetYField, 0)
                var topWidth = Math.max(1, fieldNumber(topWidthField, 160))
                var topHeight = Math.max(1, fieldNumber(topHeightField, 100))
                var topCircleRadius = Math.max(1, fieldNumber(topCircleRadiusField, Math.min(topWidth, topHeight) * 0.5))

                var roi = roiManager.AddOffsetRoi("TopOffsetROI",
                                                  offsetBaseX,
                                                  offsetBaseY,
                                                  topOffsetX,
                                                  topOffsetY,
                                                  topWidth,
                                                  topHeight,
                                                  topCircleRadius,
                                                  angle,
                                                  "#1677ff")
                var applied = mainViewModel.ApplyRoiConfig(roiPage.cameraIndex)
                var runs = roi && roi.regionRuns ? roi.regionRuns.length : -1
                var report = "TopROI偏移报告 runs=" + runs
                           + " center=(" + (roi ? roi.centerX.toFixed(1) : "-") + ", " + (roi ? roi.centerY.toFixed(1) : "-") + ")"
                           + " size=(" + topWidth + ", " + topHeight + ")"
                           + " radius=" + topCircleRadius
                           + " angle=" + angle
                roiManager.AppendRoiDebugReport(report)
                statusLabel.text = applied ? report + " 已应用" : report + " 未应用"
            }

            function createDownOffsetRoi() {
                if (!roiManager)
                    return

                var angle = fieldNumber(downRotationField, 0)
                var downOffsetX = fieldNumber(downOffsetXField, 0)
                var downOffsetY = fieldNumber(downOffsetYField, 0)
                var downWidth = Math.max(1, fieldNumber(downWidthField, 160))
                var downHeight = Math.max(1, fieldNumber(downHeightField, 100))
                var downCircleRadius = Math.max(1, fieldNumber(downCircleRadiusField, Math.min(downWidth, downHeight) * 0.5))

                var roi = roiManager.AddOffsetRoi("DownOffsetROI",
                                                  offsetBaseX,
                                                  offsetBaseY,
                                                  downOffsetX,
                                                  downOffsetY,
                                                  downWidth,
                                                  downHeight,
                                                  downCircleRadius,
                                                  angle,
                                                  "#22c55e")
                var applied = mainViewModel.ApplyRoiConfig(roiPage.cameraIndex)
                var runs = roi && roi.regionRuns ? roi.regionRuns.length : -1
                var report = "DownROI偏移报告 runs=" + runs
                           + " center=(" + (roi ? roi.centerX.toFixed(1) : "-") + ", " + (roi ? roi.centerY.toFixed(1) : "-") + ")"
                           + " size=(" + downWidth + ", " + downHeight + ")"
                           + " radius=" + downCircleRadius
                           + " angle=" + angle
                roiManager.AppendRoiDebugReport(report)
                statusLabel.text = applied ? report + " 已应用" : report + " 未应用"
            }

            function createOffsetRoi() {
                if (!roiManager)
                    return

                var topAngle = fieldNumber(topRotationField, 0)
                var downAngle = fieldNumber(downRotationField, 0)
                var topOffsetX = fieldNumber(topOffsetXField, 0)
                var topOffsetY = fieldNumber(topOffsetYField, 0)
                var topWidth = Math.max(1, fieldNumber(topWidthField, 160))
                var topHeight = Math.max(1, fieldNumber(topHeightField, 100))
                var topCircleRadius = Math.max(1, fieldNumber(topCircleRadiusField, Math.min(topWidth, topHeight) * 0.5))
                var downOffsetX = fieldNumber(downOffsetXField, 0)
                var downOffsetY = fieldNumber(downOffsetYField, 0)
                var downWidth = Math.max(1, fieldNumber(downWidthField, 160))
                var downHeight = Math.max(1, fieldNumber(downHeightField, 100))
                var downCircleRadius = Math.max(1, fieldNumber(downCircleRadiusField, Math.min(downWidth, downHeight) * 0.5))

                roiManager.AddOffsetRoi("TopOffsetROI",
                                        offsetBaseX,
                                        offsetBaseY,
                                        topOffsetX,
                                        topOffsetY,
                                        topWidth,
                                        topHeight,
                                        topCircleRadius,
                                        topAngle,
                                        "#1677ff")
                roiManager.AddOffsetRoi("DownOffsetROI",
                                        offsetBaseX,
                                        offsetBaseY,
                                        downOffsetX,
                                        downOffsetY,
                                        downWidth,
                                        downHeight,
                                        downCircleRadius,
                                        downAngle,
                                        "#22c55e")
                statusLabel.text = "已创建TopROI/DownROI偏移组"
            }

            function executeHalcon() {
                if (!roiManager)
                    return

                roiManager.SetHalconParams(halconParamMap())
                var result = roiManager.ExecuteHalcon(localFilePath(roiPage.imageSource))
                if (result.ok) {
                    roiPage.offsetBaseX = result.baseX
                    roiPage.offsetBaseY = result.baseY
                }

                statusLabel.text = result.message
            }

            Component.onCompleted: {
                if (roiManager)
                    Qt.callLater(function() {
                        if (pageAlive && roiManager)
                            setHalconParamFields(roiManager.GetHalconParams())
                    })
            }

            Component.onDestruction: {
                pageAlive = false
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
                    Layout.preferredWidth: roiPage.sidePanelWidth
                    Layout.minimumWidth: 280
                    Layout.maximumWidth: 520
                    Layout.fillHeight: true
                    color: "#2b2b2b"

                    ScrollView {
                        anchors.fill: parent
                        anchors.margins: 14
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        ScrollBar.vertical.policy: ScrollBar.AsNeeded

                        ColumnLayout {
                            width: parent.availableWidth
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

                        Label {
                            text: "偏移创建"
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        Label {
                            text: "基准点: (" + roiPage.offsetBaseX + ", " + roiPage.offsetBaseY + ")"
                            color: "#cbd5e1"
                            font.pixelSize: 12
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 3
                            columnSpacing: 8
                            rowSpacing: 8

                            Label {
                                text: "field"
                                color: "#94a3b8"
                                font.pixelSize: 12
                            }
                            Label {
                                text: "TopROI"
                                color: "#60a5fa"
                                font.pixelSize: 12
                                font.bold: true
                            }
                            Label {
                                text: "DownROI"
                                color: "#4ade80"
                                font.pixelSize: 12
                                font.bold: true
                            }

                            Label {
                                text: "offsetX"
                                color: "#d1d5db"
                                font.pixelSize: 12
                            }
                            TextField {
                                id: topOffsetXField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "0"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                            TextField {
                                id: downOffsetXField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "0"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }

                            Label {
                                text: "offsetY"
                                color: "#d1d5db"
                                font.pixelSize: 12
                            }
                            TextField {
                                id: topOffsetYField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "0"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                            TextField {
                                id: downOffsetYField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "0"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }

                            Label {
                                text: "width"
                                color: "#d1d5db"
                                font.pixelSize: 12
                            }
                            TextField {
                                id: topWidthField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "160"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                            TextField {
                                id: downWidthField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "160"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }

                            Label {
                                text: "height"
                                color: "#d1d5db"
                                font.pixelSize: 12
                            }
                            TextField {
                                id: topHeightField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "100"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                            TextField {
                                id: downHeightField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "100"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }

                            Label {
                                text: "circle radius"
                                color: "#d1d5db"
                                font.pixelSize: 12
                            }
                            TextField {
                                id: topCircleRadiusField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "50"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                            TextField {
                                id: downCircleRadiusField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "50"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }

                            Label {
                                text: "rotation"
                                color: "#d1d5db"
                                font.pixelSize: 12
                            }
                            TextField {
                                id: topRotationField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "0"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                            TextField {
                                id: downRotationField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 30
                                text: "0"
                                selectByMouse: true
                                inputMethodHints: Qt.ImhFormattedNumbersOnly
                            }
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "创建TopROI偏移"
                            onClicked: roiPage.createTopOffsetRoi()
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "创建DownROI偏移"
                            onClicked: roiPage.createDownOffsetRoi()
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "执行halcond"
                            onClicked: roiPage.executeHalcon()
                        }

                        Label {
                            text: "HALCON Params"
                            color: "#ffffff"
                            font.pixelSize: 14
                            font.bold: true
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 3
                            columnSpacing: 8
                            rowSpacing: 6

                            Label { text: "param"; color: "#94a3b8"; font.pixelSize: 12 }
                            Label { text: "min"; color: "#94a3b8"; font.pixelSize: 12 }
                            Label { text: "max"; color: "#94a3b8"; font.pixelSize: 12 }

                            Label { text: "Top th"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: topThresholdMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "0"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: topThresholdMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "240"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Down th"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: downThresholdMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "0"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: downThresholdMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "240"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Top ratio"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: topRatioMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "0"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: topRatioMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "3"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Top h"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: topHeightMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "1"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: topHeightMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "20"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Top w"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: topWidthMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "30"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: topWidthMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "300"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Down ratio"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: downRatioMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "3"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: downRatioMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "40"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Down h"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: downHeightMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "90"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: downHeightMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "2000"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }

                            Label { text: "Down w"; color: "#d1d5db"; font.pixelSize: 12 }
                            TextField { id: downWidthMinField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "1"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
                            TextField { id: downWidthMaxField; Layout.fillWidth: true; Layout.preferredHeight: 28; text: "15"; selectByMouse: true; inputMethodHints: Qt.ImhFormattedNumbersOnly }
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

                    Rectangle {
                        width: 6
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        color: resizeMouseArea.containsMouse || resizeMouseArea.pressed ? "#64748b" : "transparent"

                        MouseArea {
                            id: resizeMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.SizeHorCursor

                            property real pressX: 0
                            property real startWidth: 0

                            onPressed: function(mouse) {
                                pressX = mouse.x
                                startWidth = roiPage.sidePanelWidth
                            }

                            onPositionChanged: function(mouse) {
                                if (!pressed)
                                    return

                                var nextWidth = startWidth + mouse.x - pressX
                                roiPage.sidePanelWidth = Math.max(280, Math.min(520, nextWidth))
                            }
                        }
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
                    roiPage.roiManager.SetHalconParams(roiPage.halconParamMap())
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
                    if (roiPage.roiManager.LoadFromJson(path)) {
                        roiPage.setHalconParamFields(roiPage.roiManager.GetHalconParams())
                        var applied = mainViewModel.ApplyRoiConfig(roiPage.cameraIndex)
                        statusLabel.text = applied
                                         ? "已读取并应用: " + path
                                         : "已读取，应用失败: " + path
                    } else {
                        statusLabel.text = "读取失败: " + path
                    }
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

            Button {
                id: globalRunButton
                Layout.preferredWidth: 92
                Layout.preferredHeight: 34
                text: mainViewModel.isRunning ? "停止" : "启动"
                padding: 0

                background: Rectangle {
                    radius: 8
                    color: mainViewModel.isRunning
                           ? (globalRunButton.down ? "#a8071a" : globalRunButton.hovered ? "#ff7875" : "#ff4d4f")
                           : (globalRunButton.down ? "#237804" : globalRunButton.hovered ? "#73d13d" : "#52c41a")
                }

                contentItem: Text {
                    text: globalRunButton.text
                    color: "#ffffff"
                    font.pixelSize: 14
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: mainViewModel.ToggleDetect()
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

                        model: root.cameraOptions
                        Component.onCompleted: root.syncCameraParameterFields()
                        onCurrentIndexChanged: root.syncCameraParameterFields()

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
                        text: "500"
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
                        text: "15"
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
                        text: "27"
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

                        onClicked: root.applyCameraSettings()
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
