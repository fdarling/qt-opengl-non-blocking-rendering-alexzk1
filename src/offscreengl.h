#pragma once

#include <QObject>
#include <QImage>
#include <QtGui/QScreen>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLFunctions_3_0>
#include <QtGui/QOpenGLFramebufferObject>
#include <QtGui/QSurfaceFormat>
#include <QOpenGLShaderProgram>
#include <QOpenGLPaintDevice>
#include <atomic>
#include <mutex>
#include <thread>
#include <QOpenGLTexture>
#include <QPointer>

#include "locked_object.h"
class QThread;

class OffscreenGL : public QObject
{
    Q_OBJECT
public:
    explicit OffscreenGL(QOpenGLContext* guiContext = nullptr, QScreen* targetScreen = nullptr, const QSize& size = QSize (1, 1), QObject *parent = nullptr);
    ~OffscreenGL() override;
    void setOwningThread(QThread *owner);
    void prepareContext();
    void releaseContext();
    void render();

    QImage getImage() const;
    bool uses_texture() const;

public slots:
    void resize(int w, int h);
signals:
    void hasTextureId(unsigned int tex_id);
protected:
    virtual void paintGL() = 0;
    virtual void fboRealloacted(const QSize& size);

    QOpenGLFunctions_3_0* getFuncs() const
    {
        return m_functions_3_0;
    }

protected:
    QOffscreenSurface* surface{nullptr};
    QOpenGLContext* m_context{nullptr};
    LockedObject<QSize, std::recursive_mutex> bufferSize;
    QOpenGLFramebufferObject* getFbo() const;
    QPaintDevice* getPaintDevice() const;
private:
    QOpenGLFunctions_3_0* m_functions_3_0{nullptr};
    std::atomic<bool> isPrepared{false};
    std::atomic<bool> sizeChanged{false};
    std::shared_ptr<QOpenGLFramebufferObject> fbo{nullptr};
    std::shared_ptr<QOpenGLPaintDevice> m_paintDevice{nullptr};
    std::shared_ptr<QOpenGLTexture> m_texture{nullptr};
    QPointer<QOpenGLContext> guiContext;
    void allocFbo();

    QImage grabNotMultiSample() const;
};
