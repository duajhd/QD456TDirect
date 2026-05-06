#include <CaptureWorker.h>

#include <CameraViewModel.h>
#include <cstring>

CaptureWorker::CaptureWorker(int width,
                             int height,
                             int channel,
                             HikCamera* camera,
                             unsigned char* imageByteAddr,
                             moodycamel::ReaderWriterQueue<int>* queue,
                             QObject* parent)
    : QObject(parent),
      m_width(width),
      m_height(height),
      m_channel(channel),
      m_camera(camera),
      m_imageByteAddr(imageByteAddr),
      bufferSize(static_cast<std::size_t>(width) *
                 static_cast<std::size_t>(height) *
                 static_cast<std::size_t>(channel)),
      m_queue(queue)
{
}

void CaptureWorker::startWork()
{
    if (!m_camera || !m_imageByteAddr || !m_queue || bufferSize == 0) {
        emit finished();
        return;
    }

    m_running.store(true, std::memory_order_release);

    while (m_running.load(std::memory_order_acquire)) {
        const int slotIndex = m_frameId % CameraViewModel::FrameSlotCount;
        unsigned char* frameAddr = m_imageByteAddr + static_cast<std::size_t>(slotIndex) * bufferSize;

        const int res = m_camera->GrabOneFrame(frameAddr, bufferSize, 20);
        if (res == MV_OK) {
            const int frameId = m_frameId++;
            m_queue->try_enqueue(frameId);
            emit frameArrived(frameId);
        }
    }

    emit finished();
}

void CaptureWorker::stopWork()
{
    m_running.store(false, std::memory_order_release);
}
