
#ifndef CAMERAVIEWMODEL_H
#define CAMERAVIEWMODEL_H


#include <DetectionRoiConfig.h>
#include <HikCamera.h>
#include <QThread>
#include <QObject>
#include <CaptureWorker.h>
#include <ProcessWorker.h>
#include <memory>
class CameraViewModel: public QObject{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int frameId READ frameId NOTIFY frameIdChanged)
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
public:
    static constexpr int FrameSlotCount = 1200;

    explicit CameraViewModel(int width,
                             int height,
                             QString serialNum,
                             int channel,
                             int cameraIndex,
                             const DetectionAlgorithmParams& algorithmParams,
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
    int m_frameId = -1;
    float m_exposureTime = 5000.0f;
    float m_gain = 1.0f;
    DetectionRoiConfig m_detectionConfig;
    QString m_statusText = QStringLiteral("未连接");

      std::unique_ptr<moodycamel::ReaderWriterQueue<int>> m_frameQueue;
      std::unique_ptr<unsigned char[]> m_imageStorage;
      moodycamel::ReaderWriterQueue<int>* m_dropQueue = nullptr;

    void setStatusText(const QString& statusText);
    void setFrameId(int frameId);

public:
    unsigned char* m_imageBuffer = nullptr;

 signals:
     void resultReady(int channelId);
     void frameUpdated(int frameId);
     void statusTextChanged();
     void frameIdChanged();


public:
    void Init();
    void Start();
    void Stop();
    void CleanupThreads();
    bool SetCameraParameter(double exposureTime, double gain, int dropThres);
    void SetDetectionConfig(const DetectionRoiConfig& config);

    int imageWidth() const { return m_width; }
    int imageHeight() const { return m_height; }
    int channel() const { return m_channel; }
    QString name() const;
    QString statusText() const;
    int frameId() const;
    int width() const;
    int height() const;

};
#endif
