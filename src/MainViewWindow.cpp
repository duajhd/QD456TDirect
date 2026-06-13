#include <MainViewWindow.h>
#include "ConfigReader.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

double JsonDoubleOrDefault(const QJsonObject& obj, const QString& key, double defaultValue)
{
    return obj.contains(key) ? obj.value(key).toDouble(defaultValue) : defaultValue;
}

int JsonIntOrDefault(const QJsonObject& obj, const QString& key, int defaultValue)
{
    return obj.contains(key) ? obj.value(key).toInt(defaultValue) : defaultValue;
}

void LoadCameraUiParamsFromJson(const QString& filePath,
                                double* exposureTime,
                                double* gain,
                                int* dropThres)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return;
    }

    const QJsonObject root = doc.object();
    if (exposureTime) {
        *exposureTime = JsonDoubleOrDefault(root, "exposureTime", *exposureTime);
    }
    if (gain) {
        *gain = JsonDoubleOrDefault(root, "gain", *gain);
    }
    if (dropThres) {
        *dropThres = JsonIntOrDefault(root, "dropThres", *dropThres);
    }
}

bool SaveCameraUiParamsToJson(const QString& filePath,
                              double exposureTime,
                              double gain,
                              int dropThres)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    root["exposureTime"] = exposureTime;
    root["gain"] = gain;
    root["dropThres"] = dropThres;

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

}

MianViewModel::MianViewModel(QObject* parent)
    : QObject(parent)
{
}

MianViewModel::~MianViewModel()
{
    StopDetect();
}

void MianViewModel::Initialize(){
    QVector<CameraConfig> cameras;
    QString error;

    const QString configPath = QCoreApplication::applicationDirPath() + "/config/camera_config.json";
    const QString configDir = QFileInfo(configPath).absolutePath();
    LoadGpioConfig(configDir);

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
        double exposureTime = cfg.exposureTime;
        double gain = cfg.gain;
        int dropThres = 27;
        LoadCameraUiParamsFromJson(cfg.path, &exposureTime, &gain, &dropThres);
        const int gpioControllerIndex = FindGpioControllerIndexForCamera(i);
        moodycamel::ReaderWriterQueue<int>* dropQueue =
            gpioControllerIndex >= 0 && gpioControllerIndex < static_cast<int>(m_dropQueues.size())
                ? m_dropQueues[static_cast<std::size_t>(gpioControllerIndex)].get()
                : nullptr;

        auto* vm = new CameraViewModel(cfg.imageWidth,
                                       cfg.imageHeight,
                                       cfg.serialNumber,
                                       cfg.channel,
                                       i,
                                       exposureTime,
                                       gain,
                                       dropThres,
                                       m_algorithmParams,
                                       dropQueue,
                                       this);
        m_cameras.push_back(vm);
        auto* roiManager = new RoiManager(this);
        roiManager->SetAlgorithmParams(m_algorithmParams);
        m_roiManagers.push_back(roiManager);
        m_roiConfigPaths.push_back(cfg.path);
        if (!cfg.path.isEmpty()) {
            if (roiManager->LoadFromJson(cfg.path)) {
                ApplyRoiConfig(i);
                qDebug() << "Loaded ROI config for camera" << i << cfg.path;
            } else {
                qWarning() << "Load ROI config failed for camera" << i << cfg.path;
            }
        }
        qDebug() << "Camera" << i
                 << "SN:" << cfg.serialNumber
                 << "Width:" << cfg.imageWidth
                 << "Height:" << cfg.imageHeight
                 << "Channel:" << cfg.channel
                 << "Exposure:" << exposureTime
                 << "Gain:" << gain
                 << "DropThres:" << dropThres
                 << "GPIO controller:" << gpioControllerIndex
                 << "ROI config:" << cfg.path;
    }

};

