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
    Q_PROPERTY(double offsetX READ GetOffsetX WRITE SetOffsetX NOTIFY RoiChanged)
    Q_PROPERTY(double offsetY READ GetOffsetY WRITE SetOffsetY NOTIFY RoiChanged)
    Q_PROPERTY(double width READ GetWidth WRITE SetWidth NOTIFY RoiChanged)
    Q_PROPERTY(double height READ GetHeight WRITE SetHeight NOTIFY RoiChanged)

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

    double GetOffsetX() const;
    void SetOffsetX(double value);

    double GetOffsetY() const;
    void SetOffsetY(double value);

    double GetWidth() const;
    void SetWidth(double value);

    double GetHeight() const;
    void SetHeight(double value);

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
    double m_offsetX;
    double m_offsetY;
    double m_width;
    double m_height;

    QString m_color;
    bool m_selected;
};
