#ifndef CONFIGREADER_H
#define CONFIGREADER_H
#include "CameraConfig.h"
#include "DetectionRoiConfig.h"
#include "GPIOController.h"
#include <QString>
#include <QVector>

class ConfigReader
{
public:
    static bool loadCameraConfig(const QString& filePath,
                                 QVector<CameraConfig>& cameras,
                                 QString* errorString = nullptr);
    static bool loadAlgorithmParams(const QString& filePath,
                                    DetectionAlgorithmParams& params,
                                    QString* errorString = nullptr);
    static bool loadGPIOConfig(const QString& filePath,
                               QVector<GPIOControllerConfig>& controllers,
                               QString* errorString = nullptr);
};
#endif // CONFIGREADER_H
