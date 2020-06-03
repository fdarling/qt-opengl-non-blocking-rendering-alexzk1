#include "threadedopenglcontainer.h"
#include <QApplication>
#include <QPainter>
#include <QOpenGLTexture>

static_assert(true, "This is C++ version test. Will fail if below C++11. We need C++11 at least.");

ThreadedOpenGLContainer::ThreadedOpenGLContainer(OffscreenGL *surf, QObject *parent):
    QObject(parent),
    surf(surf)
{
    if (QThread::currentThread() != QApplication::instance()->thread())
        throw std::runtime_error("Widget must be constructed from GUI thread!");
}

ThreadedOpenGLContainer::~ThreadedOpenGLContainer()
{
    ensureThreadEnded();
}

void ThreadedOpenGLContainer::launch()
{
    startThread();
}

void ThreadedOpenGLContainer::ensureThreadEnded()
{
    thread.reset();
}

void ThreadedOpenGLContainer::startThread()
{
    if (!surf)
        throw std::runtime_error("Offscreen surface was not supplied.");

    thread = createInQThread([this](TerminateIfTruePtr stopper)
    {
        while (!(*stopper))
            renderStep();

    }, [this]()
    {
        const auto t = QApplication::instance()->thread();
        if (t->isRunning())
            surf->setOwningThread(t);
    });
    surf->setOwningThread(thread.get());
    thread->start();
}

void ThreadedOpenGLContainer::renderStep()
{
    surf->render();
    lastImage = (std::move(surf->getImage().convertToFormat(QImage::Format_RGBA8888)));
    emit readyRGBA8888(lastImage);
}
