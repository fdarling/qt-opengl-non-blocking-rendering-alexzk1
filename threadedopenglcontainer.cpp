#include "threadedopenglcontainer.h"
#include <QApplication>
#include <QPainter>
#include <QOpenGLTexture>
#include "block_delay.h"

static_assert(true, "This is C++ version test. Will fail if below C++11. We need C++11 at least.");

using DelayMeasuredIn = std::chrono::milliseconds;

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

void ThreadedOpenGLContainer::launch(int fps_limit)
{
    startThread(fps_limit);
}

void ThreadedOpenGLContainer::ensureThreadEnded()
{
    thread.reset();
}

void ThreadedOpenGLContainer::startThread(int fps_limit)
{
    if (!surf)
        throw std::runtime_error("Offscreen surface was not supplied.");

    thread = createInQThread([this, fps_limit](TerminateIfTruePtr stopper)
    {
        const DelayMeasuredIn DELAY = std::chrono::duration_cast<DelayMeasuredIn>(std::chrono::milliseconds((fps_limit) > 0 ? static_cast<int32_t>(1000.f / fps_limit) : 1));
        while (!(*stopper))
        {
            if (fps_limit > 0)
            {
                DelayBlockMs<DelayMeasuredIn> delay(DELAY);//defines FPS, however it is MS delay ...
                (void)delay;
            }
            renderStep();
        }

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
