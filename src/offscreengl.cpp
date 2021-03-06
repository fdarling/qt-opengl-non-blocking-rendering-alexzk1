#include "offscreengl.h"
#include <QApplication>
#include <QThread>
#include "logger.h"
#include "bind_release_wrap.h"

OffscreenGL::OffscreenGL(QOpenGLContext* guiContext, QScreen *targetScreen, const QSize &size, QObject *parent) :
    QObject(parent),
    guiContext(guiContext)
{
    if (QThread::currentThread() != QApplication::instance()->thread())
        throw std::runtime_error("Offscreen must be constructed from GUI thread!");

    bufferSize.set(size);

    surface = new QOffscreenSurface(targetScreen);

    auto fmt = QSurfaceFormat::defaultFormat();
    fmt.setSwapInterval(0); //disable vsync
    fmt.setSwapBehavior(QSurfaceFormat::SingleBuffer);

    surface->setFormat(fmt);
    surface->create();

    m_context = new QOpenGLContext();
    m_context->setFormat(surface->format());
    if (guiContext)
        m_context->setShareContext(guiContext);

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

QPaintDevice *OffscreenGL::getPaintDevice() const
{
    return m_paintDevice.get();
}

GLuint OffscreenGL::render()
{
    prepareContext();
    GLuint r = 0;

    const std::shared_ptr<QOpenGLFramebufferObject> tmp[] = {fbo, fbo2};
    const auto buf = (uses_texture()) ? tmp[buffer_index] : fbo;

    if (buf)
    {
        r = buf->texture();
        BIND_PTR(buf);
        glCheckError();
        paintGL();
        glFlush();
    }
    glCheckError();

    return r;
}

void OffscreenGL::flip()
{
    buffer_index = 1 - buffer_index;
}

QImage OffscreenGL::getImage() const
{
    return grabNotMultiSample();
}

bool OffscreenGL::uses_texture() const
{
    return guiContext != nullptr;
}

void OffscreenGL::allocFbo()
{
    buffer_index = 0;
    QOpenGLFramebufferObjectFormat format;
    format.setSamples(0);

    const auto sz = bufferSize.get();

    //say NO to copy-paste - use lambdas!
    const auto make_fbo = [&sz, &format]()
    {
        std::shared_ptr<QOpenGLFramebufferObject> fbo;
        fbo.reset(new QOpenGLFramebufferObject(sz, format), [](QOpenGLFramebufferObject * p)
        {
            //deletor lambda takes care of release, so no need special functions be called
            //just make new pointer
            if (p)
            {
                if (p->isBound())
                    p->release();

                delete p;
            }
        });
        if (!fbo->isValid())
            throw std::runtime_error("OffscreenGL::allocFbo() - Failed to create background FBO!");
        return fbo;
    };

    fbo = make_fbo();

    if (uses_texture())
        fbo2 = make_fbo();


    m_paintDevice.reset(new QOpenGLPaintDevice());
    m_paintDevice->setSize(sz);

    // clear framebuffer
    if (const auto f = getFuncs())
    {
        BIND_PTR(fbo);
        f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        fboRealloacted(sz);
    }
}

QImage OffscreenGL::grabNotMultiSample() const
{
    //to add multisample buffers see deleted folder or
    //https://stackoverflow.com/questions/31323749/easiest-way-for-offscreen-rendering-with-qopenglwidget
    //however that original code is real spagetti there

    QImage image;
    const auto f = getFuncs();
    if (fbo && f)
    {
        f->glFlush();
        f->glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle());

        f->glReadBuffer(GL_COLOR_ATTACHMENT0);
        GLenum internalFormat = fbo->format().internalTextureFormat();
        const bool hasAlpha = internalFormat == GL_RGBA || internalFormat == GL_BGRA || internalFormat == GL_RGBA8;
        if (internalFormat == GL_BGRA)
        {
            image = QImage(fbo->size(), hasAlpha ? QImage::Format_ARGB32 : QImage::Format_RGB32);
            f->glReadPixels(0, 0, fbo->size().width(),
                            fbo->size().height(), GL_BGRA, GL_UNSIGNED_BYTE, image.bits());
        }
        else
        {
            if ((internalFormat == GL_RGBA) || (internalFormat == GL_RGBA8))
            {
                image = QImage(fbo->size(), hasAlpha ? QImage::Format_RGBA8888 : QImage::Format_RGBX8888);
                f->glReadPixels(0, 0, fbo->size().width(),
                                fbo->size().height(), GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
            }
            else
            {
                qDebug() << "OpenGlOffscreenSurface::grabFramebuffer() - Unsupported framebuffer format"
                         << internalFormat << "!";
            }
        }
        f->glBindFramebuffer(GL_FRAMEBUFFER, fbo->handle());
    }

    return image.mirrored();
}
