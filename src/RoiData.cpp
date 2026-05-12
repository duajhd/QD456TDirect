#include "RoiData.h"

RoiData::RoiData(QObject* parent)
    : QObject(parent),
    m_centerX(0.0),
    m_centerY(0.0),
    m_roiWidth(100.0),
    m_roiHeight(80.0),
    m_angle(0.0),
    m_offsetX(0.0),
    m_offsetY(0.0),
    m_width(100.0),
    m_height(80.0),
    m_color("#00AEEF"),
    m_selected(false)
{
}

QString RoiData::GetRoiId() const
{
    return m_roiId;
}

void RoiData::SetRoiId(const QString& value)
{
    if (m_roiId == value) return;
    m_roiId = value;
    emit RoiChanged();
}

QString RoiData::GetRoiName() const
{
    return m_roiName;
}

void RoiData::SetRoiName(const QString& value)
{
    if (m_roiName == value) return;
    m_roiName = value;
    emit RoiChanged();
}

QString RoiData::GetRoiType() const
{
    return m_roiType;
}

void RoiData::SetRoiType(const QString& value)
{
    if (m_roiType == value) return;
    m_roiType = value;
    emit RoiChanged();
}

double RoiData::GetCenterX() const
{
    return m_centerX;
}

void RoiData::SetCenterX(double value)
{
    if (qFuzzyCompare(m_centerX, value)) return;
    m_centerX = value;
    emit RoiChanged();
}

double RoiData::GetCenterY() const
{
    return m_centerY;
}

void RoiData::SetCenterY(double value)
{
    if (qFuzzyCompare(m_centerY, value)) return;
    m_centerY = value;
    emit RoiChanged();
}

double RoiData::GetRoiWidth() const
{
    return m_roiWidth;
}

void RoiData::SetRoiWidth(double value)
{
    if (qFuzzyCompare(m_roiWidth, value)) return;
    m_roiWidth = value;
    emit RoiChanged();
}

double RoiData::GetRoiHeight() const
{
    return m_roiHeight;
}

void RoiData::SetRoiHeight(double value)
{
    if (qFuzzyCompare(m_roiHeight, value)) return;
    m_roiHeight = value;
    emit RoiChanged();
}

double RoiData::GetAngle() const
{
    return m_angle;
}

void RoiData::SetAngle(double value)
{
    if (qFuzzyCompare(m_angle, value)) return;
    m_angle = value;
    emit RoiChanged();
}

double RoiData::GetOffsetX() const
{
    return m_offsetX;
}

void RoiData::SetOffsetX(double value)
{
    if (qFuzzyCompare(m_offsetX, value)) return;
    m_offsetX = value;
    emit RoiChanged();
}

double RoiData::GetOffsetY() const
{
    return m_offsetY;
}

void RoiData::SetOffsetY(double value)
{
    if (qFuzzyCompare(m_offsetY, value)) return;
    m_offsetY = value;
    emit RoiChanged();
}

double RoiData::GetWidth() const
{
    return m_width;
}

void RoiData::SetWidth(double value)
{
    if (qFuzzyCompare(m_width, value)) return;
    m_width = value;
    emit RoiChanged();
}

double RoiData::GetHeight() const
{
    return m_height;
}

void RoiData::SetHeight(double value)
{
    if (qFuzzyCompare(m_height, value)) return;
    m_height = value;
    emit RoiChanged();
}

QString RoiData::GetColor() const
{
    return m_color;
}

void RoiData::SetColor(const QString& value)
{
    if (m_color == value) return;
    m_color = value;
    emit RoiChanged();
}

bool RoiData::GetSelected() const
{
    return m_selected;
}

void RoiData::SetSelected(bool value)
{
    if (m_selected == value) return;
    m_selected = value;
    emit RoiChanged();
}