void MianViewModel::LoadGpioConfig(const QString& configDir)
{
    m_gpioConfigs.clear();
    m_cameraGpioControllerIndexes.clear();
    m_dropQueues.clear();

    QString error;
    const QString gpioConfigPath = QDir(configDir).filePath(QStringLiteral("gpio_config.json"));
    if (!ConfigReader::loadGPIOConfig(gpioConfigPath, m_gpioConfigs, &error)) {
        qWarning() << "Load GPIO config failed, using default two-controller routes:" << error;

        GPIOControllerConfig first;
        first.deviceIndex = 0;
        first.routes.append(GPIOCameraRoute{0, 0});
        first.routes.append(GPIOCameraRoute{1, 1});

        GPIOControllerConfig second;
        second.deviceIndex = 1;
        second.routes.append(GPIOCameraRoute{2, 0});
        second.routes.append(GPIOCameraRoute{3, 1});

        m_gpioConfigs.append(first);
        m_gpioConfigs.append(second);
    }

    for (int i = 0; i < m_gpioConfigs.size(); ++i) {
        m_dropQueues.push_back(std::make_unique<moodycamel::ReaderWriterQueue<int>>(1024));
    }

    for (int controllerIndex = 0; controllerIndex < m_gpioConfigs.size(); ++controllerIndex) {
        for (const GPIOCameraRoute& route : m_gpioConfigs[controllerIndex].routes) {
            while (m_cameraGpioControllerIndexes.size() <= route.cameraIndex) {
                m_cameraGpioControllerIndexes.append(-1);
            }
            m_cameraGpioControllerIndexes[route.cameraIndex] = controllerIndex;
        }
    }
}

int MianViewModel::FindGpioControllerIndexForCamera(int cameraIndex) const
{
    if (cameraIndex < 0 || cameraIndex >= m_cameraGpioControllerIndexes.size()) {
        return -1;
    }
    return m_cameraGpioControllerIndexes[cameraIndex];
}
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

    if (cameraIndex < m_roiManagers.size() && m_roiManagers[cameraIndex]) {
        m_roiManagers[cameraIndex]->SetDropThres(dropThres);
    }

    const bool applied = m_cameras[cameraIndex]->SetCameraParameter(exposureTime, gain, dropThres);
    if (!applied) {
        return false;
    }

    if (cameraIndex < m_roiConfigPaths.size() && !m_roiConfigPaths[cameraIndex].isEmpty()) {
        if (!SaveCameraUiParamsToJson(m_roiConfigPaths[cameraIndex], exposureTime, gain, dropThres)) {
            qWarning() << "Save camera UI params failed" << cameraIndex << m_roiConfigPaths[cameraIndex];
            return false;
        }
    }

    return true;
}

bool MianViewModel::SetRejectAll(int cameraIndex, bool rejectAll)
{
    if (cameraIndex < 0 || cameraIndex >= m_cameras.size() || !m_cameras[cameraIndex]) {
        return false;
    }

    m_cameras[cameraIndex]->SetRejectAll(rejectAll);
    return true;
}

void MianViewModel::EnsureGpioRuntime()
{
    if (!m_gpioControllers.isEmpty()) {
        return;
    }

    for (int i = 0; i < m_gpioConfigs.size(); ++i) {
        if (i >= static_cast<int>(m_dropQueues.size())) {
            qWarning() << "Missing drop queue for GPIO controller" << i;
            continue;
        }

        auto* controller = new GPIOController(m_dropQueues[static_cast<std::size_t>(i)].get(),
                                              m_gpioConfigs[i]);
        auto* thread = new QThread(this);
        controller->moveToThread(thread);

        connect(controller,
                &GPIOController::dropped,
                this,
                [this](int cameraIndex) {
                    if (cameraIndex < 0 || cameraIndex >= m_cameras.size()) {
                        qWarning() << "Invalid dropped camera index" << cameraIndex;
                        return;
                    }

                    if (m_cameras[cameraIndex]) {
                        m_cameras[cameraIndex]->incrementRejectFrameCount();
                    }
                },
                Qt::QueuedConnection);
        connect(thread, &QThread::started, controller, &GPIOController::startWork, Qt::UniqueConnection);
        connect(controller, &GPIOController::finished, thread, &QThread::quit, Qt::UniqueConnection);
        connect(controller, &GPIOController::finished, controller, &QObject::deleteLater, Qt::UniqueConnection);

        m_gpioControllers.append(controller);
        m_gpioThreads.append(thread);
        thread->start();
    }
}

void MianViewModel::StopGpioRuntime()
{
    for (const QPointer<GPIOController>& controller : m_gpioControllers) {
        if (controller) {
            controller->StopWork();
        }
    }

    for (QThread* thread : m_gpioThreads) {
        if (thread && thread->isRunning()) {
            thread->quit();
            thread->wait();
        }
    }

    for (QThread* thread : m_gpioThreads) {
        delete thread;
    }

    m_gpioControllers.clear();
    m_gpioThreads.clear();
}

void MianViewModel::StartDetect()
{
    if (m_isRunning) {
        return;
    }

    EnsureGpioRuntime();

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

    StopGpioRuntime();

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
