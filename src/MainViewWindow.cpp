#include <MainViewWindow.h>
#include "ConfigReader.h"
void MianViewModel::Initialize(){

    QVector<CameraConfig> cameras;
    QString error;

    bool ok = ConfigReader::loadCameraConfig(
        "D:/zhijain/QT456TDirect/config/camera_config.json",
        cameras,
        &error
        );

    if (!ok) {
        qDebug() << "读取相机配置失败:" << error;
        return;
    }

    for (int i = 0; i < cameras.size(); ++i) {
        const CameraConfig& cfg = cameras[i];
        auto* vm = new CameraViewModel(cfg.imageWidth,cfg.imageHeight,cfg.serialNumber,cfg.channel,this);
        m_cameras.push_back(vm);
        auto* roiManager = new RoiManager(this);
        m_roiManagers.push_back(roiManager);
        qDebug() << "Camera" << i
                 << "SN:" << cfg.serialNumber
                 << "Width:" << cfg.imageWidth
                 << "Height:" << cfg.imageHeight
                 << "Channel:" << cfg.channel;
    }

};
QObject* MianViewModel::getCamera(int count) const{
    if(count<0 || count>=m_cameras.size())
    {
        return nullptr;
    }
    return m_cameras[count];


};
QObject* MianViewModel::getRoiManager(int count) const{
    if(count<0 || count>=m_roiManagers.size())
    {
        return nullptr;
    }
    return m_roiManagers[count];
};
