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
#include "RoiData.h"

class RoiManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<RoiData> roiList READ GetRoiList NOTIFY RoiListChanged)

public:
    explicit RoiManager(QObject* parent = nullptr);
    ~RoiManager();

    QQmlListProperty<RoiData> GetRoiList();

    Q_INVOKABLE RoiData* AddRoi(const QString& roiType,
                                double centerX,
                                double centerY,
                                double roiWidth,
                                double roiHeight,
                                double angle,
                                const QString& color);

    Q_INVOKABLE void RemoveRoi(const QString& roiId);
    Q_INVOKABLE void ClearAllRois();
    Q_INVOKABLE RoiData* GetRoiById(const QString& roiId);

    Q_INVOKABLE void SelectOnly(const QString& roiId);
    Q_INVOKABLE void UnselectAll();

    Q_INVOKABLE bool SaveToJson(const QString& filePath);
    Q_INVOKABLE bool LoadFromJson(const QString& filePath);

signals:
    void RoiListChanged();

private:
    QList<RoiData*> m_roiList;

    QString GenerateRoiId() const;

    static qsizetype RoiCount(QQmlListProperty<RoiData>* list);
    static RoiData* RoiAt(QQmlListProperty<RoiData>* list, qsizetype index);
};
