#include "mainwindow.h"
#include <QLabel>
#include <QImage>
#include <QVBoxLayout>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), imageWidget(new ImageWidget)
{
    QWidget * const dummy = new QWidget;
    {
        QVBoxLayout * const vbox = new QVBoxLayout(dummy);
        vbox->addWidget(imageWidget, 1);

        QSlider * const slider = new QSlider;
        slider->setOrientation(Qt::Horizontal);
        slider->setRange(1, 100);
        slider->setValue(50);
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
    delayedInit();
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
    gl.reset(new GLManager<ExamplePaintSurface>());
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
    gl->surface->resize(width(), height());
    lastFps = 0.f;
    connect(gl->thread, &ThreadedOpenGLContainer::singleRunFps, this, [this](int64_t ms_per_render)
    {
        constexpr static float alpha = 0.5f;
        const float fps_now = 1000.f / ms_per_render;

        //low-pass filter
        lastFps = lastFps * alpha + fps_now * (1 - alpha);
        if (statusLabel)
            statusLabel->setText(QStringLiteral("FPS: %1").arg(lastFps));

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
