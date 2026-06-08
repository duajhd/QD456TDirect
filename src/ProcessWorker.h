#ifndef PROCESSWORKER_H
#define PROCESSWORKER_H
#include "DetectionRoiConfig.h"
#include "readerwriterqueue.h"
#include <HalconCpp.h>
#include <QMutex>
#include <QObject>
#include <atomic>
#include <cstddef>

class ProcessWorker:public QObject
{
    Q_OBJECT
public:
    explicit ProcessWorker(int width,int height,int channel,  unsigned char* imageByteAddr,
                           moodycamel::ReaderWriterQueue<int>* queue,
                           int cameraIndex,
                           moodycamel::ReaderWriterQueue<int>* dropQueue,
                           QObject* parent = nullptr);
    ~ProcessWorker() override = default;
private:
    int m_width;
    int m_height;
    int m_channel;
    int m_cameraIndex = -1;

    unsigned char* m_imageByteAddr;
    std::size_t bufferSize;

    moodycamel::ReaderWriterQueue<int>* m_queue = nullptr;
    moodycamel::ReaderWriterQueue<int>* m_dropQueue = nullptr;
    std::atomic_bool m_running { false };
    int m_processFrameId = 0;

    HalconCpp::HObject DetecImage;
    QMutex m_configMutex;
    DetectionRoiConfig m_detectionConfig;
    HalconCpp::HObject m_topRectangle;
    HalconCpp::HObject m_downRectangle;
    bool m_hasDetectionRois = false;



signals:
    void finished();
     void frameUpdated(int frameId);
     void algorithmRegionCountsUpdated(int topConnectedCount, int downConnectedCount, int downSelectedCount);
public slots:
  void OnFrameArrived(int frameId);
  void StartWork();
  void StopWork();
  void SetDetectionConfig(const DetectionRoiConfig& config);

};
#endif // PROCESSWORKER_H
