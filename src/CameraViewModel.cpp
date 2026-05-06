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
    Stop();
    delete m_captureWorker;
    delete m_processWorker;
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

    connect(m_processWorker,
            &ProcessWorker::frameUpdated,
            this,
            &CameraViewModel::frameUpdated,
            Qt::QueuedConnection);

    m_initialize = true;
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
}

void CameraViewModel::Stop()
{
    if (!m_started) {
        return;
    }

    if (m_captureWorker) {
        m_captureWorker->stopWork();
    }
    if (m_processWorker) {
        m_processWorker->StopWork();
    }

    if (m_captureThread && m_captureThread->isRunning()) {
        m_captureThread->quit();
        m_captureThread->wait();
    }
    if (m_processThread && m_processThread->isRunning()) {
        m_processThread->quit();
        m_processThread->wait();
    }

    m_started = false;
}
