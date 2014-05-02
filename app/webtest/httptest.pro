#-------------------------------------------------
#
# Project created by QtCreator 2013-10-21T18:37:11
#
#-------------------------------------------------

QT       += core

QT       -= gui

include (../../../../vdream/vdream90/lib/vdream.pri)

TARGET = httptest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DESTDIR  = ../../bin

SOURCES += main.cpp \
    ../common/httptest.cpp \
    ../common/changehttprequest.cpp

HEADERS += \
    ../common/httptest.h \
    ../common/changehttprequest.h
