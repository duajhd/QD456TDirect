import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property var imageCanvas
    required property var roiManager
    property var roiData: null

    visible: roiData !== null

    property real safeCenterX: roiData ? roiData.centerX : 0
    property real safeCenterY: roiData ? roiData.centerY : 0
    property real safeWidth: roiData ? roiData.roiWidth : 0
    property real safeHeight: roiData ? roiData.roiHeight : 0
    property real safeAngle: roiData ? roiData.angle : 0
    property string safeColor: roiData ? roiData.color : "#00AEEF"
    property bool safeSelected: roiData ? roiData.selected : false
    property string safeName: roiData ? roiData.roiName : ""
    property string safeId: roiData ? roiData.roiId : ""

    property real viewCenterX: imageCanvas ? imageCanvas.imageToViewX(safeCenterX) : 0
    property real viewCenterY: imageCanvas ? imageCanvas.imageToViewY(safeCenterY) : 0
    property real viewWidth: imageCanvas ? imageCanvas.imageToViewW(safeWidth) : 0
    property real viewHeight: imageCanvas ? imageCanvas.imageToViewH(safeHeight) : 0

    x: viewCenterX - viewWidth / 2
    y: viewCenterY - viewHeight / 2
    width: viewWidth
    height: viewHeight
    rotation: safeAngle
    transformOrigin: Item.Center

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: safeSelected ? 3 : 2
        border.color: safeColor
    }

    Text {
        visible: safeSelected
        text: safeName + "  angle=" + safeAngle.toFixed(1)
        color: "white"
        font.pixelSize: 14
        x: 4
        y: 4
    }

    Rectangle {
        visible: safeSelected
        x: parent.width - 8
        y: parent.height - 8
        width: 16
        height: 16
        radius: 8
        color: "#FFFFFF"
        border.width: 2
        border.color: safeColor
    }

    Rectangle {
        visible: safeSelected
        x: parent.width / 2 - 6
        y: -36
        width: 12
        height: 12
        radius: 6
        color: "#FFD54F"
        border.width: 2
        border.color: "#333333"
    }

    Canvas {
        anchors.fill: parent
        visible: safeSelected
        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = "#FFD54F"
            ctx.lineWidth = 2
            ctx.beginPath()
            ctx.moveTo(width / 2, 0)
            ctx.lineTo(width / 2, -24)
            ctx.stroke()
        }
    }

    MouseArea {
        visible: safeSelected
        enabled: root.roiData !== null
        x: parent.width / 2 - 20
        y: -50
        width: 40
        height: 40
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        cursorShape: Qt.CrossCursor

        property real centerSceneX: 0
        property real centerSceneY: 0

        onPressed: function(mouse) {
            if (!root.roiData)
                return

            const centerPoint = root.mapToItem(imageCanvas, root.width / 2, root.height / 2)
            centerSceneX = centerPoint.x
            centerSceneY = centerPoint.y
        }

        onPositionChanged: function(mouse) {
            if (!pressed || !root.roiData)
                return

            const p = mapToItem(imageCanvas, mouse.x, mouse.y)
            const angleRad = Math.atan2(p.y - centerSceneY, p.x - centerSceneX)
            const angleDeg = angleRad * 180.0 / Math.PI + 90.0
            root.roiData.angle = angleDeg
        }
    }
    MouseArea {
        visible: safeSelected
        enabled: root.roiData !== null
        x: parent.width - 20
        y: parent.height - 20
        width: 40
        height: 40
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        cursorShape: Qt.SizeFDiagCursor

        property real startWidth: 0
        property real startHeight: 0
        property real startMouseSceneX: 0
        property real startMouseSceneY: 0

        onPressed: function(mouse) {
            if (!root.roiData)
                return

            startWidth = root.roiData.roiWidth
            startHeight = root.roiData.roiHeight

            const p = mapToItem(imageCanvas, mouse.x, mouse.y)
            startMouseSceneX = p.x
            startMouseSceneY = p.y
        }

        onPositionChanged: function(mouse) {
            if (!pressed || !root.roiData)
                return

            const p = mapToItem(imageCanvas, mouse.x, mouse.y)
            const dxView = p.x - startMouseSceneX
            const dyView = p.y - startMouseSceneY

            const rad = root.roiData.angle * Math.PI / 180.0
            const localDx =  Math.cos(rad) * dxView + Math.sin(rad) * dyView
            const localDy = -Math.sin(rad) * dxView + Math.cos(rad) * dyView

            let newWidth = startWidth + localDx / imageCanvas.displayScale
            let newHeight = startHeight + localDy / imageCanvas.displayScale

            if (newWidth < 10) newWidth = 10
            if (newHeight < 10) newHeight = 10

            root.roiData.roiWidth = newWidth
            root.roiData.roiHeight = newHeight
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.roiData !== null
        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        cursorShape: Qt.SizeAllCursor

        property real pressSceneX: 0
        property real pressSceneY: 0
        property real startCenterX: 0
        property real startCenterY: 0

        onPressed: function(mouse) {
            if (!root.roiData)
                return

            if (root.roiManager)
                root.roiManager.SelectOnly(root.safeId)

            const p = mapToItem(imageCanvas, mouse.x, mouse.y)
            pressSceneX = p.x
            pressSceneY = p.y

            startCenterX = root.roiData.centerX
            startCenterY = root.roiData.centerY
        }

        onPositionChanged: function(mouse) {
            if (!pressed || !root.roiData)
                return

            const p = mapToItem(imageCanvas, mouse.x, mouse.y)
            const dxView = p.x - pressSceneX
            const dyView = p.y - pressSceneY

            const dxImage = dxView / imageCanvas.displayScale
            const dyImage = dyView / imageCanvas.displayScale

            root.roiData.centerX = startCenterX + dxImage
            root.roiData.centerY = startCenterY + dyImage
        }
    }
}
