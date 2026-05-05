#pragma once

#include <QString>
#include <QImage>
#include <cstdint>
#include <cstddef>

#include <CameraParamsWrapper.h>
#include <HikSdkWrapper.h>
#include <MvErrorDefine.h>
#include <MvISPErrorDefine.h>

class HikCamera
{
public:
    struct FrameInfo
    {
        int Width = 0;
        int Height = 0;
        int FrameLen = 0;
        int PixelType = 0;
        std::uint64_t FrameNum = 0;
    };

public:
    HikCamera(const QString& serialNum, int width, int height);
    ~HikCamera();

    HikCamera(const HikCamera&) = delete;
    HikCamera& operator=(const HikCamera&) = delete;

    int Init();
    int Close();

    bool IsOpened() const;
    bool IsGrabbing() const;

    int GrabOneFrame(QImage& outImage, unsigned int timeoutMs = 1000);
    int GrabOneFrame(unsigned char* imageBuffer, std::size_t bufferSize, unsigned int timeoutMs = 1000);

    QString LastErrorString() const;

private:
    int EnumAndOpenMatchedDevice();
    int ConfigureDevice();
    int StartGrabbing();
    QString ExtractSerialNumber(const MV_CC_DEVICE_INFO* deviceInfo) const;
    void SetLastError(int errorCode, const QString& context);
    QString BuildErrorString(const QString& context, unsigned int errorCode) const;

private:
    QString m_serialNum;
    int m_width = 0;
    int m_height = 0;

    void* m_cameraHandle = nullptr;
    bool m_isOpen = false;
    bool m_isGrabbing = false;

    int m_lastError = MV_OK;
    QString m_lastErrorString;
};
