#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //imoprtant! making all gl contexts shared
    /*
     * Enables resource sharing between the OpenGL contexts used by classes like QOpenGLWidget and QQuickWidget.
     * This allows sharing OpenGL resources, like textures, between QOpenGLWidget instances that belong
     * to different top-level windows.
     * This attribute must be set before QGuiApplication is constructed. This value was added in Qt 5.4.
    */

    //UPDATE: this DOES NOT WORK
    //https://forum.qt.io/topic/49164/explicit-context-sharing-with-qopenglwidget/8

    //QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
