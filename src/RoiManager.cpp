#include "RoiManager.h"
#include "AppLogger.h"
#include <HalconCpp.h>
#include <QUrl>
#include <QUuid>
#include <QtMath>
#include <QVariantList>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <algorithm>
#include <cmath>

namespace {

QVariantMap AlgorithmParamsToVariantMap(const DetectionAlgorithmParams& params)
{
    return {
        {"topThresholdMin", params.topThresholdMin},
        {"topThresholdMax", params.topThresholdMax},
        {"downThresholdMin", params.downThresholdMin},
        {"downThresholdMax", params.downThresholdMax},
        {"topRatioMin", params.topRatioMin},
        {"topRatioMax", params.topRatioMax},
        {"topHeightMin", params.topHeightMin},
        {"topHeightMax", params.topHeightMax},
        {"topWidthMin", params.topWidthMin},
        {"topWidthMax", params.topWidthMax},
        {"downRatioMin", params.downRatioMin},
        {"downRatioMax", params.downRatioMax},
        {"downHeightMin", params.downHeightMin},
        {"downHeightMax", params.downHeightMax},
        {"downWidthMin", params.downWidthMin},
        {"downWidthMax", params.downWidthMax},
        {"inspectAngleDeg", params.inspectAngleDeg},
        {"inspectTopColumnOffset", params.inspectTopColumnOffset},
        {"inspectRectHalfWidth", params.inspectRectHalfWidth},
        {"inspectRectHalfHeight", params.inspectRectHalfHeight}
    };
}

QVariantMap DefaultHalconParams()
{
    return AlgorithmParamsToVariantMap(DetectionAlgorithmParams());
}

double VariantDoubleOrDefault(const QVariantMap& params, const QString& key, double defaultValue)
{
    return params.contains(key) ? params.value(key).toDouble() : defaultValue;
}

DetectionAlgorithmParams AlgorithmParamsFromVariantMap(const QVariantMap& params)
{
    DetectionAlgorithmParams next;
    next.topThresholdMin = VariantDoubleOrDefault(params, "topThresholdMin", next.topThresholdMin);
    next.topThresholdMax = VariantDoubleOrDefault(params, "topThresholdMax", next.topThresholdMax);
    next.downThresholdMin = VariantDoubleOrDefault(params, "downThresholdMin", next.downThresholdMin);
    next.downThresholdMax = VariantDoubleOrDefault(params, "downThresholdMax", next.downThresholdMax);

    next.topRatioMin = VariantDoubleOrDefault(params, "topRatioMin", next.topRatioMin);
    next.topRatioMax = VariantDoubleOrDefault(params, "topRatioMax", next.topRatioMax);
    next.topHeightMin = VariantDoubleOrDefault(params, "topHeightMin", next.topHeightMin);
    next.topHeightMax = VariantDoubleOrDefault(params, "topHeightMax", next.topHeightMax);
    next.topWidthMin = VariantDoubleOrDefault(params, "topWidthMin", next.topWidthMin);
    next.topWidthMax = VariantDoubleOrDefault(params, "topWidthMax", next.topWidthMax);

    next.downRatioMin = VariantDoubleOrDefault(params, "downRatioMin", next.downRatioMin);
    next.downRatioMax = VariantDoubleOrDefault(params, "downRatioMax", next.downRatioMax);
    next.downHeightMin = VariantDoubleOrDefault(params, "downHeightMin", next.downHeightMin);
    next.downHeightMax = VariantDoubleOrDefault(params, "downHeightMax", next.downHeightMax);
    next.downWidthMin = VariantDoubleOrDefault(params, "downWidthMin", next.downWidthMin);
    next.downWidthMax = VariantDoubleOrDefault(params, "downWidthMax", next.downWidthMax);

    next.inspectAngleDeg = VariantDoubleOrDefault(params, "inspectAngleDeg", next.inspectAngleDeg);
    next.inspectTopColumnOffset = VariantDoubleOrDefault(params, "inspectTopColumnOffset", next.inspectTopColumnOffset);
    next.inspectRectHalfWidth = VariantDoubleOrDefault(params, "inspectRectHalfWidth", next.inspectRectHalfWidth);
    next.inspectRectHalfHeight = VariantDoubleOrDefault(params, "inspectRectHalfHeight", next.inspectRectHalfHeight);
    return next;
}

RoiData* FirstRoiOfType(const QList<RoiData*>& roiList, const QString& roiType)
{
    for (RoiData* roi : roiList) {
        if (roi && roi->GetRoiType() == roiType) {
            return roi;
        }
    }

    return nullptr;
}

RoiData* LastRoiOfType(const QList<RoiData*>& roiList, const QString& roiType)
{
    for (int i = roiList.size() - 1; i >= 0; --i) {
        RoiData* roi = roiList.at(i);
        if (roi && roi->GetRoiType() == roiType) {
            return roi;
        }
    }

    return nullptr;
}

int FirstRoiIndexOfType(const QList<RoiData*>& roiList, const QString& roiType)
{
    for (int i = 0; i < roiList.size(); ++i) {
        RoiData* roi = roiList.at(i);
        if (roi && roi->GetRoiType() == roiType) {
            return i;
        }
    }

    return -1;
}

QString NormalizeImagePath(const QString& imagePath)
{
    const QUrl url(imagePath);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }

    return imagePath;
}

void GenRectangleFromRoi(RoiData* roi, HalconCpp::HObject* rectangle)
{
    HalconCpp::GenRectangle2(rectangle,
                             roi->GetCenterY(),
                             roi->GetCenterX(),
                             qDegreesToRadians(roi->GetAngle()),
                             roi->GetRoiWidth() * 0.5,
                             roi->GetRoiHeight() * 0.5);
}

