#include "ConfigReader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

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
