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
                                          bool rejectAll,
                                          double* rejectDiff)
{
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

        if (rejectAll && (topNumber[0].I() == 1 || downNumber[0].I() == 1)) {
            if (rejectDiff) {
                *rejectDiff = 0.0;
            }
            return DirectionResult::Reject;
        }

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        HTuple imageWidth, imageHeight;
        GetImageSize(image, &imageWidth, &imageHeight);
        SetSystem("width", imageWidth);
        SetSystem("height", imageHeight);

        GenRectangle2(&genTopRegion,
                      refRow + offsetY,
                      refCol + offsetX,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetYDown,
                      refCol + offsetXDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetY,
                  refCol + offsetX,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetYDown,
                  refCol + offsetXDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        const double diffValue = std::abs(meanTop[0].D() - meanDown[0].D());
        if (diffValue <= dropThres) {
            if (rejectDiff) {
                *rejectDiff = diffValue;
            }
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
                                          bool rejectAll,
                                          double* rejectDiff)
{
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

        if (rejectAll && (topNumber[0].I() == 1 || downNumber[0].I() == 1)) {
            if (rejectDiff) {
                *rejectDiff = 0.0;
            }
            return DirectionResult::Reject;
        }

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        HTuple imageWidth, imageHeight;
        GetImageSize(image, &imageWidth, &imageHeight);
        SetSystem("width", imageWidth);
        SetSystem("height", imageHeight);

        GenRectangle2(&genTopRegion,
                      refRow + offsetY,
                      refCol + offsetX,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetYDown,
                      refCol + offsetXDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetY,
                  refCol + offsetX,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetYDown,
                  refCol + offsetXDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        const double diffValue = std::abs(meanTop[0].D() - meanDown[0].D());
        if (diffValue <= dropThres) {
            if (rejectDiff) {
                *rejectDiff = diffValue;
            }
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
                                          bool rejectAll,
                                          double* rejectDiff)
{
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

        if (rejectAll && (topNumber[0].I() == 1 || downNumber[0].I() == 1)) {
            if (rejectDiff) {
                *rejectDiff = 0.0;
            }
            return DirectionResult::Reject;
        }

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        HTuple imageWidth, imageHeight;
        GetImageSize(image, &imageWidth, &imageHeight);
        SetSystem("width", imageWidth);
        SetSystem("height", imageHeight);

        GenRectangle2(&genTopRegion,
                      refRow + offsetY,
                      refCol + offsetX,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetYDown,
                      refCol + offsetXDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetY,
                  refCol + offsetX,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetYDown,
                  refCol + offsetXDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        const double diffValue = std::abs(meanTop[0].D() - meanDown[0].D());
        if (diffValue <= dropThres) {
            if (rejectDiff) {
                *rejectDiff = diffValue;
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
                                          bool rejectAll,
                                          double* rejectDiff)
{
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

        if (rejectAll && (topNumber[0].I() == 1 || downNumber[0].I() == 1)) {
            if (rejectDiff) {
                *rejectDiff = 0.0;
            }
            return DirectionResult::Reject;
        }

        if (topNumber[0].I() != 1 || downNumber[0].I() != 1) {
            return DirectionResult::NotFound;
        }

        AreaCenter(topSelectedRegion, &topArea, &topRow, &topColumn);
        AreaCenter(downSelectedRegion, &downArea, &downRow, &downColumn);

        const double refRow = topRow[0].D();
        const double refCol = downColumn[0].D();
        const double topPhi = topOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        const double downPhi = downOffsetRotationDeg * 3.14159265358979323846 / 180.0;
        HTuple imageWidth, imageHeight;
        GetImageSize(image, &imageWidth, &imageHeight);
        SetSystem("width", imageWidth);
        SetSystem("height", imageHeight);

        GenRectangle2(&genTopRegion,
                      refRow + offsetY,
                      refCol + offsetX,
                      topPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);
        GenRectangle2(&genDownRegion,
                      refRow + offsetYDown,
                      refCol + offsetXDown,
                      downPhi,
                      params.inspectRectHalfWidth,
                      params.inspectRectHalfHeight);

        GenCircle(&topCircle,
                  refRow + offsetY,
                  refCol + offsetX,
                  topOffsetCircleRadius);
        GenCircle(&downCircle,
                  refRow + offsetYDown,
                  refCol + offsetXDown,
                  downOffsetCircleRadius);

        Difference(genTopRegion, topCircle, &topDiff);
        Difference(genDownRegion, downCircle, &downDiff);

        Intensity(topDiff, image, &meanTop, &topDeviation);
        Intensity(downDiff, image, &meanDown, &downDeviation);

        const double diffValue = std::abs(meanTop[0].D() - meanDown[0].D());
        if (diffValue <= dropThres) {
            if (rejectDiff) {
                *rejectDiff = diffValue;
            }
            return DirectionResult::Reject;
        }

        return DirectionResult::Normal;
    }
    catch (const HException&) {
        return DirectionResult::NotFound;
    }
}
}
