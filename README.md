# qt-opengl-non-blocking-rendering-alexzk1
non-blocking multi-threaded OpenGL rendering in Qt 5

This is a simple proof of concept application written in C++ using Qt 5 and
OpenGL. The goal was to have the Qt GUI never "block" waiting for OpenGL
rendering (in case it is slow). This means the rendering should be done in an
asynchronous fashion. The rendering thread should be producing frames and the
main thread be consuming them, but the main thread should never wait for the
producing thread. It is acceptable for the consuming thread to wait for the
main thread. Conceptually, it is like a ring buffer.

Slow OpenGL code is simulated by an artificial 250 millisecond delay put into
the middle of the OpenGL drawing calls. For the demo app, this delay can be
enabled/disabled via the GUI.

This demo achieves the non-blocking behavior by using off-screen OpenGL
rendering to a QImage in a secondary thread. This is not ideal is there is a
performance penalty for copying so much data between threads, as well as
displaying a QImage rather than a QPixmap on a QWidget. The CPU usage seems
to be a function of the QImage resolution.

## Dependencies

* Qt5 (widgets)

The relevant development package to install on Ubuntu 18.04 is:

* qt5-default

## Compiling

```
qmake
make
```

## Running

```
./opengl_thread_test
```
