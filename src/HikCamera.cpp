#include "HikCamera.h"

#include <QDebug>
#include <cstring>
#include <vector>

HikCamera::HikCamera(const QString& serialNum, int width, int height)
    : m_serialNum(serialNum)
    , m_width(width)
    , m_height(height)
{
}

HikCamera::~HikCamera()
{
    Close();
}

int HikCamera::Init()
{
    if (m_isOpen)
    {
        return MV_OK;
    }

    int nRet = EnumAndOpenMatchedDevice();
    if (nRet != MV_OK)
    {
        Close();
        return nRet;
    }

    nRet = ConfigureDevice();
    if (nRet != MV_OK)
    {
        Close();
        return nRet;
    }

    nRet = StartGrabbing();
    if (nRet != MV_OK)
    {
        Close();
        return nRet;
    }

    return MV_OK;
}

int HikCamera::Close()
{
    int finalRet = MV_OK;

    if (m_cameraHandle != nullptr)
    {
        if (m_isGrabbing)
        {
            int nRet = MV_CC_StopGrabbing(m_cameraHandle);
            if (nRet != MV_OK && finalRet == MV_OK)
            {
                finalRet = nRet;
                SetLastError(nRet, "MV_CC_StopGrabbing failed");
            }
            m_isGrabbing = false;
        }

        if (m_isOpen)
        {
            int nRet = MV_CC_CloseDevice(m_cameraHandle);
            if (nRet != MV_OK && finalRet == MV_OK)
            {
                finalRet = nRet;
                SetLastError(nRet, "MV_CC_CloseDevice failed");
            }
            m_isOpen = false;
        }

        int nRet = MV_CC_DestroyHandle(m_cameraHandle);
        if (nRet != MV_OK && finalRet == MV_OK)
        {
            finalRet = nRet;
            SetLastError(nRet, "MV_CC_DestroyHandle failed");
        }

        m_cameraHandle = nullptr;
    }

    return finalRet;
}

bool HikCamera::IsOpened() const
{
    return m_isOpen;
}

bool HikCamera::IsGrabbing() const
{
    return m_isGrabbing;
}

int HikCamera::SetExposureTime(float exposureTime)
{
    if (m_cameraHandle == nullptr || !m_isOpen)
    {
        SetLastError(MV_E_CALLORDER, "SetExposureTime failed: camera is not open");
        return MV_E_CALLORDER;
    }

    const int autoRet = MV_CC_SetEnumValue(m_cameraHandle, "ExposureAuto", 0);
    if (autoRet != MV_OK)
    {
        qWarning() << BuildErrorString("Set ExposureAuto Off failed", static_cast<unsigned int>(autoRet));
    }

    const int nRet = MV_CC_SetExposureTime(m_cameraHandle, exposureTime);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set ExposureTime failed");
        return nRet;
    }

    m_lastError = MV_OK;
    m_lastErrorString.clear();
    return MV_OK;
}

int HikCamera::SetGain(float gain)
{
    if (m_cameraHandle == nullptr || !m_isOpen)
    {
        SetLastError(MV_E_CALLORDER, "SetGain failed: camera is not open");
        return MV_E_CALLORDER;
    }

    const int autoRet = MV_CC_SetEnumValue(m_cameraHandle, "GainAuto", 0);
    if (autoRet != MV_OK)
    {
        qWarning() << BuildErrorString("Set GainAuto Off failed", static_cast<unsigned int>(autoRet));
    }

    const int nRet = MV_CC_SetGain(m_cameraHandle, gain);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set Gain failed");
        return nRet;
    }

    m_lastError = MV_OK;
    m_lastErrorString.clear();
    return MV_OK;
}

int HikCamera::SetCameraParameters(float exposureTime, float gain)
{
    int nRet = SetExposureTime(exposureTime);
    if (nRet != MV_OK)
    {
        return nRet;
    }

    nRet = SetGain(gain);
    if (nRet != MV_OK)
    {
        return nRet;
    }

    return MV_OK;
}

int HikCamera::GrabOneFrame(unsigned char* imageBuffer, std::size_t bufferSize,  unsigned int timeoutMs)
{
    if (imageBuffer == nullptr)
    {
        SetLastError(MV_E_PARAMETER, "GrabOneFrame failed: imageBuffer is nullptr");
        return MV_E_PARAMETER;
    }

    if (m_cameraHandle == nullptr || !m_isOpen || !m_isGrabbing)
    {
        SetLastError(MV_E_CALLORDER, "GrabOneFrame failed: camera is not ready");
        return MV_E_CALLORDER;
    }

    MV_FRAME_OUT frameOut = {};
    int nRet = MV_CC_GetImageBuffer(m_cameraHandle, &frameOut, timeoutMs);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "MV_CC_GetImageBuffer failed");
        return nRet;
    }

    int result = MV_OK;

    do
    {
        if (frameOut.pBufAddr == nullptr)
        {
            result = MV_E_NODATA;
            SetLastError(result, "GrabOneFrame failed: frameOut.pBufAddr is nullptr");
            break;
        }

        const std::size_t frameLen = static_cast<std::size_t>(frameOut.stFrameInfo.nFrameLen);
        if (bufferSize < frameLen)
        {
            result = MV_E_NOENOUGH_BUF;
            SetLastError(result, "GrabOneFrame failed: buffer too small");
            break;
        }

        std::memcpy(imageBuffer, frameOut.pBufAddr, frameLen);


        m_lastError = MV_OK;
        m_lastErrorString.clear();

    } while (false);

    const int freeRet = MV_CC_FreeImageBuffer(m_cameraHandle, &frameOut);
    if (freeRet != MV_OK && result == MV_OK)
    {
        result = freeRet;
        SetLastError(freeRet, "MV_CC_FreeImageBuffer failed");
    }

    return result;
}

int HikCamera::GrabOneFrame(QImage& outImage, unsigned int timeoutMs)
{
    if (!m_isOpen || !m_isGrabbing)
    {
        SetLastError(MV_E_CALLORDER, "GrabOneFrame(QImage) failed: camera is not ready");
        return MV_E_CALLORDER;
    }

    const std::size_t bufferSize = static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height);
    std::vector<unsigned char> buffer(bufferSize);


    const int nRet = GrabOneFrame(buffer.data(), buffer.size(),  timeoutMs);
    if (nRet != MV_OK)
    {
        return nRet;
    }


    return MV_OK;
}

QString HikCamera::LastErrorString() const
{
    return m_lastErrorString;
}

int HikCamera::EnumAndOpenMatchedDevice()
{
    MV_CC_DEVICE_INFO_LIST deviceList = {};
    const unsigned int layerMask =
        MV_GIGE_DEVICE |
        MV_USB_DEVICE |
        MV_GENTL_GIGE_DEVICE |
        MV_GENTL_CAMERALINK_DEVICE |
        MV_GENTL_CXP_DEVICE |
        MV_GENTL_XOF_DEVICE;

    int nRet = MV_CC_EnumDevices(layerMask, &deviceList);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "MV_CC_EnumDevices failed");
        return nRet;
    }

    if (deviceList.nDeviceNum == 0)
    {
        SetLastError(MV_E_NODATA, "No camera devices found");
        return MV_E_NODATA;
    }

    MV_CC_DEVICE_INFO* matchedDevice = nullptr;

    for (unsigned int i = 0; i < deviceList.nDeviceNum; ++i)
    {
        MV_CC_DEVICE_INFO* deviceInfo = deviceList.pDeviceInfo[i];
        if (deviceInfo == nullptr)
        {
            continue;
        }

        const QString sn = ExtractSerialNumber(deviceInfo);
        if (sn == m_serialNum)
        {
            matchedDevice = deviceInfo;
            break;
        }
    }

    if (matchedDevice == nullptr)
    {
        SetLastError(MV_E_NODATA, QString("Camera with serial number [%1] not found").arg(m_serialNum));
        return MV_E_NODATA;
    }

    nRet = MV_CC_CreateHandle(&m_cameraHandle, matchedDevice);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "MV_CC_CreateHandle failed");
        return nRet;
    }

    nRet = MV_CC_OpenDevice(m_cameraHandle);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "MV_CC_OpenDevice failed");
        return nRet;
    }

    m_isOpen = true;
    return MV_OK;
}

int HikCamera::ConfigureDevice()
{
    if (m_cameraHandle == nullptr)
    {
        SetLastError(MV_E_HANDLE, "ConfigureDevice failed: invalid handle");
        return MV_E_HANDLE;
    }

    int nRet = MV_CC_SetEnumValue(m_cameraHandle, "AcquisitionMode", 2);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set AcquisitionMode failed");
        return nRet;
    }

    nRet = MV_CC_SetEnumValue(m_cameraHandle, "TriggerMode", 0);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set TriggerMode failed");
        return nRet;
    }

    nRet = MV_CC_SetIntValue(m_cameraHandle, "OffsetX", 0);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set OffsetX failed");
        return nRet;
    }

    nRet = MV_CC_SetIntValue(m_cameraHandle, "OffsetY", 0);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set OffsetY failed");
        return nRet;
    }

    nRet = MV_CC_SetIntValue(m_cameraHandle, "Width", static_cast<unsigned int>(m_width));
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set Width failed");
        return nRet;
    }

    nRet = MV_CC_SetIntValue(m_cameraHandle, "Height", static_cast<unsigned int>(m_height));
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "Set Height failed");
        return nRet;
    }

    return MV_OK;
}

int HikCamera::StartGrabbing()
{
    if (m_cameraHandle == nullptr)
    {
        SetLastError(MV_E_HANDLE, "StartGrabbing failed: invalid handle");
        return MV_E_HANDLE;
    }

    const int nRet = MV_CC_StartGrabbing(m_cameraHandle);
    if (nRet != MV_OK)
    {
        SetLastError(nRet, "MV_CC_StartGrabbing failed");
        return nRet;
    }

    m_isGrabbing = true;
    return MV_OK;
}

QString HikCamera::ExtractSerialNumber(const MV_CC_DEVICE_INFO* deviceInfo) const
{
    if (deviceInfo == nullptr)
    {
        return QString();
    }

    switch (deviceInfo->nTLayerType)
    {
    case MV_USB_DEVICE:
        return QString::fromLocal8Bit(reinterpret_cast<const char*>(deviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber)).trimmed();
    case MV_GIGE_DEVICE:
        return QString::fromLocal8Bit(reinterpret_cast<const char*>(deviceInfo->SpecialInfo.stGigEInfo.chSerialNumber)).trimmed();
    default:
        return QString();
    }
}

void HikCamera::SetLastError(int errorCode, const QString& context)
{
    m_lastError = errorCode;
    m_lastErrorString = BuildErrorString(context, static_cast<unsigned int>(errorCode));
    qWarning() << m_lastErrorString;
}

QString HikCamera::BuildErrorString(const QString& context, unsigned int errorCode) const
{
    return QString("%1 | error=0x%2").arg(context).arg(errorCode, 0, 16);
}
