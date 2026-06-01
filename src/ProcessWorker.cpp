#include <ProcessWorker.h>

#include <Algorithm.h>
#include <CameraViewModel.h>
#include <QMutexLocker>
#include <QThread>
#include <QtMath>

ProcessWorker::ProcessWorker(int width,
                             int height,
                             int channel,
                             unsigned char* imageByteAddr,
                             moodycamel::ReaderWriterQueue<int>* queue,
                             int cameraIndex,
                             moodycamel::ReaderWriterQueue<int>* dropQueue,
                             QObject* parent)
    : QObject(parent),
      m_width(width),
      m_height(height),
      m_channel(channel),
      m_cameraIndex(cameraIndex),
      m_imageByteAddr(imageByteAddr),
      bufferSize(static_cast<std::size_t>(width) *
                 static_cast<std::size_t>(height) *
                 static_cast<std::size_t>(channel)),
      m_queue(queue),
      m_dropQueue(dropQueue)
{
}

void ProcessWorker::StartWork()
{
    if (!m_imageByteAddr || !m_queue || bufferSize == 0) {
        emit finished();
        return;
    }

    m_running.store(true, std::memory_order_release);

    while (m_running.load(std::memory_order_acquire)) {
        int frameId = 0;

        if (!m_queue->try_dequeue(frameId)) {
            QThread::msleep(1);
            continue;
        }

        const int slotIndex = frameId % CameraViewModel::FrameSlotCount;
        unsigned char* imageByte = m_imageByteAddr + static_cast<std::size_t>(slotIndex) * bufferSize;

        HalconCpp::GenImage1(&DetecImage,
                             "byte",
                             m_width,
                             m_height,
                             HalconCpp::HTuple(reinterpret_cast<Hlong>(imageByte)));

        DetectionRoiConfig config;
        HalconCpp::HObject topRectangle;
        HalconCpp::HObject downRectangle;
        bool hasDetectionRois = false;
        {
            QMutexLocker locker(&m_configMutex);
            config = m_detectionConfig;
            topRectangle = m_topRectangle;
            downRectangle = m_downRectangle;
            hasDetectionRois = m_hasDetectionRois;
        }

        if (!config.valid || !hasDetectionRois) {
            emit frameUpdated(frameId);
            continue;
        }

        const VisionAlgorithm::DirectionResult result =
            VisionAlgorithm::DirectionRecognize(DetecImage,
                                                topRectangle,
                                                downRectangle,
                                                config.top.offsetX,
                                                config.top.offsetY,
                                                config.down.offsetX,
                                                config.down.offsetY,
                                                config.top.offsetRotation,
                                                config.down.offsetRotation,
                                                config.algorithmParams,
                                                config.dropThres);

        if (result == VisionAlgorithm::DirectionResult::Reject && m_dropQueue) {
            m_dropQueue->try_enqueue(m_cameraIndex);
        }

        emit frameUpdated(frameId);
    }

    emit finished();
}

void ProcessWorker::StopWork()
{
    m_running.store(false, std::memory_order_release);
}

void ProcessWorker::SetDetectionConfig(const DetectionRoiConfig& config)
{
    QMutexLocker locker(&m_configMutex);
    m_detectionConfig = config;
    m_hasDetectionRois = false;
    m_topRectangle = HalconCpp::HObject();
    m_downRectangle = HalconCpp::HObject();

    if (!m_detectionConfig.valid) {
        return;
    }

    try {
        HalconCpp::GenRectangle2(&m_topRectangle,
                                 m_detectionConfig.top.centerY,
                                 m_detectionConfig.top.centerX,
                                 qDegreesToRadians(m_detectionConfig.top.angle),
                                 m_detectionConfig.top.width * 0.5,
                                 m_detectionConfig.top.height * 0.5);
        HalconCpp::GenRectangle2(&m_downRectangle,
                                 m_detectionConfig.down.centerY,
                                 m_detectionConfig.down.centerX,
                                 qDegreesToRadians(m_detectionConfig.down.angle),
                                 m_detectionConfig.down.width * 0.5,
                                 m_detectionConfig.down.height * 0.5);
        m_hasDetectionRois = true;
    }
    catch (const HalconCpp::HException&) {
        m_hasDetectionRois = false;
    }
}

void ProcessWorker::OnFrameArrived(int frameId)
{
    if (m_queue) {
        m_queue->try_enqueue(frameId);
    }
}