QVariantList OffsetDifferenceRegionRuns(double centerX,
                                        double centerY,
                                        double width,
                                        double height,
                                        double circleRadius,
                                        double angle)
{
    QVariantList runs;
    if (width <= 0.0 || height <= 0.0 || circleRadius <= 0.0) {
        AppLogger::appendRoiReport(QString("OffsetDifferenceRegionRuns invalid size center=(%1,%2) size=(%3,%4) radius=%5 angle=%6")
                                       .arg(centerX)
                                       .arg(centerY)
                                       .arg(width)
                                       .arg(height)
                                       .arg(circleRadius)
                                       .arg(angle));
        qWarning() << "OffsetDifferenceRegionRuns invalid size"
                   << "centerX" << centerX
                   << "centerY" << centerY
                   << "width" << width
                   << "height" << height
                   << "circleRadius" << circleRadius
                   << "angle" << angle;
        return runs;
    }

    try {
        using namespace HalconCpp;

        HObject rectangle;
        HObject circle;
        HObject difference;
        HTuple rows;
        HTuple columnsBegin;
        HTuple columnsEnd;

        GenRectangle2(&rectangle,
                      centerY,
                      centerX,
                      qDegreesToRadians(angle),
                      width * 0.5,
                      height * 0.5);
        GenCircle(&circle, centerY, centerX, circleRadius);
        Difference(rectangle, circle, &difference);
        GetRegionRuns(difference, &rows, &columnsBegin, &columnsEnd);

        const Hlong runCount = rows.Length();
        runs.reserve(static_cast<qsizetype>(runCount));
        for (Hlong i = 0; i < runCount; ++i) {
            QVariantList run;
            run.reserve(3);
            run.append(rows[i].D());
            run.append(columnsBegin[i].D());
            run.append(columnsEnd[i].D());
            runs.append(QVariant::fromValue(run));
        }
        qDebug() << "OffsetDifferenceRegionRuns generated"
                 << "centerX" << centerX
                 << "centerY" << centerY
                 << "width" << width
                 << "height" << height
                 << "angle" << angle
                 << "circleRadius" << circleRadius
                 << "runCount" << runCount;
        AppLogger::appendRoiReport(QString("OffsetDifferenceRegionRuns generated center=(%1,%2) size=(%3,%4) angle=%5 circleRadius=%6 runCount=%7")
                                       .arg(centerX)
                                       .arg(centerY)
                                       .arg(width)
                                       .arg(height)
                                       .arg(angle)
                                       .arg(circleRadius)
                                       .arg(runCount));
    } catch (const HalconCpp::HException&) {
        AppLogger::appendRoiReport(QString("OffsetDifferenceRegionRuns HALCON exception center=(%1,%2) size=(%3,%4) radius=%5 angle=%6")
                                       .arg(centerX)
                                       .arg(centerY)
                                       .arg(width)
                                       .arg(height)
                                       .arg(circleRadius)
                                       .arg(angle));
        qWarning() << "OffsetDifferenceRegionRuns HALCON exception"
                   << "centerX" << centerX
                   << "centerY" << centerY
                   << "width" << width
                   << "height" << height
                   << "circleRadius" << circleRadius
                   << "angle" << angle;
        runs.clear();
    }

    return runs;
}

double ParamDouble(const QVariantMap& params, const QString& key)
{
    return params.value(key, DefaultHalconParams().value(key)).toDouble();
}

double JsonDoubleOrDefault(const QJsonObject& obj, const QString& key, double defaultValue)
{
    return obj.contains(key) ? obj.value(key).toDouble(defaultValue) : defaultValue;
}

QJsonObject RoiToJsonObject(RoiData* roi)
{
    QJsonObject obj;
    if (!roi) {
        return obj;
    }

    obj["roiId"] = roi->GetRoiId();
    obj["roiName"] = roi->GetRoiName();
    obj["roiType"] = roi->GetRoiType();
    obj["centerX"] = roi->GetCenterX();
    obj["centerY"] = roi->GetCenterY();
    obj["roiWidth"] = roi->GetRoiWidth();
    obj["roiHeight"] = roi->GetRoiHeight();
    obj["angle"] = roi->GetAngle();
    obj["offsetX"] = roi->GetOffsetX();
    obj["offsetY"] = roi->GetOffsetY();
    obj["width"] = roi->GetWidth();
    obj["height"] = roi->GetHeight();
    obj["circleRadius"] = roi->GetCircleRadius();
    obj["color"] = roi->GetColor();
    obj["selected"] = roi->GetSelected();
    return obj;
}

