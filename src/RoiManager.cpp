#include "RoiManager.h"
#include <QUuid>

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
