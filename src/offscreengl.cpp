#include "offscreengl.h"
#include <QApplication>
#include <QThread>

OffscreenGL::OffscreenGL(QScreen *targetScreen, const QSize &size, QObject *parent) :
    QObject(parent)
{
    if (QThread::currentThread() != QApplication::instance()->thread())
        throw std::runtime_error("Offscreen must be constructed from GUI thread!");

    bufferSize.set(size);

    surface = new QOffscreenSurface(targetScreen);
    surface->setFormat(QSurfaceFormat::defaultFormat());
    surface->create();

    m_context = new QOpenGLContext();
    m_context->setFormat(surface->format());
    if (m_context->create())
    {

    }
    else
    {
        delete m_context;
        m_context = nullptr;
        throw std::runtime_error("Failed to create OpenGL context!");
    }
}

OffscreenGL::~OffscreenGL()
{
    if (QThread::currentThread() != QApplication::instance()->thread())
        throw std::runtime_error("Offscreen must be destroyed from GUI thread!");

    releaseContext();
    delete m_context;

    surface->destroy();
    delete surface;
    surface = nullptr;
}

void OffscreenGL::setOwningThread(QThread *owner)
{
    moveToThread(owner);
    surface->moveToThread(owner);

    if (m_context)
    {
        releaseContext();
        m_context->moveToThread(owner);
    }
}

void OffscreenGL::prepareContext()
{
    /*
     * Once a QOpenGLContext has been made current, you can render to it in a platform independent way by using Qt's OpenGL
     * enablers such as QOpenGLFunctions, QOpenGLBuffer, QOpenGLShaderProgram, and QOpenGLFramebufferObject.
     * It is also possible to use the platform's OpenGL API directly, without using the Qt enablers
    */
    const bool needprep = !isPrepared.exchange(true);

    if (needprep)
    {
        m_context->makeCurrent(surface);

        /*
         * There is no need to call QAbstractOpenGLFunctions::initializeOpenGLFunctions() as long
         * as this context is current.
        */
        m_functions_3_0 = m_context->versionFunctions<QOpenGLFunctions_3_0>();
    }

    if (sizeChanged.exchange(false) || needprep)
        allocFbo();
}

void OffscreenGL::releaseContext()
{
    m_functions_3_0 = nullptr;
    fbo.reset();
    m_paintDevice.reset();
    m_context->doneCurrent();
    isPrepared = false;
}

void OffscreenGL::resize(int w, int h)
{
    bufferSize.set({w, h});
    sizeChanged = true;
}

void OffscreenGL::fboRealloacted(const QSize &size)
{
    (void)size;
}

QOpenGLFramebufferObject *OffscreenGL::getFbo() const
{
    return fbo.get();
}

QPaintDevice *OffscreenGL::getPaintDevice() const
{
    return m_paintDevice.get();
}

void OffscreenGL::render()
{
    prepareContext();
    if (fbo)
    {
        fbo->bind();
        paintGL();
        fbo->release();
    }
}

QImage OffscreenGL::getImage() const
{
    return grabNotMultiSample();
}

void OffscreenGL::allocFbo()
{
    QOpenGLFramebufferObjectFormat format;
    format.setSamples(0);

    const auto sz = bufferSize.get();

    //deletor lambda takes care of release, so no need special functions be called
    //just make new pointer

    fbo.reset(new QOpenGLFramebufferObject(sz, format), [](QOpenGLFramebufferObject * p)
    {
        if (p)
        {
            p->release();
            delete p;
        }
    });

    if (!fbo->isValid())
        throw std::runtime_error("OffscreenGL::allocFbo() - Failed to create background FBO!");

    // clear framebuffer
    if (m_functions_3_0)
    {
        fbo->bind();
        m_functions_3_0->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        fbo->release();
    }

    m_paintDevice.reset(new QOpenGLPaintDevice());
    m_paintDevice->setSize(sz);

    fbo->bind();
    fboRealloacted(sz);
    fbo->release();
}

QImage OffscreenGL::grabNotMultiSample() const
{
    //to add multisample buffers see deleted folder or
    //https://stackoverflow.com/questions/31323749/easiest-way-for-offscreen-rendering-with-qopenglwidget
    //however that original code is real spagetti there

    QImage image;

    if (fbo && m_functions_3_0)
    {
        m_functions_3_0->glFlush();
        m_functions_3_0->glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle());
        if (m_functions_3_0)
            m_functions_3_0->glReadBuffer(GL_COLOR_ATTACHMENT0);
        GLenum internalFormat = fbo->format().internalTextureFormat();
        const bool hasAlpha = internalFormat == GL_RGBA || internalFormat == GL_BGRA || internalFormat == GL_RGBA8;
        if (internalFormat == GL_BGRA)
        {
            image = QImage(fbo->size(), hasAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
            m_functions_3_0->glReadPixels(0, 0, fbo->size().width(),
                                          fbo->size().height(), GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
        }
        else
        {
            if ((internalFormat == GL_RGBA) || (internalFormat == GL_RGBA8))
            {
                image = QImage(fbo->size(), hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGBX8888);
                m_functions_3_0->glReadPixels(0, 0, fbo->size().width(),
                                              fbo->size().height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
            }
            else
            {
                qDebug() << "OpenGlOffscreenSurface::grabFramebuffer() - Unsupported framebuffer format"
                         << internalFormat << "!";
            }
        }
        m_functions_3_0->glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle());
    }

    return image.mirrored();
}