#include "ConfigReader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDir>
#include <QFileInfo>

namespace {

double JsonDoubleOrDefault(const QJsonObject& obj, const QString& key, double defaultValue)
{
    return obj.contains(key) ? obj.value(key).toDouble(defaultValue) : defaultValue;
}

}

bool ConfigReader::loadCameraConfig(const QString& filePath,
                                    QVector<CameraConfig>& cameras,
                                    QString* errorString)
{
    cameras.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorString) {
            *errorString = "无法打开配置文件: " + filePath;
        }
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        if (errorString) {
            *errorString = "JSON解析失败: " + parseError.errorString();
        }
        return false;
    }

    if (!doc.isObject()) {
        if (errorString) {
            *errorString = "JSON根节点必须是对象";
        }
        return false;
    }

    QJsonObject rootObj = doc.object();

    if (!rootObj.contains("cameras") || !rootObj["cameras"].isArray()) {
        if (errorString) {
            *errorString = "缺少 cameras 数组";
        }
        return false;
    }

    QJsonArray cameraArray = rootObj["cameras"].toArray();
    const QDir configDir = QFileInfo(filePath).absoluteDir();

    for (int i = 0; i < cameraArray.size(); ++i) {
        if (!cameraArray[i].isObject()) {
            if (errorString) {
                *errorString = QString("cameras[%1] 不是对象").arg(i);
            }
            return false;
        }

        QJsonObject obj = cameraArray[i].toObject();

        CameraConfig cfg;

        cfg.serialNumber = obj.value("serialNumber").toString();
        cfg.imageWidth = obj.value("imageWidth").toInt();
        cfg.imageHeight = obj.value("imageHeight").toInt();
        cfg.channel = obj.contains("channel") ? obj.value("channel").toInt(1) : 1;
        cfg.exposureTime = JsonDoubleOrDefault(obj, "exposureTime", cfg.exposureTime);
        cfg.gain = JsonDoubleOrDefault(obj, "gain", cfg.gain);
        cfg.path = obj.value("path").toString();
        if (cfg.path.isEmpty()) {
            cfg.path = QString("camera%1.json").arg(i + 1);
        }
        if (QDir::isRelativePath(cfg.path)) {
            cfg.path = configDir.absoluteFilePath(cfg.path);
        }

        if (cfg.serialNumber.isEmpty()) {
            if (errorString) {
                *errorString = QString("cameras[%1] serialNumber 为空").arg(i);
            }
            return false;
        }

        if (cfg.imageWidth <= 0 || cfg.imageHeight <= 0) {
            if (errorString) {
                *errorString = QString("cameras[%1] 图像宽高非法").arg(i);
            }
            return false;
        }

        if (cfg.channel <= 0) {
            cfg.channel = 1;
        }

        if (cfg.exposureTime <= 0.0) {
            cfg.exposureTime = 500.0;
        }

        if (cfg.gain < 0.0) {
            cfg.gain = 15.0;
        }

        cameras.append(cfg);
    }

    return true;
}

