
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
    Q_PROPERTY(QString serialNumber READ serialNumber CONSTANT)
    Q_PROPERTY(double exposureTime READ exposureTime NOTIFY cameraParametersChanged)
    Q_PROPERTY(double gain READ gain NOTIFY cameraParametersChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int frameId READ frameId NOTIFY frameIdChanged)
    Q_PROPERTY(int algorithmFrameCount READ algorithmFrameCount NOTIFY algorithmFrameCountChanged)
    Q_PROPERTY(int topConnectedCount READ topConnectedCount NOTIFY regionCountsChanged)
    Q_PROPERTY(int downConnectedCount READ downConnectedCount NOTIFY regionCountsChanged)
    Q_PROPERTY(int rejectFrameCount READ rejectFrameCount NOTIFY rejectFrameCountChanged)
    Q_PROPERTY(int width READ width CONSTANT)
    Q_PROPERTY(int height READ height CONSTANT)
public:
    static constexpr int FrameSlotCount = 1200;

    explicit CameraViewModel(int width,
                             int height,
                             QString serialNum,
                             int channel,
                             int cameraIndex,
                             double exposureTime,
                             double gain,
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
    int m_algorithmFrameCount = 0;
    int m_topConnectedCount = -1;
    int m_downConnectedCount = -1;
    int m_rejectFrameCount = 0;
    float m_exposureTime = 500.0f;
    float m_gain = 15.0f;
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
     void algorithmFrameCountChanged();
     void regionCountsChanged();
     void rejectFrameCountChanged();
     void cameraParametersChanged();
     void frameSourceReset();


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
    QString serialNumber() const;
    double exposureTime() const;
    double gain() const;
    QString statusText() const;
    int frameId() const;
    int algorithmFrameCount() const;
    int topConnectedCount() const;
    int downConnectedCount() const;
    int rejectFrameCount() const;
    int width() const;
    int height() const;
    void incrementAlgorithmFrameCount();
    void setAlgorithmFrameCount(int value);
    void setAlgorithmRegionCounts(int topConnectedCount, int downConnectedCount, int downSelectedCount);
    void incrementRejectFrameCount();

};
#endif
