#pragma once

#include <QObject>
#include "offscreengl.h"

#include "lambda_thread.h"

class ThreadedOpenGLContainer : public QObject
{
    Q_OBJECT
public:
    explicit ThreadedOpenGLContainer(OffscreenGL* surf, QObject* parent = nullptr);
    ~ThreadedOpenGLContainer() override;
public slots:
    void launch();

signals:
    void readyRGBA8888(const QImage& img);
protected:
    LambdaThreadPtr thread{nullptr};
    void ensureThreadEnded();
private:

    OffscreenGL* surf{nullptr};
    QImage lastImage;

    void startThread();
    void renderStep();
};

//just convinient memory managment together of offscreen and thread renderer
template <class Offscreen>
struct GLManager
{
    Offscreen *surface{nullptr};
    ThreadedOpenGLContainer* thread{nullptr};

    GLManager()
    {
        surface = new Offscreen();
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