#-------------------------------------------------
#
# Project created by QtCreator 2017-07-10T10:40:31
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ESP32tool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    esp32data.cpp \
    serialportsettings.cpp \
    csvread.cpp

HEADERS  += mainwindow.h \
    esp32data.h \
    serialportsettings.h \
    csvread.h

FORMS    += mainwindow.ui \
    serialportsettings.ui \
    csvread.ui

RESOURCES += \
    icons.qrc
