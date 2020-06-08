#include "glguiwidget.h"
#include "guard_on.h"
#include "logger.h"
#include "exec_exit.h"
#include "block_delay.h"
#include <chrono>

GLGUIWidget::GLGUIWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    QOpenGLFunctions()
{
}

void GLGUIWidget::setThreadLock(GLGUIWidget::LockT v)
{
    lock = std::move(v);
}

void GLGUIWidget::setTextureToUse(unsigned int id)
{
    static_assert(std::is_same<unsigned int, GLuint>::value, "Woops! Revise those signals / slots.");
    tex_id = id;
    std::cout << "Texture set to id: " << id << std::endl;
}

void GLGUIWidget::initializeGL()
{
    initializeOpenGLFunctions();
    lastFramePaint = 0.f;
}

void GLGUIWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glDisable(GL_DEPTH_TEST);

    m_w = w;
    m_h = h;
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

    GLuint tid = tex_id;
    if (!tid)
        return;
    LockT l = lock;
    if (!l || l->try_lock())
    {
        exec_onexit ex([l]()
        {
            if (l)
                l->unlock();
        });
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCheckError();

        glMatrixMode(GL_PROJECTION);
        glCheckError();
        glPushMatrix();
        glCheckError();
        glLoadIdentity();
        glCheckError();
        glOrtho(0.0, m_w, 0.0, m_h, -1.0, 1.0);
        glCheckError();
        glMatrixMode(GL_MODELVIEW);
        glCheckError();
        glPushMatrix();
        glCheckError();
        glLoadIdentity();
        glCheckError();
        glDisable(GL_LIGHTING);

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

        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);

        glFlush();
        glCheckError();
    }
}
