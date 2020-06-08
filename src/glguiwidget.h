#pragma once

#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <mutex>

class GLGUIWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit GLGUIWidget(QWidget *parent = nullptr);
    ~GLGUIWidget() override = default;
public slots:
    void setTextureToUse(unsigned int id);
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    //using pointer to int id, so nullptr means "not set yet" like in C#
    //and mutex just in case, as we have threads everywhere
    mutable std::recursive_mutex tex_mut;
    std::shared_ptr<GLuint> tex_id{nullptr};
    int m_w{0};
    int m_h{0};
};
