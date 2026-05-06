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
#include <atomic>
class GPIOController:public QObject{
    Q_OBJECT

public:
    explicit GPIOController(moodycamel::ReaderWriterQueue<int>* queue, QObject* parent = nullptr);
    ~GPIOController() override= default;

private :
    int SerialNumbers[16];
    moodycamel::ReaderWriterQueue<int>* m_queue = nullptr;
    std::atomic_bool m_running { false };
    bool m_initialized = false;

public slots:
    void startWork();
    void startWorks();
    void StopWork();

signals:
    void finished();

public:
    int Initialize();
    void Drop(int index);


};

#endif // GPIOCONTROLLER_H
