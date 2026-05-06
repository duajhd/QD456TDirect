#include <ProcessWorker.h>

#include <Algorithm.h>
#include <CameraViewModel.h>
#include <QThread>

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

        const VisionAlgorithm::DirectionResult result =
            VisionAlgorithm::DirectionRecognize(DetecImage,
                                                TopRegion,
                                                DownRegion,
                                                m_offsetX,
                                                m_offsetY,
                                                m_offsetXDown,
                                                m_offsetYDown,
                                                m_dropThres);

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

void ProcessWorker::OnFrameArrived(int frameId)
{
    if (m_queue) {
        m_queue->try_enqueue(frameId);
    }
}
