#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <HalconCpp.h>

namespace VisionAlgorithm {

enum class DirectionResult
{
    NotFound = 0,
    Normal   = 1,
    Reject   = 2
};

DirectionResult DirectionRecognize(const HalconCpp::HObject& image,
                                   const HalconCpp::HObject& topRectangle,
                                   const HalconCpp::HObject& downRectangle,
                                   int offsetX,
                                   int offsetY,
                                   int offsetXDown,
                                   int offsetYDown,
                                   int dropThres);

}

#endif // ALGORITHM_H
