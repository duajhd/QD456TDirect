#include <CameraViewModel.h>

CameraViewModel::CameraViewModel(int width,int height ,QString serialNum,int channel, QObject* parent):QObject(parent),m_width(width),m_height(height),m_serialNum(serialNum),
    m_channel(channel)
{

};
CameraViewModel::~CameraViewModel(){};
void CameraViewModel::Init()
{
    m_camera = std::make_unique<HikCamera>(m_serialNum,m_width,m_height);
    int res =  m_camera->Init();
    if(res != MV_OK) return;
    m_initialize = true;
    m_captureWorker = new CaptureWorker(m_width,m_height,m_channel,m_camera.get(),m_imageBuffer, m_frameQueue.get());
    m_processWorker = new ProcessWorker(m_width,m_height,m_channel,m_imageBuffer, m_frameQueue.get());
    m_captureThread = new QThread(this);
    m_processThread = new QThread(this);

    m_captureWorker->moveToThread(m_captureThread);
    m_processWorker->moveToThread(m_processThread);

    //采集线程与处理线程信号

    connect(m_captureThread,&QThread::started,m_captureWorker,&CaptureWorker::startWork);
    connect(m_processThread,&QThread::started,m_processWorker,&ProcessWorker::StartWork);

    connect(m_captureWorker, &CaptureWorker::finished,
            m_captureThread, &QThread::quit);
    connect(m_processWorker, &ProcessWorker::finished,
            m_processThread, &QThread::quit);

    connect(m_captureThread, &QThread::finished,
            m_captureWorker, &QObject::deleteLater);
    connect(m_processThread, &QThread::finished,
            m_processWorker, &QObject::deleteLater);
    //信号
};
//启动线程开始采图
void CameraViewModel::Start(){

    if(!m_initialize) return ;
    m_processThread->start();
    m_captureThread->start();
    m_started = true;


}
