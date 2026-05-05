#include <CaptureWorker.h>
#include <QThread>
CaptureWorker::CaptureWorker(int width,int height,int channel,HikCamera * camera,unsigned char* imageByteAddr,
                             moodycamel::ReaderWriterQueue<int>* queue,QObject* parent):m_width(width),
    m_height(height),
    m_channel(channel),
    m_camera(camera),
    m_imageByteAddr(imageByteAddr),
    m_queue(queue)

{
    // 安全计算 buffer 大小
    std::size_t bufferSize = static_cast<std::size_t>(m_width)
                             * static_cast<std::size_t>(m_height)
                             * static_cast<std::size_t>(m_channel);
};
void CaptureWorker::startWork()
{
    if(!m_camera){
        return;
    }
    int m_frameId = 0;
    m_running.store(true, std::memory_order_release);
    while (m_running.load(std::memory_order_acquire))
    {
        unsigned char* imageByteAddr = nullptr;
        const int res = m_camera->GrabOneFrame(imageByteAddr,bufferSize,20);
        if(res == MV_OK)
        {
            m_frameId++;

            memcpy(imageByteAddr,imageByteAddr,bufferSize);
            emit frameArrived(m_frameId);
        }
    };






    emit finished();
};
void CaptureWorker::stopWork()
{
    m_running.store(false, std::memory_order_release);
};
