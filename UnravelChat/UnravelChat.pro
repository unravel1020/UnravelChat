QT += core widgets

CONFIG += c++11

TARGET = UnravelChat
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# 输出目录
DESTDIR = bin

# 编译器定义
DEFINES += QT_DEPRECATED_WARNINGS

# 图标和版本信息
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "Unravel Studio"
QMAKE_TARGET_PRODUCT = "UnravelChat"
QMAKE_TARGET_DESCRIPTION = "C++/Qt Chat Application"
