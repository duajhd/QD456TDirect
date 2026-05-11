#include "RoiManager.h"
#include <HalconCpp.h>
#include <QUrl>
#include <QUuid>
#include <QtMath>

namespace {

RoiData* FirstRoiOfType(const QList<RoiData*>& roiList, const QString& roiType)
{
    for (RoiData* roi : roiList) {
        if (roi && roi->GetRoiType() == roiType) {
            return roi;
        }
    }

    return nullptr;
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

}

RoiManager::RoiManager(QObject* parent)
    : QObject(parent)
{
}

RoiManager::~RoiManager()
{
    qDeleteAll(m_roiList);
    m_roiList.clear();
}

QQmlListProperty<RoiData> RoiManager::GetRoiList()
{
    return QQmlListProperty<RoiData>(this, this, &RoiManager::RoiCount, &RoiManager::RoiAt);
}

qsizetype RoiManager::RoiCount(QQmlListProperty<RoiData>* list)
{
    RoiManager* manager = qobject_cast<RoiManager*>(list->object);
    if (manager == nullptr)
    {
        return 0;
    }
    return manager->m_roiList.size();
}

RoiData* RoiManager::RoiAt(QQmlListProperty<RoiData>* list, qsizetype index)
{
    RoiManager* manager = qobject_cast<RoiManager*>(list->object);
    if (manager == nullptr)
    {
        return nullptr;
    }

    if (index < 0 || index >= manager->m_roiList.size())
    {
        return nullptr;
    }

    return manager->m_roiList.at(index);
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
    RoiData* roi = new RoiData(this);
    roi->SetRoiId(GenerateRoiId());
    roi->SetRoiName(QString("%1_%2").arg(roiType).arg(m_roiList.size() + 1));
    roi->SetRoiType(roiType);
    roi->SetCenterX(centerX);
    roi->SetCenterY(centerY);
    roi->SetRoiWidth(roiWidth);
    roi->SetRoiHeight(roiHeight);
    roi->SetAngle(angle);
    roi->SetColor(color);
    roi->SetSelected(false);

    m_roiList.append(roi);
    emit RoiListChanged();
    return roi;
}

void RoiManager::RemoveRoi(const QString& roiId)
{
    for (int i = 0; i < m_roiList.size(); ++i)
    {
        if (m_roiList[i]->GetRoiId() == roiId)
        {
            RoiData* roi = m_roiList.takeAt(i);
            roi->deleteLater();
            emit RoiListChanged();
            return;
        }
    }
}

void RoiManager::ClearAllRois()
{
    qDeleteAll(m_roiList);
    m_roiList.clear();
    emit RoiListChanged();
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
    return nullptr;
}

void RoiManager::SelectOnly(const QString& roiId)
{
    for (RoiData* roi : m_roiList)
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
}

bool RoiManager::SaveToJson(const QString& filePath)
{
    QJsonArray roiArray;

    for (RoiData* roi : m_roiList)
    {
        QJsonObject obj;
        obj["roiId"] = roi->GetRoiId();
        obj["roiName"] = roi->GetRoiName();
        obj["roiType"] = roi->GetRoiType();
        obj["centerX"] = roi->GetCenterX();
        obj["centerY"] = roi->GetCenterY();
        obj["roiWidth"] = roi->GetRoiWidth();
        obj["roiHeight"] = roi->GetRoiHeight();
        obj["angle"] = roi->GetAngle();
        obj["color"] = roi->GetColor();
        obj["selected"] = roi->GetSelected();
        roiArray.append(obj);
    }

    QJsonObject root;
    root["rois"] = roiArray;

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

    ClearAllRois();

    QJsonObject root = doc.object();
    QJsonArray roiArray = root["rois"].toArray();

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
        roi->SetColor(obj["color"].toString());
        roi->SetSelected(obj["selected"].toBool(false));

        m_roiList.append(roi);
    }

    emit RoiListChanged();
    return true;
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

        Threshold(topReducedDomain, &topRegion, 0, 240);
        Threshold(downReducedDomain, &downRegion, 0, 240);

        Connection(topRegion, &topConnectedRegion);
        Connection(downRegion, &downConnectedRegion);

        SelectShape(topConnectedRegion,
                    &topSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(0).Append(1).Append(30),
                    HTuple(3).Append(20).Append(300));

        SelectShape(downConnectedRegion,
                    &downSelectedRegion,
                    HTuple("ratio").Append("height").Append("width"),
                    "and",
                    HTuple(3).Append(90).Append(1),
                    HTuple(40).Append(2000).Append(15));

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
