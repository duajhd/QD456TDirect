#ifndef MAINVIEWWINDOW_H
#define MAINVIEWWINDOW_H
#include <CameraFrameItem.h>
#include <CameraViewModel.h>
#include <RoiManager.h>
#include <QObject>
class MianViewModel:public QObject {
  Q_OBJECT


private:
  bool  m_isRunning;
   QVector<CameraViewModel*> m_cameras;
   QVector<RoiManager*> m_roiManagers;
   // std::array<std::unique_ptr<CameraFrameItem>, 4> m_cameraItem;

public:
  void StartDetect();
  void Initialize();
  void StopDetect();

 Q_INVOKABLE  QObject* getCamera(int coount) const;
 Q_INVOKABLE  QObject* getRoiManager(int count) const;



};

#endif // MAINVIEWWINDOW_H
//cameraFrameItem不需要显示构造2.cameraFrameItem持有CameraViewModel
