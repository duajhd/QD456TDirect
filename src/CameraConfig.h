#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H
#include <QString>

struct CameraConfig
{
    QString serialNumber;
    int imageWidth = 0;
    int imageHeight = 0;
    int channel = 1;   // 默认单通道灰度
};
#endif // CAMERACONFIG_H
