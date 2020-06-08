#pragma once

#include <QObject>
#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <mutex>
#include "threadedopenglcontainer.h"

class GLGUIWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:

    using LockT = ThreadedOpenGLContainer::MutexTPtr;

    explicit GLGUIWidget(QWidget *parent = nullptr);
    ~GLGUIWidget() override = default;

    void setThreadLock(LockT v);

    float lastFramePaint{0.f};

public slots:
    void setTextureToUse(unsigned int id);
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    LockT lock{nullptr};
    std::atomic<GLuint> tex_id{0};
    int m_w{0};
    int m_h{0};
};
