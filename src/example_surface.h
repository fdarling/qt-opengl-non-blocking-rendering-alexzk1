#pragma once
#include <QPainter>
#include <QObject>
#include <thread>
#include <chrono>
#include <math.h>
#include "offscreengl.h"
#include <QElapsedTimer>
#include <stdint.h>

//real class wich do real draw
class ExamplePaintSurface : public OffscreenGL
{
    Q_OBJECT
public:
    ExamplePaintSurface(QScreen*  targetScreen = nullptr, const QSize& size = QSize (1, 1));
    ~ExamplePaintSurface() override = default;
public slots:
    void setScale(int s); //0-100
    void setLag(bool on);
protected:
    QElapsedTimer timer;
    QSize imageSize;
    std::atomic<int32_t> scale{50};
    std::atomic<bool> lag{false};
    void paintGL() override;
    void fboRealloacted(const QSize& sz) override;
    void gldPerspective(GLdouble fovx, GLdouble aspect, GLdouble zNear, GLdouble zFar);
};
