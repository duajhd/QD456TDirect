#ifndef ROIDATA_H
#define ROIDATA_H

#endif // ROIDATA_H
#pragma once

#include <QObject>
#include <QString>
#include <QColor>

class RoiData : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString roiId READ GetRoiId WRITE SetRoiId NOTIFY RoiChanged)
    Q_PROPERTY(QString roiName READ GetRoiName WRITE SetRoiName NOTIFY RoiChanged)
    Q_PROPERTY(QString roiType READ GetRoiType WRITE SetRoiType NOTIFY RoiChanged)

    Q_PROPERTY(double centerX READ GetCenterX WRITE SetCenterX NOTIFY RoiChanged)
    Q_PROPERTY(double centerY READ GetCenterY WRITE SetCenterY NOTIFY RoiChanged)
    Q_PROPERTY(double roiWidth READ GetRoiWidth WRITE SetRoiWidth NOTIFY RoiChanged)
    Q_PROPERTY(double roiHeight READ GetRoiHeight WRITE SetRoiHeight NOTIFY RoiChanged)
    Q_PROPERTY(double angle READ GetAngle WRITE SetAngle NOTIFY RoiChanged)

    Q_PROPERTY(QString color READ GetColor WRITE SetColor NOTIFY RoiChanged)
    Q_PROPERTY(bool selected READ GetSelected WRITE SetSelected NOTIFY RoiChanged)

public:
    explicit RoiData(QObject* parent = nullptr);

    QString GetRoiId() const;
    void SetRoiId(const QString& value);

    QString GetRoiName() const;
    void SetRoiName(const QString& value);

    QString GetRoiType() const;
    void SetRoiType(const QString& value);

    double GetCenterX() const;
    void SetCenterX(double value);

    double GetCenterY() const;
    void SetCenterY(double value);

    double GetRoiWidth() const;
    void SetRoiWidth(double value);

    double GetRoiHeight() const;
    void SetRoiHeight(double value);

    double GetAngle() const;
    void SetAngle(double value);

    QString GetColor() const;
    void SetColor(const QString& value);

    bool GetSelected() const;
    void SetSelected(bool value);

signals:
    void RoiChanged();

private:
    QString m_roiId;
    QString m_roiName;
    QString m_roiType;

    double m_centerX;
    double m_centerY;
    double m_roiWidth;
    double m_roiHeight;
    double m_angle;

    QString m_color;
    bool m_selected;
};
