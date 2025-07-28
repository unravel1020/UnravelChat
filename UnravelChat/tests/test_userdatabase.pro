QT += core
QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = test_userdatabase
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    test_userdatabase.cpp \
    ../src/auth/UserDatabase.cpp

HEADERS += \
    ../include/auth/UserDatabase.h

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests

