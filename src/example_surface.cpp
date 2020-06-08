#include "example_surface.h"
#include <iostream>

ExamplePaintSurface::ExamplePaintSurface(QOpenGLContext *guiContext, QScreen *targetScreen, const QSize &size):
    OffscreenGL(guiContext, targetScreen, size)
{
    timer.start();
}

void ExamplePaintSurface::setScale(int s)
{
    scale = s;
}

void ExamplePaintSurface::setLag(bool on)
{
    lag = on;
}

void ExamplePaintSurface::paintGL()
{
    const auto funcptr = getFuncs();
    if (funcptr)
    {
        const float rotqube = fmodf(static_cast<float>(timer.elapsed()) / 100.0f, 360.0f);
        // Clear Screen And Depth Buffer
        funcptr->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Reset The Current Modelview Matrix
        funcptr->glLoadIdentity();

        //NEW//////////////////NEW//////////////////NEW//////////////////NEW/////////////
        const float factor = scale / 50.f;
        funcptr->glTranslatef(0.0f, 0.0f, -7.0f);   // Translate Into The Screen 7.0 Units
        funcptr->glScalef(factor, factor, factor);

        funcptr->glRotatef(rotqube, 0.0f, 1.0f, 0.0f); // Rotate The cube around the Y axis
        funcptr->glRotatef(rotqube, 1.0f, 1.0f, 1.0f);
        funcptr->glBegin(GL_QUADS);     // Draw The Cube Using quads
        funcptr->glColor3f(0.0f, 1.0f, 0.0f); // Color Blue
        funcptr->glVertex3f( 1.0f, 1.0f, -1.0f);  // Top Right Of The Quad (Top)
        funcptr->glVertex3f(-1.0f, 1.0f, -1.0f);  // Top Left Of The Quad (Top)
        funcptr->glVertex3f(-1.0f, 1.0f, 1.0f);   // Bottom Left Of The Quad (Top)
        funcptr->glVertex3f( 1.0f, 1.0f, 1.0f);   // Bottom Right Of The Quad (Top)
        funcptr->glColor3f(1.0f, 0.5f, 0.0f); // Color Orange
        funcptr->glVertex3f( 1.0f, -1.0f, 1.0f);  // Top Right Of The Quad (Bottom)
        funcptr->glVertex3f(-1.0f, -1.0f, 1.0f);  // Top Left Of The Quad (Bottom)
        funcptr->glVertex3f(-1.0f, -1.0f, -1.0f); // Bottom Left Of The Quad (Bottom)
        funcptr->glVertex3f( 1.0f, -1.0f, -1.0f); // Bottom Right Of The Quad (Bottom)
        funcptr->glColor3f(1.0f, 0.0f, 0.0f); // Color Red
        funcptr->glVertex3f( 1.0f, 1.0f, 1.0f);   // Top Right Of The Quad (Front)
        funcptr->glVertex3f(-1.0f, 1.0f, 1.0f);   // Top Left Of The Quad (Front)
        funcptr->glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom Left Of The Quad (Front)
        funcptr->glVertex3f( 1.0f, -1.0f, 1.0f);  // Bottom Right Of The Quad (Front)
        funcptr->glColor3f(1.0f, 1.0f, 0.0f); // Color Yellow
        funcptr->glVertex3f( 1.0f, -1.0f, -1.0f); // Top Right Of The Quad (Back)
        funcptr->glVertex3f(-1.0f, -1.0f, -1.0f); // Top Left Of The Quad (Back)
        funcptr->glVertex3f(-1.0f, 1.0f, -1.0f);  // Bottom Left Of The Quad (Back)
        funcptr->glVertex3f( 1.0f, 1.0f, -1.0f);  // Bottom Right Of The Quad (Back)
        funcptr->glColor3f(0.0f, 0.0f, 1.0f); // Color Blue
        funcptr->glVertex3f(-1.0f, 1.0f, 1.0f);   // Top Right Of The Quad (Left)
        funcptr->glVertex3f(-1.0f, 1.0f, -1.0f);  // Top Left Of The Quad (Left)
        funcptr->glVertex3f(-1.0f, -1.0f, -1.0f); // Bottom Left Of The Quad (Left)
        funcptr->glVertex3f(-1.0f, -1.0f, 1.0f);  // Bottom Right Of The Quad (Left)
        funcptr->glColor3f(1.0f, 0.0f, 1.0f); // Color Violet
        funcptr->glVertex3f( 1.0f, 1.0f, -1.0f);  // Top Right Of The Quad (Right)
        funcptr->glVertex3f( 1.0f, 1.0f, 1.0f);   // Top Left Of The Quad (Right)
        funcptr->glVertex3f( 1.0f, -1.0f, 1.0f);  // Bottom Left Of The Quad (Right)
        funcptr->glVertex3f( 1.0f, -1.0f, -1.0f); // Bottom Right Of The Quad (Right)
        funcptr->glEnd();           // End Drawing The Cube
    }

    if (lag)
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void ExamplePaintSurface::fboRealloacted(const QSize &sz)
{
    imageSize = sz;
    const auto funcptr = getFuncs();
    if (funcptr)
    {
        funcptr->glViewport(0, 0, sz.width(), sz.height());                    // Reset The Current Viewport
        funcptr->glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        funcptr->glEnable(GL_CULL_FACE);
        funcptr->glEnable(GL_DEPTH_TEST);
        funcptr->glDepthMask(GL_TRUE);

        funcptr->glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
        funcptr->glLoadIdentity();                                   // Reset The Projection Matrix

        // Calculate The Aspect Ratio Of The Window
        gldPerspective(45.0f, (GLfloat)sz.width() / (GLfloat)sz.height(), 0.1f, 100.0f);

        funcptr->glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
        funcptr->glLoadIdentity();                                   // Reset The Modelview Matrix
    }
}

void ExamplePaintSurface::gldPerspective(GLdouble fovx, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    // This code is based off the MESA source for gluPerspective
    // *NOTE* This assumes GL_PROJECTION is the current matrix


    GLdouble xmin, xmax, ymin, ymax;
    GLdouble m[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const auto M = [&m](int i, int j)->GLdouble *
    {
        return (m + j * 4 + i);
    };

    xmax = zNear * tan(fovx * M_PI / 360.0);
    xmin = -xmax;

    ymin = xmin / aspect;
    ymax = xmax / aspect;

    // Set up the projection matrix
    *M(0, 0) = (2.0 * zNear) / (xmax - xmin);
    *M(1, 1) = (2.0 * zNear) / (ymax - ymin);
    *M(2, 2) = -(zFar + zNear) / (zFar - zNear);

    *M(0, 2) = (xmax + xmin) / (xmax - xmin);
    *M(1, 2) = (ymax + ymin) / (ymax - ymin);
    *M(3, 2) = -1.0;

    *M(2, 3) = -(2.0 * zFar * zNear) / (zFar - zNear);

    // Add to current matrix
    getFuncs()->glMultMatrixd(m);
}
