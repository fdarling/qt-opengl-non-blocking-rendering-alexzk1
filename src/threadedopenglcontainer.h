#pragma once

#include <QObject>
#include <QOpenGLContext>
#include "offscreengl.h"

#include "lambda_thread.h"

class ThreadedOpenGLContainer : public QObject
{
    Q_OBJECT
public:
    explicit ThreadedOpenGLContainer(OffscreenGL* surf, QObject* parent = nullptr);
    ~ThreadedOpenGLContainer() override;
public slots:
    void launch(int fps_limit = 0);
signals:
    void readyRGBA8888(const QImage& img); //launched from thread, maybe skip if offscreen renders to texture
    void singleRunFps(int64_t elapsed); //launched from thread
    void readyFrame(); // always emmitted
protected:
    LambdaThreadPtr thread{nullptr};

    void ensureThreadEnded();
private:
    OffscreenGL* surf{nullptr};
    QImage lastImage;

    void startThread(int fps_limit);
    void renderStep();
};

//just convinient memory managment together of offscreen and thread renderer
template <class Offscreen>
struct GLManager
{
    Offscreen *surface{nullptr};
    ThreadedOpenGLContainer* thread{nullptr};

    GLManager(QOpenGLContext *c)
    {
        surface = new Offscreen(c);
        thread = new ThreadedOpenGLContainer(surface);
    }

    ~GLManager()
    {
        reset();
    }

    void reset()
    {
        delete thread;
        delete surface;

        thread = nullptr;
        surface = nullptr;
    }
};
