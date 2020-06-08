#include "glguiwidget.h"
#include "guard_on.h"
#include "logger.h"

GLGUIWidget::GLGUIWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    QOpenGLFunctions()
{
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
    GLuint tid = tex_id;
    if (!tid)
        return;

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
