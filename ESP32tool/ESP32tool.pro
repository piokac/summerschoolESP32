#-------------------------------------------------
#
# Project created by QtCreator 2017-07-10T10:40:31
#
#-------------------------------------------------

QT       += core gui serialport
QT += network
QT += opengl
#LIBS+= -lglew32 -lGLFW3 -lopengl32 -lglu32


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ESP32tool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    esp32data.cpp \
    serialportsettings.cpp \
    csvread.cpp \
    logger.cpp \
    wifisettings.cpp \
    madgwickahrs.cpp \
    visualisationgl.cpp \
    visualisationwindow.cpp \
    glwidget.cpp \
    geometryengine.cpp

HEADERS  += mainwindow.h \
    esp32data.h \
    serialportsettings.h \
    csvread.h \
    logger.h \
    wifisettings.h \
    madgwickahrs.h \
    visualisationgl.h \
    visualisationwindow.h \
    glwidget.h \
    geometryengine.h

FORMS    += mainwindow.ui \
    serialportsettings.ui \
    csvread.ui \
    wifisettings.ui

RESOURCES += \
    icons.qrc \
    shaders.qrc \
    textures.qrc