QJsonObject CoreRoiToJsonObject(RoiData* roi,
                                RoiData* offsetRoi,
                                const DetectionAlgorithmParams& params,
                                const QString& prefix)
{
    QJsonObject obj;
    if (!roi) {
        return obj;
    }

    const bool isTop = prefix == "top";
    obj["centerX"] = roi->GetCenterX();
    obj["centerY"] = roi->GetCenterY();
    obj["width"] = roi->GetRoiWidth();
    obj["height"] = roi->GetRoiHeight();
    obj["rotation"] = roi->GetAngle();

    obj["offsetROIX"] = offsetRoi ? offsetRoi->GetOffsetX() : 0.0;
    obj["offsetROIY"] = offsetRoi ? offsetRoi->GetOffsetY() : 0.0;
    obj["offsetWidth"] = offsetRoi ? offsetRoi->GetRoiWidth() : roi->GetRoiWidth();
    obj["offsetHeight"] = offsetRoi ? offsetRoi->GetRoiHeight() : roi->GetRoiHeight();
    obj["offsetCircleRadius"] = offsetRoi
        ? offsetRoi->GetCircleRadius()
        : qMin(roi->GetRoiWidth(), roi->GetRoiHeight()) * 0.5;
    obj["offsetRotation"] = offsetRoi ? offsetRoi->GetAngle() : params.inspectAngleDeg;

    obj[prefix + "ThresholdMin"] = isTop ? params.topThresholdMin : params.downThresholdMin;
    obj[prefix + "ThresholdMax"] = isTop ? params.topThresholdMax : params.downThresholdMax;
    obj[prefix + "RatioMin"] = isTop ? params.topRatioMin : params.downRatioMin;
    obj[prefix + "RatioMax"] = isTop ? params.topRatioMax : params.downRatioMax;
    obj[prefix + "WidthMin"] = isTop ? params.topWidthMin : params.downWidthMin;
    obj[prefix + "WidthMax"] = isTop ? params.topWidthMax : params.downWidthMax;
    obj[prefix + "HeightMin"] = isTop ? params.topHeightMin : params.downHeightMin;
    obj[prefix + "HeightMax"] = isTop ? params.topHeightMax : params.downHeightMax;
    return obj;
}

void LoadCoreRoiParams(const QJsonObject& obj,
                       const QString& prefix,
                       DetectionAlgorithmParams* params)
{
    if (!params) {
        return;
    }

    const bool isTop = prefix == "top";
    if (isTop) {
        params->topThresholdMin = JsonDoubleOrDefault(obj, "topThresholdMin", params->topThresholdMin);
        params->topThresholdMax = JsonDoubleOrDefault(obj, "topThresholdMax", params->topThresholdMax);
        params->topRatioMin = JsonDoubleOrDefault(obj, "topRatioMin", params->topRatioMin);
        params->topRatioMax = JsonDoubleOrDefault(obj, "topRatioMax", params->topRatioMax);
        params->topWidthMin = JsonDoubleOrDefault(obj, "topWidthMin", params->topWidthMin);
        params->topWidthMax = JsonDoubleOrDefault(obj, "topWidthMax", params->topWidthMax);
        params->topHeightMin = JsonDoubleOrDefault(obj, "topHeightMin", params->topHeightMin);
        params->topHeightMax = JsonDoubleOrDefault(obj, "topHeightMax", params->topHeightMax);
    } else {
        params->downThresholdMin = JsonDoubleOrDefault(obj, "downThresholdMin", params->downThresholdMin);
        params->downThresholdMax = JsonDoubleOrDefault(obj, "downThresholdMax", params->downThresholdMax);
        params->downRatioMin = JsonDoubleOrDefault(obj, "downRatioMin", params->downRatioMin);
        params->downRatioMax = JsonDoubleOrDefault(obj, "downRatioMax", params->downRatioMax);
        params->downWidthMin = JsonDoubleOrDefault(obj, "downWidthMin", params->downWidthMin);
        params->downWidthMax = JsonDoubleOrDefault(obj, "downWidthMax", params->downWidthMax);
        params->downHeightMin = JsonDoubleOrDefault(obj, "downHeightMin", params->downHeightMin);
        params->downHeightMax = JsonDoubleOrDefault(obj, "downHeightMax", params->downHeightMax);
    }
}

QJsonObject DetectionShapeToJsonObject(const DetectionRoiShape& shape)
{
    QJsonObject obj;
    obj["centerX"] = shape.centerX;
    obj["centerY"] = shape.centerY;
    obj["width"] = shape.width;
    obj["height"] = shape.height;
    obj["rotation"] = shape.angle;
    obj["offsetX"] = shape.offsetX;
    obj["offsetY"] = shape.offsetY;
    obj["offsetRotation"] = shape.offsetRotation;
    return obj;
}

QJsonObject VariantMapToJsonObject(const QVariantMap& map)
{
    QJsonObject obj;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        obj.insert(it.key(), QJsonValue::fromVariant(it.value()));
    }
    return obj;
}

QVariantMap JsonObjectToVariantMap(const QJsonObject& obj)
{
    QVariantMap map;
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        map.insert(it.key(), it.value().toVariant());
    }
    return map;
}

}

RoiManager::RoiManager(QObject* parent)
    : QObject(parent),
      m_halconParams(DefaultHalconParams())
{
}

RoiManager::~RoiManager()
{
    qDeleteAll(m_roiList);
    qDeleteAll(m_offsetRoiList);
    m_roiList.clear();
    m_offsetRoiList.clear();
}

QQmlListProperty<RoiData> RoiManager::GetRoiItems()
{
    return QQmlListProperty<RoiData>(this, &m_roiList, RoiListCount, RoiListAt);
}

QQmlListProperty<RoiData> RoiManager::GetOffsetRoiItems()
{
    return QQmlListProperty<RoiData>(this, &m_offsetRoiList, RoiListCount, RoiListAt);
}

QString RoiManager::GenerateRoiId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

RoiData* RoiManager::AddRoi(const QString& roiType,
                            double centerX,
                            double centerY,
                            double roiWidth,
                            double roiHeight,
                            double angle,
                            const QString& color)
{
    RoiData* roi = CreateRoi(roiType, centerX, centerY, roiWidth, roiHeight, angle, color);
    m_roiList.append(roi);
    emit RoiListChanged();
    return roi;
}

