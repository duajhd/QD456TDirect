#include <ProcessWorker.h>

#include <Algorithm.h>
#include <CameraViewModel.h>
#include <QMutexLocker>
#include <QDebug>
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

        VisionAlgorithm::DirectionDebugInfo debugInfo;
        using DirectionRecognizeFn = VisionAlgorithm::DirectionResult (*)(
            const HalconCpp::HObject&,
            const HalconCpp::HObject&,
            const HalconCpp::HObject&,
            int,
            int,
            int,
            int,
            double,
            double,
            double,
            double,
            const DetectionAlgorithmParams&,
            int,
            VisionAlgorithm::DirectionDebugInfo*);

        DirectionRecognizeFn directionRecognize = VisionAlgorithm::DirectionRecognizeCamera1;
        switch (m_cameraIndex) {
        case 0:
            directionRecognize = VisionAlgorithm::DirectionRecognizeCamera1;
            break;
        case 1:
            directionRecognize = VisionAlgorithm::DirectionRecognizeCamera2;
            break;
        case 2:
            directionRecognize = VisionAlgorithm::DirectionRecognizeCamera3;
            break;
        case 3:
            directionRecognize = VisionAlgorithm::DirectionRecognizeCamera4;
            break;
        default:
            qWarning() << "Invalid camera index for DirectionRecognize" << m_cameraIndex;
            break;
        }

        const VisionAlgorithm::DirectionResult result =
            directionRecognize(DetecImage,
                               topRectangle,
                               downRectangle,
                               config.top.offsetX,
                               config.top.offsetY,
                               config.down.offsetX,
                               config.down.offsetY,
                               config.top.offsetCircleRadius,
                               config.down.offsetCircleRadius,
                               config.top.offsetRotation,
                               config.down.offsetRotation,
                               config.algorithmParams,
                               config.dropThres,
                               &debugInfo);
        emit algorithmRegionCountsUpdated(debugInfo.topConnectedCount,
                                          debugInfo.downConnectedCount,
                                          debugInfo.downSelectedCount);
        /*
        qDebug() << "DirectionRecognize debug"
                 << "camera" << m_cameraIndex
                 << "topConnected" << debugInfo.topConnectedCount
                 << "downConnected" << debugInfo.downConnectedCount
                 << "topSelected" << debugInfo.topSelectedCount
                 << "downSelected" << debugInfo.downSelectedCount
                 << "topThresholdArea" << debugInfo.topRegionArea
                 << "downThresholdArea" << debugInfo.downRegionArea
                 << "downROI center=(" << config.down.centerX << "," << config.down.centerY << ")"
                 << "size=(" << config.down.width << "," << config.down.height << ")"
                 << "angle" << config.down.angle
                 << "downThreshold=(" << config.algorithmParams.downThresholdMin << "," << config.algorithmParams.downThresholdMax << ")"
                 << "downRatio=(" << config.algorithmParams.downRatioMin << "," << config.algorithmParams.downRatioMax << ")"
                 << "downHeight=(" << config.algorithmParams.downHeightMin << "," << config.algorithmParams.downHeightMax << ")"
                 << "downWidth=(" << config.algorithmParams.downWidthMin << "," << config.algorithmParams.downWidthMax << ")";
        */

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
