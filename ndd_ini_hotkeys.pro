TEMPLATE = lib
TARGET = ndd_ini_hotkeys
CONFIG += qt warn_on release c++17
CONFIG -= debug
QT += core gui widgets

isEmpty(NDD_SDK_ROOT) {
    NDD_SDK_ROOT = $$getenv(NDD_SDK_ROOT)
}
isEmpty(NDD_SDK_ROOT): error(Set NDD_SDK_ROOT to the ndd108 folder from the official ndd-plugin repository)

HEADERS += section_move.hpp
SOURCES += ndd_ini_hotkeys.cpp
INCLUDEPATH += "$$NDD_SDK_ROOT/include"
INCLUDEPATH += "$$NDD_SDK_ROOT/qscint/src"
INCLUDEPATH += "$$NDD_SDK_ROOT/qscint/src/Qsci"
LIBS += -L"$$NDD_SDK_ROOT/x64/Release" -lqmyedit_qt5
DESTDIR = $$PWD/build
win32:QMAKE_CXXFLAGS += /utf-8 /wd4828
