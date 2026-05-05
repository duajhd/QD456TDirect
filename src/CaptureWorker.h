#ifndef CAPTUREWORKER_H
#define CAPTUREWORKER_H
#include <QObject>

#include <HikCamera.h>
#include "readerwriterqueue.h"
class CaptureWorker:public QObject
{

     Q_OBJECT
public:
     explicit CaptureWorker(int width,int height,int channel,HikCamera * camera,unsigned char* imageByteAddr,
                           moodycamel::ReaderWriterQueue<int>* queue,QObject* parent = nullptr);
    ~CaptureWorker() override = default;

private:
    int m_width;
    int m_height;
    int m_channel;
    QString m_serialNum;
    HikCamera* m_camera;
    unsigned char*  m_imageByteAddr = nullptr;
    std::size_t bufferSize;
    std::atomic_bool m_running { false };
    moodycamel::ReaderWriterQueue<int>* m_queue = nullptr;


    int m_frameId = 0;
signals:
   void frameArrived(int frameId);
    void finished();
public slots:
    void startWork();
    void stopWork();

};
#endif // CAPTUREWORKER_H
