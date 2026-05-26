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
                                   int dropThres)
{
    try {
        HObject topReducedDomain, downReducedDomain;
        HObject topRegion, downRegion;
        HObject topConnectedRegion, downConnectedRegion;
        HObject topSelectedRegion, downSelectedRegion;
        HObject genTopRegion, genDownRegion;

        HTuple topNumber, downNumber;
        HTuple topArea, topRow, topColumn;
        HTuple downArea, downRow, downColumn;
        HTuple meanTop, meanDown;
        HTuple topDeviation, downDeviation;

        ReduceDomain(image, topRectangle, &topReducedDomain);
        ReduceDomain(image, downRectangle, &downReducedDomain);

        Threshold(topReducedDomain, &topRegion, 0, 170);
        Threshold(downReducedDomain, &downRegion, 0, 170);

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(0).Append(1).Append(30),
                    HTuple(3).Append(20).Append(300));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(3).Append(90).Append(1),
                    HTuple(40).Append(2000).Append(15));

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



        // 如果你这里不是 10 度而是水平矩形，应改成 0.0
        const double phi = 10.0 * 3.14159265358979323846 / 180.0;

        GenRectangle2(&genTopRegion,  refRow + offsetX,  refCol+offsetY + 365.0, phi, 90.0, 25.0);
        GenRectangle2(&genDownRegion, refRow + offsetXDown, refCol + offsetYDown, phi, 90.0, 25.0);

        Intensity(genTopRegion, image, &meanTop, &topDeviation);
        Intensity(genDownRegion, image, &meanDown, &downDeviation);

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
