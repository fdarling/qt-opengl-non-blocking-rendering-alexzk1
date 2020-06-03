#pragma once
#include <QPainter>
#include <QObject>
#include <thread>
#include <chrono>
#include <math.h>
#include "offscreengl.h"

//real class wich do real draw
class ExamplePaintSurface : public OffscreenGL
{
    Q_OBJECT
public:
    ExamplePaintSurface(QScreen*  targetScreen = nullptr, const QSize& size = QSize (1, 1));
    ~ExamplePaintSurface() override = default;
public slots:
    void setAngle(int a);
    void addAngle(int a);
    void setScale(int s); //0-100
protected:
    std::atomic<int32_t> angle{0};
    std::atomic<int32_t> scale{50};
    void paintGL() override;
    void fboRealloacted(const QSize& sz) override;
    void gldPerspective(GLdouble fovx, GLdouble aspect, GLdouble zNear, GLdouble zFar);
};
