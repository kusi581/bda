QT += core
QT -= gui

TARGET = SignalManager
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    config.cpp \
    common.cpp

HEADERS += \
    config.h \
    common.h \
    main.h

