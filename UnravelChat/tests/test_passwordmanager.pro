QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = test_passwordmanager
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    test_passwordmanager.cpp \
    ../src/auth/PasswordManager.cpp

HEADERS += \
    ../include/auth/PasswordManager.h

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests

