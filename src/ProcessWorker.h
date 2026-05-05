#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H
#include "readerwriterqueue.h"
#include <HalconCpp.h>
#include <QObject>

class ProcessWorker:public QObject
{
    Q_OBJECT
public:
    explicit ProcessWorker(int width,int height,int channel,  unsigned char* imageByteAddr,
                           moodycamel::ReaderWriterQueue<int>* queue,QObject* parent = nullptr);
    ~ProcessWorker() override = default;
private:
    int m_width;
    int m_height;
    int m_channel;

    unsigned char* m_imageByteAddr;
    std::size_t bufferSize;

    moodycamel::ReaderWriterQueue<int>* m_queue = nullptr;
    std::atomic_bool m_running { false };
    int m_processFrameId = 0;

    HalconCpp::HObject DetecImage;
    HalconCpp::HObject TopRegion;
    HalconCpp::HObject DownRegion;
    float m_dropThres = 27.0;
    int m_offsetX = 0;
    int m_offsetY = 0;
    int m_offsetXDown = 0;
    int m_offsetYDown = 0;



 signals:
    void finished();
     void frameUpdated(int frameId);
public slots:
  void OnFrameArrived(int frameId);
  void StartWork();
  void StopWork();

};
#endif // PROCESSWORKER_H
