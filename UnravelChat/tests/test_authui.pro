QT += core widgets testlib network
QT -= gui

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TARGET = test_authui
TEMPLATE = app

INCLUDEPATH += ../include

SOURCES += \
    test_authui.cpp \
    ../src/auth/AuthenticationManager.cpp \
    ../src/auth/SessionManager.cpp \
    ../src/auth/PasswordManager.cpp \
    ../src/auth/UserDatabase.cpp

HEADERS += \
    ../include/auth/AuthenticationManager.h \
    ../include/auth/SessionManager.h \
    ../include/auth/PasswordManager.h \
    ../include/auth/UserDatabase.h

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests
MOC_DIR = ../build/tests
RCC_DIR = ../build/tests

LIBS += -lcrypto

unix:!macx {
    LIBS += -ldl
}

win32 {
    LIBS += -lws2_32
}

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += TESTING_MODE
EOF