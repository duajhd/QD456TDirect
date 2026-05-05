#ifndef CAMERAFRAMEITEM_H
#define CAMERAFRAMEITEM_H

#include <QQuickItem>
#include <QImage>
#include <QMutex>

#include "CameraViewModel.h"

class CameraFrameItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QObject* cameraVm READ cameraVm WRITE setCameraVm NOTIFY cameraVmChanged)

public:
    explicit CameraFrameItem(QQuickItem* parent = nullptr);
    ~CameraFrameItem() override;

    QObject* cameraVm() const;
    void setCameraVm(QObject* vmObj);

signals:
    void cameraVmChanged();

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data) override;
    void geometryChange(const QRectF& newGeometry, const QRectF& oldGeometry) override;

private slots:
    void onFrameUpdated(int frameId);

private:
    CameraViewModel* m_cameraVm = nullptr;

    QMutex m_mutex;
    QImage m_pendingImage;
};

#endif // CAMERAFRAMEITEM_H
