#-------------------------------------------------
#
# Project created by QtCreator 2011-03-19T00:21:58
#
#-------------------------------------------------

QT       += core gui network

TARGET = ChooseVictim
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    arprequest.cpp \
    packet.cpp

HEADERS  += mainwindow.h \
    arprequest.h \
    arp.h \
    eth.h \
    packet.h

FORMS    += mainwindow.ui