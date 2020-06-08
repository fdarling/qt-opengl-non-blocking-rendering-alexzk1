#include "mainwindow.h"
#include <QLabel>
#include <QImage>
#include <QVBoxLayout>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include "cpuusage.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent)
{
    QWidget * const dummy = new QWidget;
    {
        QVBoxLayout * const vbox = new QVBoxLayout(dummy);

#ifdef USE_QIMAGE
        imageWidget = new ImageWidget();
        vbox->addWidget(imageWidget, 1);
#else
        oglWidget = new GLGUIWidget();
        vbox->addWidget(oglWidget, 1);
#endif
        QSlider * const slider = new QSlider;
        slider->setOrientation(Qt::Horizontal);
        slider->setRange(1, 100);
        slider->setValue(50);

        //FIXME: stop use old SIGNAL/SLOT macroses. It cannot be checked by compiler
        //there are few cases where Qt has special overloads and u can use it only by SIGNAL/SLOT
        //consider that old and outdated for 7 years at least.
        //also u keep repeating data, (int) is in class header and here ... try to change 20 such
        connect(slider, SIGNAL(valueChanged(int)), this, SLOT(on_horizontalSlider_valueChanged(int)));
        vbox->addWidget(slider);

        statusLabel = new QLabel(this);
        vbox->addWidget(statusLabel);

        QPushButton * const resetButton = new QPushButton("Reset");
        connect(resetButton, SIGNAL(clicked()), this, SLOT(on_Reset_clicked()));
        vbox->addWidget(resetButton);

        QCheckBox * const lagCheckbox = new QCheckBox("Enable OpenGL Lag");
        connect(lagCheckbox, SIGNAL(toggled(bool)), this, SLOT(on_Lag_checked(bool)));
        vbox->addWidget(lagCheckbox);
    }
    setCentralWidget(dummy);
    resize(640, 480);
#ifndef USE_QIMAGE
    oglWidget->update(); //need initial update wich will create gui context so it will be shared to thread
    QTimer::singleShot(100, this, &MainWindow::delayedInit);
#else
    delayedInit();
#endif
}

MainWindow::~MainWindow()
{
    gl.reset(); //prior destroying ui
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // HACK: really we should be hooking the ImageWidget::resizeEvent(), but since they will always be resized together, this works...

    //in this example that will resize label which will resize gl which will resize label which will resize gl ...
    if (gl)
        gl->surface->resize(width(), height());
}

void MainWindow::delayedInit()
{

#ifdef USE_QIMAGE
    gl.reset(new GLManager<ExamplePaintSurface>(nullptr));
    connect(gl->thread, &ThreadedOpenGLContainer::readyRGBA8888, this, [this](const QImage & img)
    {
        imageWidget->setImage(img);
        static bool once = true;
        if (once)
        {
            img.save("image.png");
            once = false;
        }
    }, Qt::QueuedConnection); //it is important to have Qt::QueuedConnection here
#else
    gl.reset(new GLManager<ExamplePaintSurface>(oglWidget->context()));

    connect(oglWidget, SIGNAL(frameSwapped()), oglWidget, SLOT(update()));

    connect(oglWidget, &GLGUIWidget::renderGlDone, gl->thread, &ThreadedOpenGLContainer::unpause);
    connect(gl->thread, &ThreadedOpenGLContainer::readyFrameOnId, this, [this](unsigned int t)
    {
        oglWidget->setTextureToUse(t);
    }, Qt::QueuedConnection);
#endif
    gl->surface->resize(width(), height());
    lastFps = 0.f;

    //this is new syntax of C++, checked directly by compiler
    connect(gl->thread, &ThreadedOpenGLContainer::singleRunFps, this, [this](int64_t ms_per_render)
    {
        static CpuUsage usage;
        static int64_t u = usage.getCpuUsage() * 100.f;
        static uint64_t cntru = 1;
        if ((++cntru) % 10 == 0)
            u = usage.getCpuUsage() * 100.f;


        constexpr static float alpha = 0.5f;
        const float fps_now = 1000.f / ms_per_render;

        //low-pass filter
        lastFps = lastFps * alpha + fps_now * (1.f - alpha);
        if (statusLabel)
        {
#ifdef USE_QIMAGE
            statusLabel->setText(QStringLiteral("FPS (thread): %1; CPU: %2%").arg(lastFps).arg(u));
#else
            statusLabel->setText(QStringLiteral("FPS (thread): %1; CPU: %2%; GUI-Frame (ms): %3").arg(lastFps).arg(u).arg(oglWidget->lastFramePaint));
#endif
        }

    }, Qt::QueuedConnection);
    gl->thread->launch(60);
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if (gl)
        gl->surface->setScale(value);
}

void MainWindow::on_Lag_checked(bool on)
{
    if (gl)
        gl->surface->setLag(on);
}

void MainWindow::on_Reset_clicked()
{
    delayedInit();
}