bool ConfigReader::loadAlgorithmParams(const QString& filePath,
                                       DetectionAlgorithmParams& params,
                                       QString* errorString)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorString) {
            *errorString = "Cannot open config file: " + filePath;
        }
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorString) {
            *errorString = "Algorithm params JSON parse failed: " + parseError.errorString();
        }
        return false;
    }

    const QJsonObject rootObj = doc.object();
    if (!rootObj.value("algorithm").isObject()) {
        params = DetectionAlgorithmParams();
        return true;
    }

    const QJsonObject obj = rootObj.value("algorithm").toObject();
    DetectionAlgorithmParams next;
    next.topThresholdMin = JsonDoubleOrDefault(obj, "topThresholdMin", next.topThresholdMin);
    next.topThresholdMax = JsonDoubleOrDefault(obj, "topThresholdMax", next.topThresholdMax);
    next.downThresholdMin = JsonDoubleOrDefault(obj, "downThresholdMin", next.downThresholdMin);
    next.downThresholdMax = JsonDoubleOrDefault(obj, "downThresholdMax", next.downThresholdMax);

    next.topRatioMin = JsonDoubleOrDefault(obj, "topRatioMin", next.topRatioMin);
    next.topRatioMax = JsonDoubleOrDefault(obj, "topRatioMax", next.topRatioMax);
    next.topHeightMin = JsonDoubleOrDefault(obj, "topHeightMin", next.topHeightMin);
    next.topHeightMax = JsonDoubleOrDefault(obj, "topHeightMax", next.topHeightMax);
    next.topWidthMin = JsonDoubleOrDefault(obj, "topWidthMin", next.topWidthMin);
    next.topWidthMax = JsonDoubleOrDefault(obj, "topWidthMax", next.topWidthMax);

    next.downRatioMin = JsonDoubleOrDefault(obj, "downRatioMin", next.downRatioMin);
    next.downRatioMax = JsonDoubleOrDefault(obj, "downRatioMax", next.downRatioMax);
    next.downHeightMin = JsonDoubleOrDefault(obj, "downHeightMin", next.downHeightMin);
    next.downHeightMax = JsonDoubleOrDefault(obj, "downHeightMax", next.downHeightMax);
    next.downWidthMin = JsonDoubleOrDefault(obj, "downWidthMin", next.downWidthMin);
    next.downWidthMax = JsonDoubleOrDefault(obj, "downWidthMax", next.downWidthMax);

    next.inspectAngleDeg = JsonDoubleOrDefault(obj, "inspectAngleDeg", next.inspectAngleDeg);
    next.inspectTopColumnOffset = JsonDoubleOrDefault(obj, "inspectTopColumnOffset", next.inspectTopColumnOffset);
    next.inspectRectHalfWidth = JsonDoubleOrDefault(obj, "inspectRectHalfWidth", next.inspectRectHalfWidth);
    next.inspectRectHalfHeight = JsonDoubleOrDefault(obj, "inspectRectHalfHeight", next.inspectRectHalfHeight);

    params = next;
    return true;
}

bool ConfigReader::loadGPIOConfig(const QString& filePath,
                                  QVector<GPIOControllerConfig>& controllers,
                                  QString* errorString)
{
    controllers.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorString) {
            *errorString = "Cannot open GPIO config file: " + filePath;
        }
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorString) {
            *errorString = "GPIO config JSON parse failed: " + parseError.errorString();
        }
        return false;
    }

    const QJsonObject rootObj = doc.object();
    if (!rootObj.value("controllers").isArray()) {
        if (errorString) {
            *errorString = "GPIO config missing controllers array";
        }
        return false;
    }

    const QJsonArray controllerArray = rootObj.value("controllers").toArray();
    for (int i = 0; i < controllerArray.size(); ++i) {
        if (!controllerArray.at(i).isObject()) {
            if (errorString) {
                *errorString = QString("controllers[%1] is not an object").arg(i);
            }
            return false;
        }

        const QJsonObject obj = controllerArray.at(i).toObject();
        GPIOControllerConfig config;
        config.serialNumber = obj.value("serialNumber").toInt(0);
        config.deviceIndex = obj.value("deviceIndex").toInt(i);

        if (!obj.value("routes").isArray()) {
            if (errorString) {
                *errorString = QString("controllers[%1] missing routes array").arg(i);
            }
            return false;
        }

        const QJsonArray routeArray = obj.value("routes").toArray();
        for (int routeIndex = 0; routeIndex < routeArray.size(); ++routeIndex) {
            if (!routeArray.at(routeIndex).isObject()) {
                if (errorString) {
                    *errorString = QString("controllers[%1].routes[%2] is not an object")
                                       .arg(i)
                                       .arg(routeIndex);
                }
                return false;
            }

            const QJsonObject routeObj = routeArray.at(routeIndex).toObject();
            GPIOCameraRoute route;
            route.cameraIndex = routeObj.value("cameraIndex").toInt(-1);
            route.pin = routeObj.value("pin").toInt(-1);
            if (route.cameraIndex < 0 || route.pin < 0) {
                if (errorString) {
                    *errorString = QString("controllers[%1].routes[%2] has invalid cameraIndex or pin")
                                       .arg(i)
                                       .arg(routeIndex);
                }
                return false;
            }

            config.routes.append(route);
        }

        if (config.routes.isEmpty()) {
            if (errorString) {
                *errorString = QString("controllers[%1] has no GPIO routes").arg(i);
            }
            return false;
        }

        controllers.append(config);
    }

    return !controllers.isEmpty();
}
