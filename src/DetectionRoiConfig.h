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
};

struct DetectionRoiConfig
{
    bool valid = false;
    DetectionRoiShape top;
    DetectionRoiShape down;
    int dropThres = 27;
};

#endif // DETECTIONROICONFIG_H
