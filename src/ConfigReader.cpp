#include "ConfigReader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

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
