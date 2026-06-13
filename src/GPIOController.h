#ifndef GPIOCONTROLLER_H
#define GPIOCONTROLLER_H
#include <QObject>
#include "stdio.h"

#include "adc.h"
#include "device.h"
#include "gpio.h"
#include "dio.h"
#include "misc.h"
#include "pwm.h"
#include "usb_device.h"
#include "readerwriterqueue.h"
#include <array>
#include <atomic>
#include <QHash>
#include <QVector>

struct GPIOCameraRoute
{
    int cameraIndex = -1;
    int pin = -1;
};

struct GPIOControllerConfig
{
    int serialNumber = 0;
    int deviceIndex = -1;
    QVector<GPIOCameraRoute> routes;
};

class GPIOController:public QObject{
    Q_OBJECT

public:
    explicit GPIOController(moodycamel::ReaderWriterQueue<int>* queue,
                            const GPIOControllerConfig& config,
                            QObject* parent = nullptr);
    ~GPIOController() override= default;

private :
    static constexpr int MaxDeviceCount = 256;
    std::array<int, MaxDeviceCount> SerialNumbers {};
    moodycamel::ReaderWriterQueue<int>* m_queue = nullptr;
    GPIOControllerConfig m_config;
    QHash<int, int> m_cameraPins;
    std::atomic_bool m_running { false };
    bool m_initialized = false;
    int m_serialNumber = 0;

public slots:
    void startWork();
    void startWorks();
    void StopWork();

signals:
    void finished();
    void dropped(int index);

public:
    int Initialize();
    void Drop(int index);


};

#endif // GPIOCONTROLLER_H
