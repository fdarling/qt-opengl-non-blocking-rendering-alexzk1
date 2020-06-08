QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#uncomment to use initial qimage dump
#DEFINES += USE_QIMAGE

SOURCES += \
    src/cpuusage.cpp \
    src/example_surface.cpp \
    src/glguiwidget.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/imagewidget.cpp \
    src/offscreengl.cpp \
    src/threadedopenglcontainer.cpp

HEADERS += \
    src/bind_release_wrap.h \
    src/cm_ctors.h \
    src/cpuusage.h \
    src/example_surface.h \
    src/exec_exit.h \
    src/glguiwidget.h \
    src/guard_on.h \
    src/lambda_thread.h \
    src/locked_object.h \
    src/logger.h \
    src/mainwindow.h \
    src/imagewidget.h \
    src/offscreengl.h \
    src/strfmt.h \
    src/threadedopenglcontainer.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
