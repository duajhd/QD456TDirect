#include <CameraViewModel.h>

#include <QDebug>
#include <cmath>

CameraViewModel::CameraViewModel(int width,
                                 int height,
                                 QString serialNum,
                                 int channel,
                                 int cameraIndex,
                                 double exposureTime,
                                 double gain,
                                 const DetectionAlgorithmParams& algorithmParams,
                                 moodycamel::ReaderWriterQueue<int>* dropQueue,
                                 QObject* parent)
    : QObject(parent),
      m_width(width),
      m_height(height),
      m_channel(channel),
      m_cameraIndex(cameraIndex),
      m_serialNum(serialNum),
      m_exposureTime(static_cast<float>(exposureTime)),
      m_gain(static_cast<float>(gain)),
      m_dropQueue(dropQueue)
{
    m_detectionConfig.algorithmParams = algorithmParams;
}

CameraViewModel::~CameraViewModel()
{
    CleanupThreads();
}

void CameraViewModel::Init()
{
    if (m_initialize) {
        return;
    }

    const qsizetype frameBytes =
        static_cast<qsizetype>(m_width) *
        static_cast<qsizetype>(m_height) *
        static_cast<qsizetype>(m_channel);

    if (frameBytes <= 0) {
        qWarning() << "Invalid camera buffer size"
                   << m_serialNum << m_width << m_height << m_channel;
        setStatusText(QStringLiteral("参数错误"));
        return;
    }

    m_imageStorage = std::make_unique<unsigned char[]>(
        static_cast<std::size_t>(frameBytes) * FrameSlotCount);
    m_imageBuffer = m_imageStorage.get();
    m_frameQueue = std::make_unique<moodycamel::ReaderWriterQueue<int>>(FrameSlotCount);

    m_camera = std::make_unique<HikCamera>(m_serialNum, m_width, m_height);
    const int res = m_camera->Init();
    if (res != MV_OK) {
        qWarning() << "Camera init failed" << m_serialNum << QString("0x%1").arg(res, 0, 16);
        setStatusText(QStringLiteral("连接失败"));
        return;
    }

    const int paramRes = m_camera->SetCameraParameters(m_exposureTime, m_gain);
    if (paramRes != MV_OK) {
        qWarning() << "Camera parameter apply failed"
                   << m_serialNum
                   << QString("0x%1").arg(paramRes, 0, 16)
                   << m_camera->LastErrorString();
        setStatusText(QStringLiteral("参数设置失败"));
        return;
    }

    m_captureWorker = new CaptureWorker(m_width, m_height, m_channel, m_camera.get(), m_imageBuffer, m_frameQueue.get());
    m_processWorker = new ProcessWorker(m_width,
                                        m_height,
                                        m_channel,
                                        m_imageBuffer,
                                        m_frameQueue.get(),
                                        m_cameraIndex,
                                        m_dropQueue);
    m_processWorker->SetDetectionConfig(m_detectionConfig);
    m_captureThread = new QThread(this);
    m_processThread = new QThread(this);

    m_captureWorker->moveToThread(m_captureThread);
    m_processWorker->moveToThread(m_processThread);

    connect(m_captureThread, &QThread::started, m_captureWorker, &CaptureWorker::startWork);
    connect(m_processThread, &QThread::started, m_processWorker, &ProcessWorker::StartWork);

    connect(m_captureWorker, &CaptureWorker::finished, m_captureThread, &QThread::quit);
    connect(m_processWorker, &ProcessWorker::finished, m_processThread, &QThread::quit);
    connect(m_captureWorker, &CaptureWorker::finished, m_captureWorker, &QObject::deleteLater);
    connect(m_processWorker, &ProcessWorker::finished, m_processWorker, &QObject::deleteLater);

    connect(m_processWorker,
            &ProcessWorker::frameUpdated,
            this,
            [this](int frameId) {
                setFrameId(frameId);
                emit frameUpdated(frameId);
            },
            Qt::QueuedConnection);
    connect(m_processWorker,
            &ProcessWorker::algorithmFrameAccepted,
            this,
            &CameraViewModel::incrementAlgorithmFrameCount,
            Qt::QueuedConnection);
    connect(m_processWorker,
            &ProcessWorker::rejectDiffUpdated,
            this,
            &CameraViewModel::setRejectDiffValue,
            Qt::QueuedConnection);

    m_initialize = true;
    setStatusText(QStringLiteral("已连接"));
}

void CameraViewModel::Start()
{
    if (!m_initialize) {
        Init();
    }

    if (!m_initialize || m_started) {
        return;
    }

    m_processThread->start();
    m_captureThread->start();
    m_started = true;
    setStatusText(QStringLiteral("运行中"));
}

void CameraViewModel::Stop()
{
    if (!m_started) {
        return;
    }

    CleanupThreads();
    setStatusText(QStringLiteral("已停止"));
}

