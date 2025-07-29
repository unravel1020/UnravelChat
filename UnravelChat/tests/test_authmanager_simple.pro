QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = test_authmanager_simple
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    test_authmanager_simple.cpp \
    ../src/auth/AuthenticationManager.cpp \
    ../src/auth/UserDatabase.cpp \
    ../src/auth/PasswordManager.cpp

HEADERS += \
    ../include/auth/AuthenticationManager.h \
    ../include/auth/UserDatabase.h \
    ../include/auth/PasswordManager.h

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests
