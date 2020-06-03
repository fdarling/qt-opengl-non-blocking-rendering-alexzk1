#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QImage>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    delayedInit();
    rotator = new QTimer(this);
    connect(rotator, &QTimer::timeout, this, [this]()
    {
        if (gl)
            gl->surface->addAngle(1);
    });
    rotator->start(100);
}

MainWindow::~MainWindow()
{
    gl.reset(); //prior destroying ui
    delete ui;
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
        ui->label->setPixmap(QPixmap::fromImage(img));
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
