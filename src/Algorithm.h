#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "DetectionRoiConfig.h"

#include <HalconCpp.h>

namespace VisionAlgorithm {

enum class DirectionResult
{
    NotFound = 0,
    Normal   = 1,
    Reject   = 2
};

struct DirectionDebugInfo
{
    int topConnectedCount = -1;
    int downConnectedCount = -1;
    int topSelectedCount = -1;
    int downSelectedCount = -1;
    double topRegionArea = -1.0;
    double downRegionArea = -1.0;
};

DirectionResult DirectionRecognizeCamera1(const HalconCpp::HObject& image,
                                          const HalconCpp::HObject& topRectangle,
                                          const HalconCpp::HObject& downRectangle,
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
                                          DirectionDebugInfo* debugInfo = nullptr);

DirectionResult DirectionRecognizeCamera2(const HalconCpp::HObject& image,
                                          const HalconCpp::HObject& topRectangle,
                                          const HalconCpp::HObject& downRectangle,
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
                                          DirectionDebugInfo* debugInfo = nullptr);

DirectionResult DirectionRecognizeCamera3(const HalconCpp::HObject& image,
                                          const HalconCpp::HObject& topRectangle,
                                          const HalconCpp::HObject& downRectangle,
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
                                          DirectionDebugInfo* debugInfo = nullptr);

DirectionResult DirectionRecognizeCamera4(const HalconCpp::HObject& image,
                                          const HalconCpp::HObject& topRectangle,
                                          const HalconCpp::HObject& downRectangle,
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
                                          DirectionDebugInfo* debugInfo = nullptr);

}

#endif // ALGORITHM_H