RoiData* RoiManager::CreateRoi(const QString& roiType,
                               double centerX,
                               double centerY,
                               double roiWidth,
                               double roiHeight,
                               double angle,
                               const QString& color)
{
    RoiData* roi = new RoiData(this);
    roi->SetRoiId(GenerateRoiId());
    const int roiIndex = (roiType == "TopOffsetROI" || roiType == "DownOffsetROI")
        ? m_offsetRoiList.size() + 1
        : m_roiList.size() + 1;
    roi->SetRoiName(QString("%1_%2").arg(roiType).arg(roiIndex));
    roi->SetRoiType(roiType);
    roi->SetCenterX(centerX);
    roi->SetCenterY(centerY);
    roi->SetRoiWidth(roiWidth);
    roi->SetRoiHeight(roiHeight);
    roi->SetAngle(angle);
    roi->SetOffsetX(0.0);
    roi->SetOffsetY(0.0);
    roi->SetWidth(roiWidth);
    roi->SetHeight(roiHeight);
    roi->SetCircleRadius(qMin(roiWidth, roiHeight) * 0.5);
    roi->SetColor(color);
    roi->SetSelected(false);
    return roi;
}

RoiData* RoiManager::AddOffsetRoi(const QString& roiType,
                                  double baseX,
                                  double baseY,
                                  double offsetX,
                                  double offsetY,
                                  double width,
                                  double height,
                                  double circleRadius,
                                  double angle,
                                  const QString& color)
{
    const int existingIndex = FirstRoiIndexOfType(m_offsetRoiList, roiType);
    RoiData* roi = existingIndex >= 0
        ? m_offsetRoiList.at(existingIndex)
        : CreateRoi(roiType, baseX + offsetX, baseY + offsetY, width, height, angle, color);

    roi->SetCenterX(baseX + offsetX);
    roi->SetCenterY(baseY + offsetY);
    roi->SetRoiWidth(width);
    roi->SetRoiHeight(height);
    roi->SetAngle(angle);
    roi->SetOffsetX(offsetX);
    roi->SetOffsetY(offsetY);
    roi->SetWidth(width);
    roi->SetHeight(height);
    roi->SetCircleRadius(circleRadius);
    roi->SetColor(color);
    roi->SetRegionRuns(OffsetDifferenceRegionRuns(roi->GetCenterX(),
                                                  roi->GetCenterY(),
                                                  roi->GetRoiWidth(),
                                                  roi->GetRoiHeight(),
                                                  roi->GetCircleRadius(),
                                                  roi->GetAngle()));
    qDebug() << "AddOffsetRoi report"
             << "type" << roiType
             << "centerX" << roi->GetCenterX()
             << "centerY" << roi->GetCenterY()
             << "offsetX" << roi->GetOffsetX()
             << "offsetY" << roi->GetOffsetY()
             << "width" << roi->GetRoiWidth()
             << "height" << roi->GetRoiHeight()
             << "circleRadius" << roi->GetCircleRadius()
             << "angle" << roi->GetAngle()
             << "regionRuns" << roi->GetRegionRuns().size();
    AppLogger::appendRoiReport(QString("AddOffsetRoi type=%1 center=(%2,%3) offset=(%4,%5) size=(%6,%7) radius=%8 angle=%9 regionRuns=%10")
                                   .arg(roiType)
                                   .arg(roi->GetCenterX())
                                   .arg(roi->GetCenterY())
                                   .arg(roi->GetOffsetX())
                                   .arg(roi->GetOffsetY())
                                   .arg(roi->GetRoiWidth())
                                   .arg(roi->GetRoiHeight())
                                   .arg(roi->GetCircleRadius())
                                   .arg(roi->GetAngle())
                                   .arg(roi->GetRegionRuns().size()));

    if (existingIndex < 0) {
        m_offsetRoiList.append(roi);
    } else {
        QList<RoiData*> removedRois;
        for (int i = m_offsetRoiList.size() - 1; i > existingIndex; --i) {
            RoiData* extra = m_offsetRoiList.at(i);
            if (extra && extra->GetRoiType() == roiType) {
                removedRois.append(TakeRoiAt(&m_offsetRoiList, i));
            }
        }
        emit OffsetRoiListChanged();
        DisposeRois(removedRois);
        return roi;
    }

    emit OffsetRoiListChanged();
    return roi;
}

RoiData* RoiManager::TakeRoiAt(QList<RoiData*>* roiList, int index)
{
    if (!roiList || index < 0 || index >= roiList->size()) {
        return nullptr;
    }

    RoiData* roi = roiList->takeAt(index);
    if (roi) {
        roi->SetSelected(false);
    }
    return roi;
}

void RoiManager::DisposeRois(const QList<RoiData*>& rois)
{
    for (RoiData* roi : rois) {
        if (roi) {
            roi->deleteLater();
        }
    }
}

void RoiManager::ClearRoiStorage(bool notify)
{
    const QList<RoiData*> removedRois = m_roiList;
    const QList<RoiData*> removedOffsetRois = m_offsetRoiList;
    m_roiList.clear();
    m_offsetRoiList.clear();

    if (notify) {
        emit RoiListChanged();
        emit OffsetRoiListChanged();
    }

    DisposeRois(removedRois);
    DisposeRois(removedOffsetRois);
}

