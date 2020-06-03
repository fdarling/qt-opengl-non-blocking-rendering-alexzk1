#include "mainwindow.h"
#include <QLabel>
#include <QImage>
#include <QVBoxLayout>
#include <QSlider>
#include <QPushButton>

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
        
        QPushButton * const resetButton = new QPushButton("Reset");
        connect(resetButton, SIGNAL(clicked()), this, SLOT(on_Reset_clicked()));
        vbox->addWidget(resetButton);
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

    //in this example that will resize label which will resize gl which will resize label which will resize gl ...
    //if (gl)
    //  gl->surface->resize(width(), height());
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
    gl->thread->launch();
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    if (gl)
        gl->surface->setScale(value);
}

void MainWindow::on_Reset_clicked()
{
    delayedInit();
}
