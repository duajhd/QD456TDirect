#ifndef HIKSDKRUNTIME_H
#define HIKSDKRUNTIME_H

#endif // HIKSDKRUNTIME_H
#pragma once

#include <HikSdkWrapper.h>
#include <QDebug>

class HikSdkRuntime
{
public:
    HikSdkRuntime()
    {
        const int nRet = MV_CC_Initialize();
        if (nRet != MV_OK)
        {
            qCritical() << "MV_CC_Initialize failed:" << QString("0x%1").arg(nRet, 0, 16);
        }
        else
        {
            qDebug() << "MV_CC_Initialize success";
        }
    }

    ~HikSdkRuntime()
    {
        const int nRet = MV_CC_Finalize();
        if (nRet != MV_OK)
        {
            qCritical() << "MV_CC_Finalize failed:" << QString("0x%1").arg(nRet, 0, 16);
        }
        else
        {
            qDebug() << "MV_CC_Finalize success";
        }
    }

    HikSdkRuntime(const HikSdkRuntime&) = delete;
    HikSdkRuntime& operator=(const HikSdkRuntime&) = delete;
};
