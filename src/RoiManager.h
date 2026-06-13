#ifndef ROIMANAGER_H
#define ROIMANAGER_H

#endif // ROIMANAGER_H
#pragma once

#include <QObject>
#include <QQmlListProperty>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QByteArray>
#include <QVariantMap>
#include "DetectionRoiConfig.h"
#include "RoiData.h"

class RoiManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<RoiData> roiItems READ GetRoiItems NOTIFY RoiListChanged)
    Q_PROPERTY(QQmlListProperty<RoiData> offsetRoiItems READ GetOffsetRoiItems NOTIFY OffsetRoiListChanged)
    Q_PROPERTY(QVariantMap halconParams READ GetHalconParams WRITE SetHalconParams NOTIFY HalconParamsChanged)

public:
    explicit RoiManager(QObject* parent = nullptr);
    ~RoiManager();

    QQmlListProperty<RoiData> GetRoiItems();
    QQmlListProperty<RoiData> GetOffsetRoiItems();

    Q_INVOKABLE RoiData* AddRoi(const QString& roiType,
                                double centerX,
                                double centerY,
                                double roiWidth,
                                double roiHeight,
                                double angle,
                                const QString& color);
    Q_INVOKABLE RoiData* AddOffsetRoi(const QString& roiType,
                                      double baseX,
                                      double baseY,
                                      double offsetX,
                                      double offsetY,
                                      double width,
                                      double height,
                                      double circleRadius,
                                      double angle,
                                      const QString& color);

    Q_INVOKABLE void RemoveRoi(const QString& roiId);
    Q_INVOKABLE void RemoveRoisByType(const QString& roiType);
    Q_INVOKABLE void ClearAllRois();
    Q_INVOKABLE RoiData* GetRoiById(const QString& roiId);
    Q_INVOKABLE RoiData* GetLastOffsetRoiByType(const QString& roiType) const;

    Q_INVOKABLE void SelectOnly(const QString& roiId);
    Q_INVOKABLE void UnselectAll();

    Q_INVOKABLE bool SaveToJson(const QString& filePath);
    Q_INVOKABLE bool LoadFromJson(const QString& filePath);
    Q_INVOKABLE bool LoadImageFrame(const QString& imagePath);
    Q_INVOKABLE QVariantMap ExecuteHalcon(const QString& imagePath);
    Q_INVOKABLE QVariantMap GetHalconParams() const;
    Q_INVOKABLE void SetHalconParams(const QVariantMap& params);
    Q_INVOKABLE void AppendRoiDebugReport(const QString& message) const;
    int DropThres() const;
    void SetDropThres(int dropThres);
    DetectionAlgorithmParams GetAlgorithmParams() const;
    void SetAlgorithmParams(const DetectionAlgorithmParams& params);

    bool BuildDetectionConfig(DetectionRoiConfig* config) const;

signals:
    void RoiListChanged();
    void OffsetRoiListChanged();
    void HalconParamsChanged();

private:
    QList<RoiData*> m_roiList;
    QList<RoiData*> m_offsetRoiList;
    QVariantMap m_halconParams;
    int m_dropThres = 27;
    QByteArray m_loadedImageFrame;
    QString m_loadedImagePath;
    int m_loadedImageWidth = 0;
    int m_loadedImageHeight = 0;

    QString GenerateRoiId() const;
    RoiData* CreateRoi(const QString& roiType,
                       double centerX,
                       double centerY,
                       double roiWidth,
                       double roiHeight,
                       double angle,
                       const QString& color);
    RoiData* TakeRoiAt(QList<RoiData*>* roiList, int index);
    void DisposeRois(const QList<RoiData*>& rois);
    void ClearRoiStorage(bool notify);
    void RemoveOffsetRoisByType(const QString& roiType);

    static qsizetype RoiListCount(QQmlListProperty<RoiData>* property);
    static RoiData* RoiListAt(QQmlListProperty<RoiData>* property, qsizetype index);
};
