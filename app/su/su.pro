#-------------------------------------------------
#
# Project created by QtCreator 2013-10-19T04:24:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include (../../../../vdream/vdream90/lib/vdream.pri)

TARGET   = su
TEMPLATE = app
DESTDIR  = ../../bin
win32:RC_FILE = su.rc

LIBS    += -lWininet

SOURCES += main.cpp\
    ../common/webtest.cpp \
    ../common/changehttprequest.cpp \
    bypasshttpproxy.cpp \
    hostmgr.cpp \
    siteunblocker.cpp \
    maindlg.cpp \
    optiondlg.cpp

HEADERS  += \
    ../common/webtest.h \
    ../common/changehttprequest.h \
    bypasshttpproxy.h \
    hostmgr.h \
    siteunblocker.h \
    maindlg.h \
    optiondlg.h

FORMS    += \
    maindlg.ui \
    optiondlg.ui

RESOURCES += \
    su.qrc
