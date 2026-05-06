
#ifndef CAMERAVIEWMODEL_H
#define CAMERAVIEWMODEL_H


#include <HikCamera.h>
#include <QThread>
#include <QObject>
#include <CaptureWorker.h>
#include <ProcessWorker.h>
#include <memory>
class CameraViewModel: public QObject{
    Q_OBJECT
public:
    static constexpr int FrameSlotCount = 1200;

    explicit CameraViewModel(int width,
                             int height,
                             QString serialNum,
                             int channel,
                             int cameraIndex,
                             moodycamel::ReaderWriterQueue<int>* dropQueue,
                             QObject* parent = nullptr);
      ~CameraViewModel() override;

private:
    int m_width;
    int m_height;
    int m_channel;
    int m_cameraIndex;
    std::unique_ptr<HikCamera> m_camera;
    QString m_serialNum;
    QThread* m_captureThread = nullptr;
    QThread* m_processThread = nullptr;
    CaptureWorker* m_captureWorker = nullptr;
    ProcessWorker* m_processWorker = nullptr;
    bool m_initialize = false;
    bool m_started = false;

      std::unique_ptr<moodycamel::ReaderWriterQueue<int>> m_frameQueue;
      std::unique_ptr<unsigned char[]> m_imageStorage;
      moodycamel::ReaderWriterQueue<int>* m_dropQueue = nullptr;


public:
    unsigned char* m_imageBuffer = nullptr;

 signals:
     void resultReady(int channelId);
     void frameUpdated(int frameId);


public:
    void Init();
    void Start();
    void Stop();
    void SetCameraParameter();

    int imageWidth() const { return m_width; }
    int imageHeight() const { return m_height; }
    int channel() const { return m_channel; }

};
#endif
