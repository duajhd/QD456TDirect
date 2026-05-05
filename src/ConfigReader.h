#ifndef CONFIGREADER_H
#define CONFIGREADER_H
#include "CameraConfig.h"
#include <QString>
#include <QVector>

class ConfigReader
{
public:
    static bool loadCameraConfig(const QString& filePath,
                                 QVector<CameraConfig>& cameras,
                                 QString* errorString = nullptr);
};
#endif // CONFIGREADER_H
