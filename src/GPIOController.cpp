#include <GPIOController.h>

#include <QDebug>
#include <QThread>

GPIOController::GPIOController(moodycamel::ReaderWriterQueue<int>* queue, QObject* parent)
    : QObject(parent),
      m_queue(queue)
{
}

int GPIOController::Initialize()
{
    int ret = UsbDevice_Scan(SerialNumbers.data());
    if (ret < 0) {
        qWarning() << "GPIO scan error:" << ret;
        return ret;
    }

    if (ret == 0) {
        qWarning() << "No GPIO device";
        return ret;
    }

    const int deviceCount = qMin(ret, MaxDeviceCount);
    if (ret > MaxDeviceCount) {
        qWarning() << "GPIO scan returned more devices than local buffer:" << ret;
    }

    for (int i = 0; i < deviceCount; i++) {
        qDebug() << "GPIO device" << i << "SN:" << SerialNumbers[i];
    }

    IO_InitStruct_Tx_t initStruct_Tx[16] = {};
    IO_InitStruct_Rx_t initStruct_Rx[16] = {};
    for (int i = 0; i < 16; i++) {
        initStruct_Tx[i].Pin = i;
        initStruct_Tx[i].Mode = 1;
        initStruct_Tx[i].Pull = 0;
    }

    ret = IO_InitMultiPin(SerialNumbers[0], initStruct_Tx, initStruct_Rx, 16);
    if (ret < 0) {
        qWarning() << "GPIO init error:" << ret;
        return ret;
    }

    return ret;
}

void GPIOController::startWork()
{
    if (!m_queue) {
        emit finished();
        return;
    }

    if (!m_initialized) {
        m_initialized = Initialize() > 0;
    }

    m_running.store(true, std::memory_order_release);

    while (m_running.load(std::memory_order_acquire)) {
        int cameraIndex = -1;
        if (!m_queue->try_dequeue(cameraIndex)) {
            QThread::msleep(1);
            continue;
        }

        Drop(cameraIndex);
    }

    emit finished();
}

void GPIOController::startWorks()
{
    startWork();
}

void GPIOController::StopWork()
{
    m_running.store(false, std::memory_order_release);
}

void GPIOController::Drop(int index)
{
    qDebug() << "Drop camera index:" << index;
}
