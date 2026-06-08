#ifndef ALGORITHM_H
#define ALGORITHM_H

#include "DetectionRoiConfig.h"

#include <HalconCpp.h>
#include <QVariantList>

namespace VisionAlgorithm {

enum class DirectionResult
{
    NotFound = 0,
    Normal   = 1,
    Reject   = 2
};

struct DirectionOverlayRegions
{
    QVariantList topDiffRuns;
    QVariantList downDiffRuns;
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
                                          DirectionOverlayRegions* overlayRegions = nullptr);

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
                                          DirectionOverlayRegions* overlayRegions = nullptr);

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
                                          DirectionOverlayRegions* overlayRegions = nullptr);

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
                                          DirectionOverlayRegions* overlayRegions = nullptr);

}

#endif // ALGORITHM_H