void RoiManager::RemoveOffsetRoisByType(const QString& roiType)
{
    const int beforeCount = m_offsetRoiList.size();
    QList<RoiData*> removedRois;
    for (int i = m_offsetRoiList.size() - 1; i >= 0; --i)
    {
        RoiData* roi = m_offsetRoiList[i];
        if (roi && roi->GetRoiType() == roiType)
        {
            removedRois.append(TakeRoiAt(&m_offsetRoiList, i));
        }
    }

    if (beforeCount != m_offsetRoiList.size()) {
        emit OffsetRoiListChanged();
        DisposeRois(removedRois);
    }
}

void RoiManager::RemoveRoi(const QString& roiId)
{
    for (int i = 0; i < m_roiList.size(); ++i)
    {
        if (m_roiList[i]->GetRoiId() == roiId)
        {
            RoiData* removedRoi = TakeRoiAt(&m_roiList, i);
            emit RoiListChanged();
            DisposeRois({removedRoi});
            return;
        }
    }

    for (int i = 0; i < m_offsetRoiList.size(); ++i)
    {
        if (m_offsetRoiList[i]->GetRoiId() == roiId)
        {
            RoiData* removedRoi = TakeRoiAt(&m_offsetRoiList, i);
            emit OffsetRoiListChanged();
            DisposeRois({removedRoi});
            return;
        }
    }
}

void RoiManager::RemoveRoisByType(const QString& roiType)
{
    const int beforeCount = m_roiList.size();
    QList<RoiData*> removedRois;
    for (int i = m_roiList.size() - 1; i >= 0; --i)
    {
        RoiData* roi = m_roiList[i];
        if (roi && roi->GetRoiType() == roiType)
        {
            removedRois.append(TakeRoiAt(&m_roiList, i));
        }
    }

    if (beforeCount != m_roiList.size())
    {
        emit RoiListChanged();
        DisposeRois(removedRois);
    }
}

void RoiManager::ClearAllRois()
{
    ClearRoiStorage(true);
}

RoiData* RoiManager::GetRoiById(const QString& roiId)
{
    for (RoiData* roi : m_roiList)
    {
        if (roi->GetRoiId() == roiId)
        {
            return roi;
        }
    }
    for (RoiData* roi : m_offsetRoiList)
    {
        if (roi->GetRoiId() == roiId)
        {
            return roi;
        }
    }
    return nullptr;
}

qsizetype RoiManager::RoiListCount(QQmlListProperty<RoiData>* property)
{
    auto* roiList = property
        ? static_cast<QList<RoiData*>*>(property->data)
        : nullptr;
    return roiList ? roiList->size() : 0;
}

RoiData* RoiManager::RoiListAt(QQmlListProperty<RoiData>* property, qsizetype index)
{
    auto* roiList = property
        ? static_cast<QList<RoiData*>*>(property->data)
        : nullptr;
    if (!roiList || index < 0 || index >= roiList->size()) {
        return nullptr;
    }

    return roiList->at(index);
}

void RoiManager::SelectOnly(const QString& roiId)
{
    for (RoiData* roi : m_roiList)
    {
        roi->SetSelected(roi->GetRoiId() == roiId);
    }
    for (RoiData* roi : m_offsetRoiList)
    {
        roi->SetSelected(roi->GetRoiId() == roiId);
    }
}

void RoiManager::UnselectAll()
{
    for (RoiData* roi : m_roiList)
    {
        roi->SetSelected(false);
    }
    for (RoiData* roi : m_offsetRoiList)
    {
        roi->SetSelected(false);
    }
}

