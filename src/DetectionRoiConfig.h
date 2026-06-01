#ifndef DETECTIONROICONFIG_H
#define DETECTIONROICONFIG_H

struct DetectionRoiShape
{
    double centerX = 0.0;
    double centerY = 0.0;
    double width = 0.0;
    double height = 0.0;
    double angle = 0.0;
    int offsetX = 0;
    int offsetY = 0;
    double offsetRotation = 0.0;
};

struct DetectionAlgorithmParams
{
    double topThresholdMin = 0.0;
    double topThresholdMax = 170.0;
    double downThresholdMin = 0.0;
    double downThresholdMax = 170.0;

    double topRatioMin = 0.0;
    double topRatioMax = 3.0;
    double topHeightMin = 1.0;
    double topHeightMax = 20.0;
    double topWidthMin = 30.0;
    double topWidthMax = 300.0;

    double downRatioMin = 3.0;
    double downRatioMax = 40.0;
    double downHeightMin = 90.0;
    double downHeightMax = 2000.0;
    double downWidthMin = 1.0;
    double downWidthMax = 15.0;

    double inspectAngleDeg = 10.0;
    double inspectTopColumnOffset = 365.0;
    double inspectRectHalfWidth = 90.0;
    double inspectRectHalfHeight = 25.0;
};

struct DetectionRoiConfig
{
    bool valid = false;
    DetectionRoiShape top;
    DetectionRoiShape down;
    DetectionAlgorithmParams algorithmParams;
    int dropThres = 27;
};

#endif // DETECTIONROICONFIG_H
