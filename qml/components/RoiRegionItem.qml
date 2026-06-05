import QtQuick

Item {
    id: root

    required property var imageCanvas
    property var roiManager: null
    property var roiData: null

    anchors.fill: parent
    visible: roiData !== null

    property string safeColor: roiData ? roiData.color : "#00AEEF"
    property var safeRuns: roiData ? roiData.regionRuns : []
    property real safeCenterX: roiData ? roiData.centerX : 0
    property real safeCenterY: roiData ? roiData.centerY : 0
    property real safeWidth: roiData ? roiData.roiWidth : 0
    property real safeHeight: roiData ? roiData.roiHeight : 0
    property real safeAngle: roiData ? roiData.angle : 0
    property bool paintReportPrinted: false

    Component.onCompleted: {
        const report = "RoiRegionItem created roi=" + (roiData ? roiData.roiName : "null")
                     + " runs=" + (safeRuns ? safeRuns.length : -1)
                     + " center=(" + safeCenterX + "," + safeCenterY + ")"
                     + " size=(" + safeWidth + "," + safeHeight + ")"
                     + " angle=" + safeAngle
        console.log(report)
        if (roiManager)
            roiManager.AppendRoiDebugReport(report)
        regionCanvas.requestPaint()
    }
    onSafeRunsChanged: regionCanvas.requestPaint()
    onVisibleChanged: regionCanvas.requestPaint()
    onWidthChanged: regionCanvas.requestPaint()
    onHeightChanged: regionCanvas.requestPaint()

    Connections {
        target: root.roiData
        function onRoiChanged() {
            regionCanvas.requestPaint()
        }
    }

    Connections {
        target: root.imageCanvas
        function onDisplayScaleChanged() { regionCanvas.requestPaint() }
        function onOffsetXChanged() { regionCanvas.requestPaint() }
        function onOffsetYChanged() { regionCanvas.requestPaint() }
        function onDisplayWidthChanged() { regionCanvas.requestPaint() }
        function onDisplayHeightChanged() { regionCanvas.requestPaint() }
    }

    Canvas {
        id: regionCanvas
        anchors.fill: parent

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (!root.imageCanvas || !root.roiData)
                return

            const scale = root.imageCanvas.displayScale
            if (scale <= 0)
                return

            if (!root.paintReportPrinted) {
                root.paintReportPrinted = true
                const report = "RoiRegionItem paint roi=" + (root.roiData ? root.roiData.roiName : "null")
                             + " canvas=(" + width + "," + height + ")"
                             + " scale=" + scale
                             + " imageOffset=(" + root.imageCanvas.offsetX + "," + root.imageCanvas.offsetY + ")"
                             + " runs=" + (root.safeRuns ? root.safeRuns.length : -1)
                             + " center=(" + root.safeCenterX + "," + root.safeCenterY + ")"
                             + " size=(" + root.safeWidth + "," + root.safeHeight + ")"
                             + " angle=" + root.safeAngle
                console.log(report)
                if (root.roiManager)
                    root.roiManager.AppendRoiDebugReport(report)
            }

            ctx.fillStyle = root.safeColor
            ctx.globalAlpha = 0.35

            if (root.safeRuns && root.safeRuns.length > 0) {
                for (let i = 0; i < root.safeRuns.length; ++i) {
                    const run = root.safeRuns[i]
                    if (!run)
                        continue

                    const row = Number(run[0])
                    const colBegin = Number(run[1])
                    const colEnd = Number(run[2])
                    if (isNaN(row) || isNaN(colBegin) || isNaN(colEnd))
                        continue

                    const x = root.imageCanvas.imageToViewX(colBegin)
                    const y = root.imageCanvas.imageToViewY(row)
                    const w = Math.max(scale, (colEnd - colBegin + 1) * scale)
                    const h = Math.max(1, scale)
                    ctx.fillRect(x, y, w, h)
                }
            } else {
                ctx.save()
                ctx.translate(root.imageCanvas.imageToViewX(root.safeCenterX),
                              root.imageCanvas.imageToViewY(root.safeCenterY))
                ctx.rotate(root.safeAngle * Math.PI / 180.0)
                const rectW = root.imageCanvas.imageToViewW(root.safeWidth)
                const rectH = root.imageCanvas.imageToViewH(root.safeHeight)
                const radius = Math.min(rectW, rectH) / 2
                ctx.fillRect(-rectW / 2, -rectH / 2, rectW, rectH)
                ctx.globalCompositeOperation = "destination-out"
                ctx.beginPath()
                ctx.arc(0, 0, radius, 0, Math.PI * 2)
                ctx.fill()
                ctx.globalCompositeOperation = "source-over"
                ctx.restore()
            }

            ctx.globalAlpha = 1.0
        }
    }
}
