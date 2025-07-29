QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = test_sessionmanager
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    test_sessionmanager.cpp \
    ../src/auth/SessionManager.cpp

HEADERS += \
    ../include/auth/SessionManager.h

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests

