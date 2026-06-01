#ifndef MAINVIEWWINDOW_H
#define MAINVIEWWINDOW_H
#include <CameraFrameItem.h>
#include <CameraViewModel.h>
#include <GPIOController.h>
#include <RoiManager.h>
#include <QObject>
#include <QThread>
#include <memory>
class MianViewModel:public QObject {
  Q_OBJECT
  Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)


private:
  bool  m_isRunning = false;
   QVector<CameraViewModel*> m_cameras;
   QVector<RoiManager*> m_roiManagers;
   std::unique_ptr<moodycamel::ReaderWriterQueue<int>> m_dropQueue;
   DetectionAlgorithmParams m_algorithmParams;
   GPIOController* m_gpioController = nullptr;
   QThread* m_gpioThread = nullptr;
   // std::array<std::unique_ptr<CameraFrameItem>, 4> m_cameraItem;

public:
  explicit MianViewModel(QObject* parent = nullptr);
  ~MianViewModel() override;
  Q_INVOKABLE void StartDetect();
  void Initialize();
  Q_INVOKABLE void StopDetect();
  Q_INVOKABLE void ToggleDetect();
  Q_INVOKABLE bool ApplyRoiConfig(int cameraIndex);
  Q_INVOKABLE bool SetCameraParameter(int cameraIndex, double exposureTime, double gain, int dropThres);
  bool isRunning() const;

 Q_INVOKABLE  QObject* getCamera(int coount) const;
 Q_INVOKABLE  QObject* getRoiManager(int count) const;

signals:
 void isRunningChanged();



};

#endif // MAINVIEWWINDOW_H
//cameraFrameItem不需要显示构造2.cameraFrameItem持有CameraViewModel
