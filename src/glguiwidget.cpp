#include "glguiwidget.h"
#include "guard_on.h"
#include "logger.h"
#include "exec_exit.h"
#include "block_delay.h"
#include <chrono>
#include "lambda_thread.h"

GLGUIWidget::GLGUIWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    QOpenGLFunctions()
{
}

void GLGUIWidget::setTextureToUse(unsigned int id)
{
    static_assert(std::is_same<unsigned int, GLuint>::value, "Woops! Revise those signals / slots.");
    emit_once = tex_id.exchange(id) != id;

    //std::cout << "ID: " << id << std::endl;
}

void GLGUIWidget::initializeGL()
{
    initializeOpenGLFunctions();
    lastFramePaint = 0.f;
}

void GLGUIWidget::resizeGL(int w, int h)
{
    m_w = w;
    m_h = h;

    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_LIGHTING);

    glCheckError();
}

void GLGUIWidget::paintGL()
{
    const std::chrono::steady_clock::time_point sted_b{std::chrono::steady_clock::now()};

    exec_onexit ex1([&]()
    {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - sted_b);
        constexpr static float alpha = 0.5f;
        //low-pass filter
        lastFramePaint = lastFramePaint * alpha + elapsed.count() * (1.f - alpha);
    });

    const bool can_emit = testandflip(emit_once, true);
    GLuint tid = tex_id;

    if (!tid)
        return;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glCheckError();

    glBindTexture(GL_TEXTURE_2D, tid);
    glCheckError();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0, 0, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(m_w, 0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(m_w, m_h,  0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0,  m_h,  0.0f);
    glEnd();
    glCheckError();

    glBindTexture(GL_TEXTURE_2D, 0);
    glCheckError();

    glDisable(GL_TEXTURE_2D);
    glCheckError();

    glFlush();
    glCheckError();

    if (can_emit)
        emit renderGlDone();
}
