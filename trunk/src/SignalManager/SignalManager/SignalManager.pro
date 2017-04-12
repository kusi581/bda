QT += core
QT -= gui

TARGET = SignalManager
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    config.cpp \
    common.cpp \
    dspmanager.cpp

HEADERS += \
    config.h \
    common.h \
    main.h \
    dspmanager.h

