#include <GPIOController.h>

#include <chrono>
#include <QDebug>
#include <QThread>
#include <thread>

GPIOController::GPIOController(moodycamel::ReaderWriterQueue<int>* queue,
                               const GPIOControllerConfig& config,
                               QObject* parent)
    : QObject(parent),
      m_queue(queue),
      m_config(config)
{
    for (const GPIOCameraRoute& route : m_config.routes) {
        if (route.cameraIndex >= 0 && route.pin >= 0) {
            m_cameraPins.insert(route.cameraIndex, route.pin);
        }
    }
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

    if (m_config.serialNumber > 0) {
        for (int i = 0; i < deviceCount; ++i) {
            if (SerialNumbers[i] == m_config.serialNumber) {
                m_serialNumber = SerialNumbers[i];
                break;
            }
        }

        if (m_serialNumber == 0) {
            qWarning() << "Configured GPIO serial number not found:" << m_config.serialNumber;
            return -1;
        }
    } else {
        const int deviceIndex = m_config.deviceIndex >= 0 ? m_config.deviceIndex : 0;
        if (deviceIndex >= deviceCount) {
            qWarning() << "Configured GPIO device index out of range:" << deviceIndex
                       << "deviceCount:" << deviceCount;
            return -1;
        }
        m_serialNumber = SerialNumbers[deviceIndex];
    }

    IO_InitStruct_Tx_t initStruct_Tx[16] = {};
    IO_InitStruct_Rx_t initStruct_Rx[16] = {};
    for (int i = 0; i < 16; i++) {
        initStruct_Tx[i].Pin = i;
        initStruct_Tx[i].Mode = 1;
        initStruct_Tx[i].Pull = 0;
    }

    ret = IO_InitMultiPin(m_serialNumber, initStruct_Tx, initStruct_Rx, 16);
    if (ret < 0) {
        qWarning() << "GPIO init error:" << ret;
        return ret;
    }

    for (auto it = m_cameraPins.constBegin(); it != m_cameraPins.constEnd(); ++it) {
        IO_WritePin(m_serialNumber, it.value(), 1);
    }

    qDebug() << "GPIO controller initialized"
             << "SN:" << m_serialNumber
             << "routes:" << m_cameraPins;
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
        if (!m_initialized) {
            emit finished();
            return;
        }
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
    const auto pinIt = m_cameraPins.constFind(index);
    if (pinIt == m_cameraPins.constEnd()) {
        qWarning() << "No GPIO pin route for camera index" << index;
        return;
    }

    IO_WritePin(m_serialNumber, pinIt.value(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    IO_WritePin(m_serialNumber, pinIt.value(), 1);
    emit dropped(index);
}
