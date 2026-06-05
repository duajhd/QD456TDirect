#include <MainViewWindow.h>
#include "ConfigReader.h"
#include <QDebug>

MianViewModel::MianViewModel(QObject* parent)
    : QObject(parent)
{
}

MianViewModel::~MianViewModel()
{
    StopDetect();
    if (m_gpioController) {
        delete m_gpioController;
        m_gpioController = nullptr;
    }
}

void MianViewModel::Initialize(){

    if (!m_dropQueue) {
        m_dropQueue = std::make_unique<moodycamel::ReaderWriterQueue<int>>(1024);
    }

    if (!m_gpioController) {
        m_gpioController = new GPIOController(m_dropQueue.get());
        m_gpioThread = new QThread(this);
    }

    QVector<CameraConfig> cameras;
    QString error;

    const QString configPath = "D:/zhijain/QT456TDirect/config/camera_config.json";

    bool ok = ConfigReader::loadCameraConfig(
        configPath,
        cameras,
        &error
        );

    if (!ok) {
        qDebug() << "读取相机配置失败:" << error;
        return;
    }

    if (!ConfigReader::loadAlgorithmParams(configPath, m_algorithmParams, &error)) {
        qDebug() << "Read algorithm params failed:" << error;
        return;
    }

    for (int i = 0; i < cameras.size(); ++i) {
        const CameraConfig& cfg = cameras[i];
        auto* vm = new CameraViewModel(cfg.imageWidth,
                                       cfg.imageHeight,
                                       cfg.serialNumber,
                                       cfg.channel,
                                       i,
                                       cfg.exposureTime,
                                       cfg.gain,
                                       m_algorithmParams,
                                       m_dropQueue.get(),
                                       this);
        m_cameras.push_back(vm);
        auto* roiManager = new RoiManager(this);
        roiManager->SetAlgorithmParams(m_algorithmParams);
        m_roiManagers.push_back(roiManager);
        qDebug() << "Camera" << i
                 << "SN:" << cfg.serialNumber
                 << "Width:" << cfg.imageWidth
                 << "Height:" << cfg.imageHeight
                 << "Channel:" << cfg.channel
                 << "Exposure:" << cfg.exposureTime
                 << "Gain:" << cfg.gain;
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

bool MianViewModel::ApplyRoiConfig(int cameraIndex)
{
    if (cameraIndex < 0 ||
        cameraIndex >= m_cameras.size() ||
        cameraIndex >= m_roiManagers.size()) {
        return false;
    }

    DetectionRoiConfig config;
    if (!m_roiManagers[cameraIndex] ||
        !m_roiManagers[cameraIndex]->BuildDetectionConfig(&config)) {
        qWarning() << "Invalid ROI config for camera" << cameraIndex;
        return false;
    }

    if (!m_cameras[cameraIndex]) {
        return false;
    }

    m_cameras[cameraIndex]->SetDetectionConfig(config);
    return true;
}

bool MianViewModel::SetCameraParameter(int cameraIndex, double exposureTime, double gain, int dropThres)
{
    if (cameraIndex < 0 || cameraIndex >= m_cameras.size() || !m_cameras[cameraIndex]) {
        return false;
    }

    return m_cameras[cameraIndex]->SetCameraParameter(exposureTime, gain, dropThres);
}

void MianViewModel::StartDetect()
{
    if (m_isRunning) {
        return;
    }

    if (!m_gpioController) {
        m_gpioController = new GPIOController(m_dropQueue.get());
    }
    if (!m_gpioThread) {
        m_gpioThread = new QThread(this);
    }

    if (m_gpioThread && !m_gpioThread->isRunning()) {
        m_gpioController->moveToThread(m_gpioThread);
        connect(m_gpioThread, &QThread::started, m_gpioController, &GPIOController::startWork, Qt::UniqueConnection);
        connect(m_gpioController, &GPIOController::finished, m_gpioThread, &QThread::quit, Qt::UniqueConnection);
        connect(m_gpioController, &GPIOController::finished, m_gpioController, &QObject::deleteLater, Qt::UniqueConnection);
        m_gpioThread->start();
    }

    for (int i = 0; i < m_cameras.size(); ++i) {
        ApplyRoiConfig(i);
    }

    for (CameraViewModel* camera : m_cameras) {
        if (camera) {
            camera->Start();
        }
    }

    m_isRunning = true;
    emit isRunningChanged();
}

void MianViewModel::StopDetect()
{
    const bool wasRunning = m_isRunning;

    for (CameraViewModel* camera : m_cameras) {
        if (camera) {
            camera->Stop();
        }
    }

    if (m_gpioController) {
        m_gpioController->StopWork();
    }
    if (m_gpioThread && m_gpioThread->isRunning()) {
        m_gpioThread->quit();
        m_gpioThread->wait();
        m_gpioController = nullptr;
    } else if (m_gpioController) {
        delete m_gpioController;
        m_gpioController = nullptr;
    }

    if (m_gpioThread) {
        delete m_gpioThread;
        m_gpioThread = nullptr;
    }

    m_isRunning = false;
    if (wasRunning) {
        emit isRunningChanged();
    }
}

void MianViewModel::ToggleDetect()
{
    if (m_isRunning) {
        StopDetect();
    } else {
        StartDetect();
    }
}

bool MianViewModel::isRunning() const
{
    return m_isRunning;
}
