#ifndef MAINVIEWWINDOW_H
#define MAINVIEWWINDOW_H
#include <CameraFrameItem.h>
#include <CameraViewModel.h>
#include <QOBject.h>
class MianViewModel:public QObject {


private:
  bool  m_isRunning;
   QVector<CameraViewModel*> m_cameras;
   // std::array<std::unique_ptr<CameraFrameItem>, 4> m_cameraItem;

public:
  void StartDetect();
  void Initialize();
  void StopDetect();

 Q_INVOKABLE  QObject* getCamera(int coount) const;



};

#endif // MAINVIEWWINDOW_H
//cameraFrameItem不需要显示构造2.cameraFrameItem持有CameraViewModel
