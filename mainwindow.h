#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "example_surface.h"
#include "threadedopenglcontainer.h"
#include <QTimer>
#include <QPointer>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

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

private:
    Ui::MainWindow *ui;
    std::shared_ptr<GLManager<ExamplePaintSurface>> gl{nullptr};
    QPointer<QTimer> rotator{nullptr};
};
#endif // MAINWINDOW_H