void CameraViewModel::SetDetectionConfig(const DetectionRoiConfig& config)
{
    const int currentDropThres = m_detectionConfig.dropThres;
    m_detectionConfig = config;
    m_detectionConfig.dropThres = currentDropThres;
    if (m_processWorker) {
        m_processWorker->SetDetectionConfig(m_detectionConfig);
    }
}

bool CameraViewModel::SetCameraParameter(double exposureTime, double gain, int dropThres)
{
    if (!std::isfinite(exposureTime) || !std::isfinite(gain) || exposureTime <= 0.0 || gain < 0.0 || dropThres < 0) {
        qWarning() << "Invalid camera parameter"
                   << m_serialNum
                   << exposureTime
                   << gain
                   << dropThres;
        return false;
    }

    m_exposureTime = static_cast<float>(exposureTime);
    m_gain = static_cast<float>(gain);
    m_detectionConfig.dropThres = dropThres;
    emit cameraParametersChanged();

    if (m_processWorker) {
        m_processWorker->SetDetectionConfig(m_detectionConfig);
    }

    if (!m_camera || !m_camera->IsOpened()) {
        return true;
    }

    const int res = m_camera->SetCameraParameters(m_exposureTime, m_gain);
    if (res != MV_OK) {
        qWarning() << "Set camera parameter failed"
                   << m_serialNum
                   << QString("0x%1").arg(res, 0, 16)
                   << m_camera->LastErrorString();
        setStatusText(QStringLiteral("参数设置失败"));
        return false;
    }

    setStatusText(m_started ? QStringLiteral("运行中") : QStringLiteral("已连接"));
    return true;
}

void CameraViewModel::CleanupThreads()
{
    const bool captureThreadRunning = m_captureThread && m_captureThread->isRunning();
    const bool processThreadRunning = m_processThread && m_processThread->isRunning();

    if (m_captureWorker) {
        m_captureWorker->stopWork();
    }
    if (m_processWorker) {
        m_processWorker->StopWork();
    }

    if (captureThreadRunning) {
        m_captureThread->quit();
        m_captureThread->wait();
    }
    if (processThreadRunning) {
        m_processThread->quit();
        m_processThread->wait();
    }

    if (m_captureWorker) {
        if (!captureThreadRunning) {
            delete m_captureWorker;
        }
        m_captureWorker = nullptr;
    }
    if (m_processWorker) {
        if (!processThreadRunning) {
            delete m_processWorker;
        }
        m_processWorker = nullptr;
    }

    if (m_captureThread) {
        delete m_captureThread;
        m_captureThread = nullptr;
    }
    if (m_processThread) {
        delete m_processThread;
        m_processThread = nullptr;
    }

    emit frameSourceReset();

    m_started = false;
    m_initialize = false;
    m_camera.reset();
    m_frameQueue.reset();
    m_imageStorage.reset();
    m_imageBuffer = nullptr;
}

QString CameraViewModel::name() const
{
    return QStringLiteral("相机 %1").arg(m_cameraIndex + 1);
}

QString CameraViewModel::serialNumber() const
{
    return m_serialNum;
}

double CameraViewModel::exposureTime() const
{
    return m_exposureTime;
}

double CameraViewModel::gain() const
{
    return m_gain;
}

QString CameraViewModel::statusText() const
{
    return m_statusText;
}

int CameraViewModel::frameId() const
{
    return m_frameId;
}

int CameraViewModel::algorithmFrameCount() const
{
    return m_algorithmFrameCount;
}

int CameraViewModel::rejectFrameCount() const
{
    return m_rejectFrameCount;
}

double CameraViewModel::rejectDiffValue() const
{
    return m_rejectDiffValue;
}

int CameraViewModel::width() const
{
    return m_width;
}

int CameraViewModel::height() const
{
    return m_height;
}

void CameraViewModel::setStatusText(const QString& statusText)
{
    if (m_statusText == statusText) {
        return;
    }

    m_statusText = statusText;
    emit statusTextChanged();
}

void CameraViewModel::setFrameId(int frameId)
{
    if (m_frameId == frameId) {
        return;
    }

    m_frameId = frameId;
    emit frameIdChanged();
}

void CameraViewModel::incrementAlgorithmFrameCount()
{
    ++m_algorithmFrameCount;
    emit algorithmFrameCountChanged();
}

void CameraViewModel::setAlgorithmFrameCount(int value)
{
    if (m_algorithmFrameCount == value) {
        return;
    }

    m_algorithmFrameCount = value;
    emit algorithmFrameCountChanged();
}

void CameraViewModel::incrementRejectFrameCount()
{
    ++m_rejectFrameCount;
    emit rejectFrameCountChanged();
}

void CameraViewModel::setRejectDiffValue(double value)
{
    if (qFuzzyCompare(m_rejectDiffValue, value)) {
        return;
    }

    m_rejectDiffValue = value;
    emit rejectDiffValueChanged();
}
