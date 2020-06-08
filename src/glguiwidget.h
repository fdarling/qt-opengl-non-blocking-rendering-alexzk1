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
    float lastFramePaint{0.f};
signals:
    void renderGlDone();
public slots:
    void setTextureToUse(unsigned int id);
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    std::atomic<bool> emit_once{false};
    std::atomic<GLuint> tex_id{0};
    int m_w{0};
    int m_h{0};
};