bool RoiManager::SaveToJson(const QString& filePath)
{
    RoiData* topRoi = FirstRoiOfType(m_roiList, "TopROI");
    RoiData* topOffsetRoi = LastRoiOfType(m_offsetRoiList, "TopOffsetROI");
    RoiData* downRoi = FirstRoiOfType(m_roiList, "DownROI");
    RoiData* downOffsetRoi = LastRoiOfType(m_offsetRoiList, "DownOffsetROI");
    const DetectionAlgorithmParams params = GetAlgorithmParams();

    QJsonObject inspectObject;
    inspectObject["inspectAngleDeg"] = params.inspectAngleDeg;
    inspectObject["inspectTopColumnOffset"] = params.inspectTopColumnOffset;
    inspectObject["inspectRectHalfWidth"] = params.inspectRectHalfWidth;
    inspectObject["inspectRectHalfHeight"] = params.inspectRectHalfHeight;

    QJsonObject root;
    root["TopROI"] = CoreRoiToJsonObject(topRoi, topOffsetRoi, params, "top");
    root["DownROI"] = CoreRoiToJsonObject(downRoi, downOffsetRoi, params, "down");
    root["inspect"] = inspectObject;
    root["dropThres"] = 27;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

bool RoiManager::LoadFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject())
    {
        return false;
    }

    QJsonObject root = doc.object();
    if (root.value("TopROI").isObject() || root.value("DownROI").isObject()) {
        QJsonObject topObject = root.value("TopROI").toObject();
        QJsonObject downObject = root.value("DownROI").toObject();
        QJsonObject inspectObject = root.value("inspect").toObject();

        DetectionAlgorithmParams params = GetAlgorithmParams();
        LoadCoreRoiParams(topObject, "top", &params);
        LoadCoreRoiParams(downObject, "down", &params);
        params.inspectAngleDeg = JsonDoubleOrDefault(inspectObject, "inspectAngleDeg", params.inspectAngleDeg);
        params.inspectTopColumnOffset = JsonDoubleOrDefault(inspectObject, "inspectTopColumnOffset", params.inspectTopColumnOffset);
        params.inspectRectHalfWidth = JsonDoubleOrDefault(inspectObject, "inspectRectHalfWidth", params.inspectRectHalfWidth);
        params.inspectRectHalfHeight = JsonDoubleOrDefault(inspectObject, "inspectRectHalfHeight", params.inspectRectHalfHeight);
        SetAlgorithmParams(params);

        const auto loadCoreRoi = [this](const QJsonObject& obj,
                                        const QString& roiType,
                                        const QString& offsetRoiType,
                                        const QString& color) {
            if (obj.isEmpty()) {
                return;
            }

            const double centerX = obj.value("centerX").toDouble();
            const double centerY = obj.value("centerY").toDouble();
            const double width = obj.value("width").toDouble();
            const double height = obj.value("height").toDouble();
            const double rotation = obj.value("rotation").toDouble();

            RoiData* roi = CreateRoi(roiType, centerX, centerY, width, height, rotation, color);
            m_roiList.append(roi);

            if (!obj.contains("offsetROIX") &&
                !obj.contains("offsetROIY") &&
                !obj.contains("offsetWidth") &&
                !obj.contains("offsetHeight") &&
                !obj.contains("offsetRotation")) {
                return;
            }

            const double offsetX = JsonDoubleOrDefault(obj, "offsetROIX", JsonDoubleOrDefault(obj, "offsetX", 0.0));
            const double offsetY = JsonDoubleOrDefault(obj, "offsetROIY", JsonDoubleOrDefault(obj, "offsetY", 0.0));
            const double offsetWidth = JsonDoubleOrDefault(obj, "offsetWidth", width);
            const double offsetHeight = JsonDoubleOrDefault(obj, "offsetHeight", height);
            const double offsetCircleRadius =
                JsonDoubleOrDefault(obj, "offsetCircleRadius", qMin(offsetWidth, offsetHeight) * 0.5);
            const double offsetRotation = JsonDoubleOrDefault(obj, "offsetRotation", rotation);

            RoiData* offsetRoi = CreateRoi(offsetRoiType,
                                           centerX + offsetX,
                                           centerY + offsetY,
                                           offsetWidth,
                                           offsetHeight,
                                           offsetRotation,
                                           color);
            offsetRoi->SetOffsetX(offsetX);
            offsetRoi->SetOffsetY(offsetY);
            offsetRoi->SetCircleRadius(offsetCircleRadius);
            offsetRoi->SetRegionRuns(OffsetDifferenceRegionRuns(offsetRoi->GetCenterX(),
                                                                offsetRoi->GetCenterY(),
                                                                offsetRoi->GetRoiWidth(),
                                                                offsetRoi->GetRoiHeight(),
                                                                offsetRoi->GetCircleRadius(),
                                                                offsetRoi->GetAngle()));
            m_offsetRoiList.append(offsetRoi);
        };

        ClearRoiStorage(false);
        loadCoreRoi(topObject, "TopROI", "TopOffsetROI", "#1677ff");
        loadCoreRoi(downObject, "DownROI", "DownOffsetROI", "#22c55e");

        emit RoiListChanged();
        emit OffsetRoiListChanged();
        return true;
    }

    if (root.value("algorithm").isObject()) {
        SetHalconParams(JsonObjectToVariantMap(root.value("algorithm").toObject()));
    } else if (root.value("detectionRoiConfig").toObject().value("algorithm").isObject()) {
        SetHalconParams(JsonObjectToVariantMap(root.value("detectionRoiConfig").toObject().value("algorithm").toObject()));
    } else if (root.value("halconParams").isObject()) {
        SetHalconParams(JsonObjectToVariantMap(root.value("halconParams").toObject()));
    } else {
        SetHalconParams(DefaultHalconParams());
    }

    const auto loadRoiArray = [this](const QJsonArray& roiArray, QList<RoiData*>* targetList) {
        for (const QJsonValue& value : roiArray)
        {
            if (!value.isObject()) continue;

            QJsonObject obj = value.toObject();

            RoiData* roi = new RoiData(this);
            roi->SetRoiId(obj["roiId"].toString());
            roi->SetRoiName(obj["roiName"].toString());
            roi->SetRoiType(obj["roiType"].toString());
            roi->SetCenterX(obj["centerX"].toDouble());
            roi->SetCenterY(obj["centerY"].toDouble());
            roi->SetRoiWidth(obj["roiWidth"].toDouble());
            roi->SetRoiHeight(obj["roiHeight"].toDouble());
            roi->SetAngle(obj["angle"].toDouble());
            roi->SetOffsetX(obj["offsetX"].toDouble());
            roi->SetOffsetY(obj["offsetY"].toDouble());
            roi->SetWidth(obj.contains("width") ? obj["width"].toDouble() : roi->GetRoiWidth());
            roi->SetHeight(obj.contains("height") ? obj["height"].toDouble() : roi->GetRoiHeight());
            roi->SetCircleRadius(obj.contains("circleRadius")
                                     ? obj["circleRadius"].toDouble()
                                     : qMin(roi->GetRoiWidth(), roi->GetRoiHeight()) * 0.5);
            roi->SetColor(obj["color"].toString());
            roi->SetSelected(obj["selected"].toBool(false));
            if (targetList == &m_offsetRoiList) {
                roi->SetRegionRuns(OffsetDifferenceRegionRuns(roi->GetCenterX(),
                                                              roi->GetCenterY(),
                                                              roi->GetRoiWidth(),
                                                              roi->GetRoiHeight(),
                                                              roi->GetCircleRadius(),
                                                              roi->GetAngle()));
            }

            targetList->append(roi);
        }
    };

    ClearRoiStorage(false);
    if (root.value("rois").isArray()) {
        loadRoiArray(root.value("rois").toArray(), &m_roiList);
    } else {
        const auto loadNamedRoi = [this](const QJsonObject& obj,
                                         const QString& roiType,
                                         const QString& color) {
            if (obj.isEmpty()) {
                return;
            }

            RoiData* roi = CreateRoi(roiType,
                                     obj.value("centerX").toDouble(),
                                     obj.value("centerY").toDouble(),
                                     obj.value("roiWidth").toDouble(obj.value("width").toDouble()),
                                     obj.value("roiHeight").toDouble(obj.value("height").toDouble()),
                                     obj.value("angle").toDouble(obj.value("rotation").toDouble()),
                                     obj.value("color").toString(color));
            m_roiList.append(roi);
        };

        QJsonObject topObject = root.value("top").toObject();
        QJsonObject downObject = root.value("down").toObject();
        if (topObject.isEmpty() || downObject.isEmpty()) {
            const QJsonObject detectionObject = root.value("detectionRoiConfig").toObject();
            topObject = detectionObject.value("top").toObject();
            downObject = detectionObject.value("down").toObject();
        }
        loadNamedRoi(topObject, "TopROI", "#1677ff");
        loadNamedRoi(downObject, "DownROI", "#22c55e");
    }

    if (root.value("offsetRois").isArray()) {
        loadRoiArray(root.value("offsetRois").toArray(), &m_offsetRoiList);
    } else {
        const auto loadNamedOffsetRoi = [this](const QJsonObject& obj,
                                               const QString& roiType,
                                               const QString& color) {
            if (obj.isEmpty() || (!obj.contains("offsetX") && !obj.contains("offsetY"))) {
                return;
            }

            const double centerX = obj.value("centerX").toDouble();
            const double centerY = obj.value("centerY").toDouble();
            const double offsetX = obj.value("offsetX").toDouble();
            const double offsetY = obj.value("offsetY").toDouble();
            const double offsetWidth = obj.value("offsetWidth").toDouble(obj.value("roiWidth").toDouble(obj.value("width").toDouble(1.0)));
            const double offsetHeight = obj.value("offsetHeight").toDouble(obj.value("roiHeight").toDouble(obj.value("height").toDouble(1.0)));
            const double offsetCircleRadius = obj.value("offsetCircleRadius").toDouble(
                obj.value("circleRadius").toDouble(qMin(offsetWidth, offsetHeight) * 0.5));
            RoiData* roi = CreateRoi(roiType,
                                     centerX + offsetX,
                                     centerY + offsetY,
                                     offsetWidth,
                                     offsetHeight,
                                     obj.value("offsetRotation").toDouble(obj.value("rotation").toDouble()),
                                     color);
            roi->SetOffsetX(offsetX);
            roi->SetOffsetY(offsetY);
            roi->SetCircleRadius(offsetCircleRadius);
            roi->SetRegionRuns(OffsetDifferenceRegionRuns(roi->GetCenterX(),
                                                          roi->GetCenterY(),
                                                          roi->GetRoiWidth(),
                                                          roi->GetRoiHeight(),
                                                          roi->GetCircleRadius(),
                                                          roi->GetAngle()));
            m_offsetRoiList.append(roi);
        };

        QJsonObject topObject = root.value("top").toObject();
        QJsonObject downObject = root.value("down").toObject();
        if (topObject.isEmpty() || downObject.isEmpty()) {
            const QJsonObject detectionObject = root.value("detectionRoiConfig").toObject();
            topObject = detectionObject.value("top").toObject();
            downObject = detectionObject.value("down").toObject();
        }
        loadNamedOffsetRoi(topObject, "TopOffsetROI", "#1677ff");
        loadNamedOffsetRoi(downObject, "DownOffsetROI", "#22c55e");
    }

    emit RoiListChanged();
    emit OffsetRoiListChanged();
    return true;
}

