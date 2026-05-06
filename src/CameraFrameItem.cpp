#include "CameraFrameItem.h"

#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
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
        m_pendingImage = QImage();
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

    QImage image;

    if (channel == 1) {
        image = QImage(
                    imageByteAddr,
                    width,
                    height,
                    width,
                    QImage::Format_Grayscale8
                    ).copy();
    } else if (channel == 3) {
        image = QImage(
                    imageByteAddr,
                    width,
                    height,
                    width * 3,
                    QImage::Format_RGB888
                    ).copy();
    } else {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        m_pendingImage = image;
    }

    update();
}

QSGNode* CameraFrameItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*)
{
    QImage image;
    {
        QMutexLocker locker(&m_mutex);
        image = m_pendingImage;
    }

    if (image.isNull() || !window()) {
        delete oldNode;
        return nullptr;
    }

    auto* node = static_cast<QSGSimpleTextureNode*>(oldNode);
    if (!node) {
        node = new QSGSimpleTextureNode();
    }

    QSGTexture* texture = window()->createTextureFromImage(image);

    if (!texture) {
        delete node;
        return nullptr;
    }

    node->setTexture(texture);
    node->setOwnsTexture(true);
    node->setRect(boundingRect());

    return node;
}

void CameraFrameItem::geometryChange(const QRectF& newGeometry,
                                     const QRectF& oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    update();
}
