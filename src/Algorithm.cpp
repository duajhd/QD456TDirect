#include "Algorithm.h"
#include <QDir>
#include <QString>
#include <atomic>
#include <cmath>

using namespace HalconCpp;

namespace VisionAlgorithm {

namespace {

std::atomic_int g_camera3CropImageIndex { 0 };

void SaveCamera3CropImages(const HObject& topReducedDomain,
                           const HObject& downReducedDomain)
{
    QDir imageDir(QDir::current().absoluteFilePath(QStringLiteral("images")));
    if (!imageDir.exists() && !imageDir.mkpath(QStringLiteral("."))) {
        return;
    }

    const int index = g_camera3CropImageIndex.fetch_add(1, std::memory_order_relaxed) + 1;
    const QString topPath = imageDir.absoluteFilePath(QString("TopCropImage_%1.bmp").arg(index));
    const QString downPath = imageDir.absoluteFilePath(QString("DownCropImage_%1.bmp").arg(index));

    HObject topCropImage;
    HObject downCropImage;
    CropDomain(topReducedDomain, &topCropImage);
    CropDomain(downReducedDomain, &downCropImage);
    WriteImage(topCropImage, "bmp", 0, topPath.toLocal8Bit().constData());
    WriteImage(downCropImage, "bmp", 0, downPath.toLocal8Bit().constData());
}

QVariantList RegionRunsFromHObject(const HObject& region)
{
    QVariantList runs;
    HTuple rows;
    HTuple columnsBegin;
    HTuple columnsEnd;
    GetRegionRuns(region, &rows, &columnsBegin, &columnsEnd);
    const Hlong count = rows.Length();
    runs.reserve(static_cast<qsizetype>(count));
    for (Hlong i = 0; i < count; ++i) {
        QVariantList run;
        run << rows[i].I() << columnsBegin[i].I() << columnsEnd[i].I();
        runs.append(run);
    }
    return runs;
}

}
DirectionResult DirectionRecognizeCamera1(const HObject& image,
                                          const HObject& topRectangle,
                                          const HObject& downRectangle,
                                          int offsetX,
                                          int offsetY,
                                          int offsetXDown,
                                          int offsetYDown,
                                          double topOffsetCircleRadius,
                                          double downOffsetCircleRadius,
                                          double topOffsetRotationDeg,
                                          double downOffsetRotationDeg,
                                          const DetectionAlgorithmParams& params,
                                          int dropThres,
                                          DirectionOverlayRegions* overlayRegions)
{
    Q_UNUSED(overlayRegions);

    try {
        HObject topReducedDomain, downReducedDomain;
        HObject topRegion, downRegion;
        HObject topConnectedRegion, downConnectedRegion;
        HObject topSelectedRegion, downSelectedRegion;
        HObject genTopRegion, genDownRegion;
        HObject topCircle, downCircle;
        HObject topDiff, downDiff;

        HTuple topNumber, downNumber;
        HTuple topArea, topRow, topColumn;
        HTuple downArea, downRow, downColumn;
        HTuple meanTop, meanDown;
        HTuple topDeviation, downDeviation;

        ReduceDomain(image, topRectangle, &topReducedDomain);
        ReduceDomain(image, downRectangle, &downReducedDomain);

        Threshold(topReducedDomain, &topRegion, params.topThresholdMin, params.topThresholdMax);
        Threshold(downReducedDomain, &downRegion, params.downThresholdMin, params.downThresholdMax);

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.topRatioMin).Append(params.topHeightMin).Append(params.topWidthMin),
                    HTuple(params.topRatioMax).Append(params.topHeightMax).Append(params.topWidthMax));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.downRatioMin).Append(params.downHeightMin).Append(params.downWidthMin),
                    HTuple(params.downRatioMax).Append(params.downHeightMax).Append(params.downWidthMax));

        CountObj(topSelectedRegion, &topNumber);
        CountObj(downSelectedRegion, &downNumber);

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;

        GenRectangle2(&genTopRegion,
                      refRow + offsetX,
                      refCol + offsetY + params.inspectTopColumnOffset,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetXDown,
                      refCol + offsetYDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetX,
                  refCol + offsetY + params.inspectTopColumnOffset,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetXDown,
                  refCol + offsetYDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        if (std::abs(meanTop[0].D() - meanDown[0].D()) <= dropThres) {
            return DirectionResult::Reject;
        }

        return DirectionResult::Normal;
    }
    catch (const HException&) {
        return DirectionResult::NotFound;
    }
}
DirectionResult DirectionRecognizeCamera2(const HObject& image,
                                          const HObject& topRectangle,
                                          const HObject& downRectangle,
                                          int offsetX,
                                          int offsetY,
                                          int offsetXDown,
                                          int offsetYDown,
                                          double topOffsetCircleRadius,
                                          double downOffsetCircleRadius,
                                          double topOffsetRotationDeg,
                                          double downOffsetRotationDeg,
                                          const DetectionAlgorithmParams& params,
                                          int dropThres,
                                          DirectionOverlayRegions* overlayRegions)
{
    Q_UNUSED(overlayRegions);

    try {
        HObject topReducedDomain, downReducedDomain;
        HObject topRegion, downRegion;
        HObject topConnectedRegion, downConnectedRegion;
        HObject topSelectedRegion, downSelectedRegion;
        HObject genTopRegion, genDownRegion;
        HObject topCircle, downCircle;
        HObject topDiff, downDiff;

        HTuple topNumber, downNumber;
        HTuple topArea, topRow, topColumn;
        HTuple downArea, downRow, downColumn;
        HTuple meanTop, meanDown;
        HTuple topDeviation, downDeviation;

        ReduceDomain(image, topRectangle, &topReducedDomain);
        ReduceDomain(image, downRectangle, &downReducedDomain);

        Threshold(topReducedDomain, &topRegion, params.topThresholdMin, params.topThresholdMax);
        Threshold(downReducedDomain, &downRegion, params.downThresholdMin, params.downThresholdMax);

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.topRatioMin).Append(params.topHeightMin).Append(params.topWidthMin),
                    HTuple(params.topRatioMax).Append(params.topHeightMax).Append(params.topWidthMax));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.downRatioMin).Append(params.downHeightMin).Append(params.downWidthMin),
                    HTuple(params.downRatioMax).Append(params.downHeightMax).Append(params.downWidthMax));

        CountObj(topSelectedRegion, &topNumber);
        CountObj(downSelectedRegion, &downNumber);

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;

        GenRectangle2(&genTopRegion,
                      refRow + offsetX,
                      refCol + offsetY + params.inspectTopColumnOffset,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetXDown,
                      refCol + offsetYDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetX,
                  refCol + offsetY + params.inspectTopColumnOffset,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetXDown,
                  refCol + offsetYDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        if (std::abs(meanTop[0].D() - meanDown[0].D()) <= dropThres) {
            return DirectionResult::Reject;
        }

        return DirectionResult::Normal;
    }
    catch (const HException&) {
        return DirectionResult::NotFound;
    }
}
DirectionResult DirectionRecognizeCamera3(const HObject& image,
                                          const HObject& topRectangle,
                                          const HObject& downRectangle,
                                          int offsetX,
                                          int offsetY,
                                          int offsetXDown,
                                          int offsetYDown,
                                          double topOffsetCircleRadius,
                                          double downOffsetCircleRadius,
                                          double topOffsetRotationDeg,
                                          double downOffsetRotationDeg,
                                          const DetectionAlgorithmParams& params,
                                          int dropThres,
                                          DirectionOverlayRegions* overlayRegions)
{
    if (overlayRegions) {
        overlayRegions->topDiffRuns.clear();
        overlayRegions->downDiffRuns.clear();
    }

    try {
        HObject topReducedDomain, downReducedDomain;
        HObject topRegion, downRegion;
        HObject topConnectedRegion, downConnectedRegion;
        HObject topSelectedRegion, downSelectedRegion;
        HObject genTopRegion, genDownRegion;
        HObject topCircle, downCircle;
        HObject topDiff, downDiff;

        HTuple topNumber, downNumber;
        HTuple topArea, topRow, topColumn;
        HTuple downArea, downRow, downColumn;
        HTuple meanTop, meanDown;
        HTuple topDeviation, downDeviation;
        ReduceDomain(image, topRectangle, &topReducedDomain);
        ReduceDomain(image, downRectangle, &downReducedDomain);



     //   WriteImage(topCropImage,"bmp",0,"D:\\zhijain\\QT456TDirect\\build\\Desktop_Qt_6_8_3_MSVC2022_64bit-Debug\\t.bmp");


        Threshold(topReducedDomain, &topRegion, params.topThresholdMin, params.topThresholdMax);
        Threshold(downReducedDomain, &downRegion, params.downThresholdMin, params.downThresholdMax);

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.topRatioMin).Append(params.topHeightMin).Append(params.topWidthMin),
                    HTuple(params.topRatioMax).Append(params.topHeightMax).Append(params.topWidthMax));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.downRatioMin).Append(params.downHeightMin).Append(params.downWidthMin),
                    HTuple(params.downRatioMax).Append(params.downHeightMax).Append(params.downWidthMax));

        CountObj(topSelectedRegion, &topNumber);
        CountObj(downSelectedRegion, &downNumber);

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;

        GenRectangle2(&genTopRegion,
                      refRow + offsetX,
                      refCol + offsetY + params.inspectTopColumnOffset,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetXDown,
                      refCol + offsetYDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetX,
                  refCol + offsetY + params.inspectTopColumnOffset,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetXDown,
                  refCol + offsetYDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        if (std::abs(meanTop[0].D() - meanDown[0].D()) <= dropThres) {
            if (overlayRegions) {
                overlayRegions->topDiffRuns = RegionRunsFromHObject(topDiff);
                overlayRegions->downDiffRuns = RegionRunsFromHObject(downDiff);
            }
            return DirectionResult::Reject;
        }

        return DirectionResult::Normal;
    }
    catch (const HException&) {
        return DirectionResult::NotFound;
    }
}
DirectionResult DirectionRecognizeCamera4(const HObject& image,
                                          const HObject& topRectangle,
                                          const HObject& downRectangle,
                                          int offsetX,
                                          int offsetY,
                                          int offsetXDown,
                                          int offsetYDown,
                                          double topOffsetCircleRadius,
                                          double downOffsetCircleRadius,
                                          double topOffsetRotationDeg,
                                          double downOffsetRotationDeg,
                                          const DetectionAlgorithmParams& params,
                                          int dropThres,
                                          DirectionOverlayRegions* overlayRegions)
{
    Q_UNUSED(overlayRegions);

    try {
        HObject topReducedDomain, downReducedDomain;
        HObject topRegion, downRegion;
        HObject topConnectedRegion, downConnectedRegion;
        HObject topSelectedRegion, downSelectedRegion;
        HObject genTopRegion, genDownRegion;
        HObject topCircle, downCircle;
        HObject topDiff, downDiff;

        HTuple topNumber, downNumber;
        HTuple topArea, topRow, topColumn;
        HTuple downArea, downRow, downColumn;
        HTuple meanTop, meanDown;
        HTuple topDeviation, downDeviation;

        ReduceDomain(image, topRectangle, &topReducedDomain);
        ReduceDomain(image, downRectangle, &downReducedDomain);

        Threshold(topReducedDomain, &topRegion, params.topThresholdMin, params.topThresholdMax);
        Threshold(downReducedDomain, &downRegion, params.downThresholdMin, params.downThresholdMax);

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.topRatioMin).Append(params.topHeightMin).Append(params.topWidthMin),
                    HTuple(params.topRatioMax).Append(params.topHeightMax).Append(params.topWidthMax));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(params.downRatioMin).Append(params.downHeightMin).Append(params.downWidthMin),
                    HTuple(params.downRatioMax).Append(params.downHeightMax).Append(params.downWidthMax));

        CountObj(topSelectedRegion, &topNumber);
        CountObj(downSelectedRegion, &downNumber);

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;

        GenRectangle2(&genTopRegion,
                      refRow + offsetX,
                      refCol + offsetY + params.inspectTopColumnOffset,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetXDown,
                      refCol + offsetYDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetX,
                  refCol + offsetY + params.inspectTopColumnOffset,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetXDown,
                  refCol + offsetYDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        if (std::abs(meanTop[0].D() - meanDown[0].D()) <= dropThres) {
            return DirectionResult::Reject;
        }

        return DirectionResult::Normal;
    }
    catch (const HException&) {
        return DirectionResult::NotFound;
    }
}
}
