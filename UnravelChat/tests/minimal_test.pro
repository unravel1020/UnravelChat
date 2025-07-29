QT += core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = minimal_test
TEMPLATE = app

SOURCES += minimal_test.cpp

DESTDIR = ../bin
OBJECTS_DIR = ../build/tests

