#include "CameraFrameItem.h"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QtQuick/private/qsgplaintexture_p.h>
#include <QMutexLocker>
#include <QDebug>

CameraFrameItem::CameraFrameItem(QQuickItem* parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

CameraFrameItem::~CameraFrameItem()
{
    if (m_cameraVm) {
        disconnect(m_cameraVm, nullptr, this, nullptr);
    }
}

QObject* CameraFrameItem::cameraVm() const
{
    return m_cameraVm;
}

void CameraFrameItem::setCameraVm(QObject* vmObj)
{
    auto* vm = qobject_cast<CameraViewModel*>(vmObj);

    if (m_cameraVm == vm) {
        return;
    }

    if (m_cameraVm) {
        disconnect(m_cameraVm, nullptr, this, nullptr);
    }

    m_cameraVm = vm;

    if (m_cameraVm) {
        connect(m_cameraVm,
                &CameraViewModel::frameUpdated,
                this,
                &CameraFrameItem::onFrameUpdated,
                Qt::QueuedConnection);
    } else {
        QMutexLocker locker(&m_mutex);
        m_pendingFrameAddr = nullptr;
        m_pendingWidth = 0;
        m_pendingHeight = 0;
        m_pendingBytesPerLine = 0;
        m_pendingFormat = QImage::Format_Invalid;
    }

    emit cameraVmChanged();
    update();
}

void CameraFrameItem::onFrameUpdated(int frameId)
{
    if (!m_cameraVm || !m_cameraVm->m_imageBuffer) {
        return;
    }

    const int width = m_cameraVm->imageWidth();
    const int height = m_cameraVm->imageHeight();
    const int channel = m_cameraVm->channel();

    if (width <= 0 || height <= 0 || channel <= 0) {
        return;
    }

    const int slotIndex = frameId % CameraViewModel::FrameSlotCount;

    const qsizetype frameBytes =
        static_cast<qsizetype>(width) *
        static_cast<qsizetype>(height) *
        static_cast<qsizetype>(channel);

    unsigned char* imageByteAddr =
        m_cameraVm->m_imageBuffer + static_cast<qsizetype>(slotIndex) * frameBytes;

    int bytesPerLine = 0;
    QImage::Format format = QImage::Format_Invalid;

    if (channel == 1) {
        bytesPerLine = width;
        format = QImage::Format_Grayscale8;
    } else if (channel == 3) {
        bytesPerLine = width * 3;
        format = QImage::Format_RGB888;
    } else {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_pendingFrameAddr = imageByteAddr;
        m_pendingWidth = width;
        m_pendingHeight = height;
        m_pendingBytesPerLine = bytesPerLine;
        m_pendingFormat = format;
    }

    update();
}

QSGNode* CameraFrameItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    unsigned char* frameAddr = nullptr;
    int width = 0;
    int height = 0;
    int bytesPerLine = 0;
    QImage::Format format = QImage::Format_Invalid;
    {
        QMutexLocker locker(&m_mutex);
        frameAddr = m_pendingFrameAddr;
        width = m_pendingWidth;
        height = m_pendingHeight;
        bytesPerLine = m_pendingBytesPerLine;
        format = m_pendingFormat;
    }

    if (!frameAddr || width <= 0 || height <= 0 || bytesPerLine <= 0 || format == QImage::Format_Invalid || !window()) {
        delete oldNode;
        return nullptr;
    }

    QImage image(frameAddr, width, height, bytesPerLine, format);

    auto* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode();
    }

    auto* texture = static_cast<QSGPlainTexture*>(node->texture());
    if (!texture) {
        texture = new QSGPlainTexture();
        node->setTexture(texture);
        node->setOwnsTexture(true);
    }
    texture->setImage(image);

    const QRectF bounds = boundingRect();
    const QSize imageSize = image.size();
    QRectF targetRect = bounds;

    if (imageSize.width() > 0 &&
        imageSize.height() > 0 &&
        bounds.width() > 0.0 &&
        bounds.height() > 0.0) {
        const qreal scale = qMin(bounds.width() / imageSize.width(),
                                 bounds.height() / imageSize.height());
        const qreal targetWidth = imageSize.width() * scale;
        const qreal targetHeight = imageSize.height() * scale;
        targetRect = QRectF(bounds.x() + (bounds.width() - targetWidth) * 0.5,
                            bounds.y() + (bounds.height() - targetHeight) * 0.5,
                            targetWidth,
                            targetHeight);
    }

    node->setRect(targetRect);

    return node;
}

void CameraFrameItem::geometryChange(const QRectF& newGeometry,
                                     const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    update();
}
