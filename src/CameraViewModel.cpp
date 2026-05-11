#include <CameraViewModel.h>

#include <QDebug>

CameraViewModel::CameraViewModel(int width,
                                 int height,
                                 QString serialNum,
                                 int channel,
                                 int cameraIndex,
                                 moodycamel::ReaderWriterQueue<int>* dropQueue,
                                 QObject* parent)
    : QObject(parent),
      m_width(width),
      m_height(height),
      m_channel(channel),
      m_cameraIndex(cameraIndex),
      m_serialNum(serialNum),
      m_dropQueue(dropQueue)
{
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

    m_captureWorker = new CaptureWorker(m_width, m_height, m_channel, m_camera.get(), m_imageBuffer, m_frameQueue.get());
    m_processWorker = new ProcessWorker(m_width,
                                        m_height,
                                        m_channel,
                                        m_imageBuffer,
                                        m_frameQueue.get(),
                                        m_cameraIndex,
                                        m_dropQueue);
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

QString CameraViewModel::statusText() const
{
    return m_statusText;
}

int CameraViewModel::frameId() const
{
    return m_frameId;
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