bool RoiManager::BuildDetectionConfig(DetectionRoiConfig* config) const
{
    if (!config) {
        return false;
    }

    DetectionRoiConfig nextConfig;
    RoiData* topRoi = FirstRoiOfType(m_roiList, "TopROI");
    RoiData* downRoi = FirstRoiOfType(m_roiList, "DownROI");
    RoiData* topOffsetRoi = LastRoiOfType(m_offsetRoiList, "TopOffsetROI");
    RoiData* downOffsetRoi = LastRoiOfType(m_offsetRoiList, "DownOffsetROI");
    nextConfig.algorithmParams = GetAlgorithmParams();

    if (!topRoi || !downRoi) {
        *config = nextConfig;
        return false;
    }

    nextConfig.valid = true;
    nextConfig.top.centerX = topRoi->GetCenterX();
    nextConfig.top.centerY = topRoi->GetCenterY();
    nextConfig.top.width = topRoi->GetRoiWidth();
    nextConfig.top.height = topRoi->GetRoiHeight();
    nextConfig.top.angle = topRoi->GetAngle();
    nextConfig.top.offsetX = static_cast<int>(std::lround(topOffsetRoi ? topOffsetRoi->GetOffsetX() : topRoi->GetOffsetX()));
    nextConfig.top.offsetY = static_cast<int>(std::lround(topOffsetRoi ? topOffsetRoi->GetOffsetY() : topRoi->GetOffsetY()));
    nextConfig.top.offsetCircleRadius = topOffsetRoi
        ? topOffsetRoi->GetCircleRadius()
        : qMin(topRoi->GetRoiWidth(), topRoi->GetRoiHeight()) * 0.5;
    nextConfig.top.offsetRotation = topOffsetRoi ? topOffsetRoi->GetAngle() : nextConfig.algorithmParams.inspectAngleDeg;

    nextConfig.down.centerX = downRoi->GetCenterX();
    nextConfig.down.centerY = downRoi->GetCenterY();
    nextConfig.down.width = downRoi->GetRoiWidth();
    nextConfig.down.height = downRoi->GetRoiHeight();
    nextConfig.down.angle = downRoi->GetAngle();
    nextConfig.down.offsetX = static_cast<int>(std::lround(downOffsetRoi ? downOffsetRoi->GetOffsetX() : downRoi->GetOffsetX()));
    nextConfig.down.offsetY = static_cast<int>(std::lround(downOffsetRoi ? downOffsetRoi->GetOffsetY() : downRoi->GetOffsetY()));
    nextConfig.down.offsetCircleRadius = downOffsetRoi
        ? downOffsetRoi->GetCircleRadius()
        : qMin(downRoi->GetRoiWidth(), downRoi->GetRoiHeight()) * 0.5;
    nextConfig.down.offsetRotation = downOffsetRoi ? downOffsetRoi->GetAngle() : nextConfig.algorithmParams.inspectAngleDeg;

    *config = nextConfig;
    return true;
}

