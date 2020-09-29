#-------------------------------------------------
#
# Project created by QtCreator 2013-10-04T09:38:11
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT       += network

TARGET = sClient
TEMPLATE = app

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp\
    mainwindow.cpp \
    dlgconnect.cpp \
    dlgaliases.cpp \
    util.cpp

HEADERS  += mainwindow.h \
    dlgconnect.h \
    dlgaliases.h \
    util.h

FORMS    += mainwindow.ui \
    dlgconnect.ui \
    dlgaliases.ui
