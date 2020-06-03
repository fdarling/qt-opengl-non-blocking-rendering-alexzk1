#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "example_surface.h"
#include "threadedopenglcontainer.h"
#include "imagewidget.h"
#include <QTimer>
#include <QPointer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void delayedInit();
    void on_horizontalSlider_valueChanged(int value);
    void on_Lag_checked(bool on);

    void on_Reset_clicked();

private:
    ImageWidget *imageWidget;
    std::shared_ptr<GLManager<ExamplePaintSurface>> gl{nullptr};
};
#endif // MAINWINDOW_H
