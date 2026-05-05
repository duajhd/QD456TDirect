#include <ProcessWorker.h>
#include <QThread>
#include <QDebug>
#include <Algorithm.h>
ProcessWorker::ProcessWorker(int width,int height,int channel, unsigned char* imageByteAddr,
                             moodycamel::ReaderWriterQueue<int>* queue,QObject* parent):
    QObject(parent),
    m_width(width),
    m_height(height),
    m_channel(channel),
    m_imageByteAddr(imageByteAddr),
    m_queue(queue)
{
    // 安全计算 buffer 大小
    std::size_t bufferSize = static_cast<std::size_t>(m_width)
                             * static_cast<std::size_t>(m_height)
                             * static_cast<std::size_t>(m_channel);
};
void ProcessWorker::StartWork()
{
    m_running.store(true);
    VisionAlgorithm::DirectionResult res;
    while (m_running.load()) {

        int frameId = 0;

        if (!m_queue->try_dequeue(frameId)) {
          //  QThread::msleep(1);
            continue;
        }

        unsigned char* imageByte = frameId*bufferSize + m_imageByteAddr;
        HalconCpp::GenImage1(&DetecImage,"byte",m_width,m_height, HalconCpp::HTuple(reinterpret_cast<Hlong>(imageByte)));
        res = VisionAlgorithm::DirectionRecognize(DetecImage,TopRegion,DownRegion,m_offsetX,m_offsetY,m_offsetXDown,m_offsetYDown,m_dropThres);
        // ⚠️ 注意：这里读取的是同一个 m_imageBuffer
       // processFrame(m_imageByteAddr);

        emit frameUpdated(frameId);
    }

    emit finished();
};
void ProcessWorker::StopWork(){
     m_running.store(false, std::memory_order_release);

};
void ProcessWorker::OnFrameArrived(int frameId)
{



};
