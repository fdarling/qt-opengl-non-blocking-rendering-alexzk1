#include "threadedopenglcontainer.h"
#include <QApplication>
#include <QPainter>
#include <QOpenGLTexture>
#include "block_delay.h"
#include "guard_on.h"

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

ThreadedOpenGLContainer::MutexTPtr ThreadedOpenGLContainer::getRenderLock() const
{
    return renderLock;
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
            DelayMeasuredIn elapsed;
            {
                DelayBlockMs<DelayMeasuredIn> delay(DELAY, &elapsed);//defines FPS, however it is MS delay ...
                (void)delay;
                std::lock_guard<MutexT> grd(*renderLock);
                renderStep();
            }
            emit singleRunFps(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
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
    if (!surf->uses_texture())
    {
        lastImage = (std::move(surf->getImage().convertToFormat(QImage::Format_RGBA8888)));
        emit readyRGBA8888(lastImage);
    }
    emit readyFrame();
}
