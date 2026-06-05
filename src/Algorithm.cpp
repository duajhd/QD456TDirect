#include "Algorithm.h"
#include <cmath>

using namespace HalconCpp;

namespace VisionAlgorithm {

DirectionResult DirectionRecognize(const HObject& image,
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
                                   int dropThres)
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

        // 只有上下都找到 1 个目标才继续
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

        if (std::abs(topDeviation[0].D() - downDeviation[0].D()) > dropThres) {
            return DirectionResult::Reject;
        }

        return DirectionResult::Normal;
    }
    catch (const HException&) {
        return DirectionResult::NotFound;
    }
}

}