DetectionAlgorithmParams RoiManager::GetAlgorithmParams() const
{
    return AlgorithmParamsFromVariantMap(m_halconParams);
}

void RoiManager::SetAlgorithmParams(const DetectionAlgorithmParams& params)
{
    SetHalconParams(AlgorithmParamsToVariantMap(params));
}

QVariantMap RoiManager::GetHalconParams() const
{
    return AlgorithmParamsToVariantMap(GetAlgorithmParams());
}

void RoiManager::SetHalconParams(const QVariantMap& params)
{
    QVariantMap nextParams = AlgorithmParamsToVariantMap(AlgorithmParamsFromVariantMap(params));

    if (m_halconParams == nextParams) {
        return;
    }

    m_halconParams = nextParams;
    emit HalconParamsChanged();
}

void RoiManager::AppendRoiDebugReport(const QString& message) const
{
    AppLogger::appendRoiReport(message);
}

QVariantMap RoiManager::ExecuteHalcon(const QString& imagePath)
{
    QVariantMap result;
    result["ok"] = false;

    RoiData* topRoi = FirstRoiOfType(m_roiList, "TopROI");
    RoiData* downRoi = FirstRoiOfType(m_roiList, "DownROI");

    if (!topRoi || !downRoi) {
        result["message"] = "需要至少一个TopROI和一个DownROI";
        return result;
    }

    const QString normalizedImagePath = NormalizeImagePath(imagePath);
    if (normalizedImagePath.isEmpty()) {
        result["message"] = "请先加载图片";
        return result;
    }

    try {
        using namespace HalconCpp;

        HObject image;
        HObject topRectangle, downRectangle;
        HObject topReducedDomain, downReducedDomain;
        HObject topRegion, downRegion;
        HObject topConnectedRegion, downConnectedRegion;
        HObject topSelectedRegion, downSelectedRegion;
        HTuple topNumber, downNumber;
        HTuple topRows, downColumns;

        const QByteArray pathBytes = normalizedImagePath.toLocal8Bit();
        ReadImage(&image, pathBytes.constData());

        GenRectangleFromRoi(topRoi, &topRectangle);
        GenRectangleFromRoi(downRoi, &downRectangle);

        ReduceDomain(image, topRectangle, &topReducedDomain);
        ReduceDomain(image, downRectangle, &downReducedDomain);

        const QVariantMap params = m_halconParams;

        Threshold(topReducedDomain,
                  &topRegion,
                  ParamDouble(params, "topThresholdMin"),
                  ParamDouble(params, "topThresholdMax"));
        Threshold(downReducedDomain,
                  &downRegion,
                  ParamDouble(params, "downThresholdMin"),
                  ParamDouble(params, "downThresholdMax"));

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(ParamDouble(params, "topRatioMin"))
                        .Append(ParamDouble(params, "topHeightMin"))
                        .Append(ParamDouble(params, "topWidthMin")),
                    HTuple(ParamDouble(params, "topRatioMax"))
                        .Append(ParamDouble(params, "topHeightMax"))
                        .Append(ParamDouble(params, "topWidthMax")));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(ParamDouble(params, "downRatioMin"))
                        .Append(ParamDouble(params, "downHeightMin"))
                        .Append(ParamDouble(params, "downWidthMin")),
                    HTuple(ParamDouble(params, "downRatioMax"))
                        .Append(ParamDouble(params, "downHeightMax"))
                        .Append(ParamDouble(params, "downWidthMax")));

        CountObj(topSelectedRegion, &topNumber);
        CountObj(downSelectedRegion, &downNumber);

        const int topCount = topNumber[0].I();
        const int downCount = downNumber[0].I();

        result["topCount"] = topCount;
        result["downCount"] = downCount;

        if (topCount != 1 || downCount != 1) {
            result["message"] = QString("区域数量不满足要求: TopROI=%1, DownROI=%2")
                                    .arg(topCount)
                                    .arg(downCount);
            return result;
        }

        RegionFeatures(topSelectedRegion, "row", &topRows);
        RegionFeatures(downSelectedRegion, "column", &downColumns);

        result["baseY"] = topRows[0].D();
        result["baseX"] = downColumns[0].D();
        result["ok"] = true;
        result["message"] = "HALCON执行完成";
        return result;
    }
    catch (const HalconCpp::HException&) {
        result["message"] = "HALCON执行失败";
        return result;
    }
}
