#-------------------------------------------------
#
# Project created by QtCreator 2013-04-28T14:45:32
#
#-------------------------------------------------

#include(lib/qextserialport.pri)

QT       += core gui xml serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sterm
TEMPLATE = app

INCLUDEPATH += include/

SOURCES += \
    src/main.cc \
    src/terminal.cc \
    src/userinput.cc \
    src/serialport.cc \
    src/control.cc \
    src/xmlcontrol.cc \
    src/serialport_utils.cc

HEADERS  += \
    include/terminal.h \
    include/userinput.h \
    include/serialport.h \
    include/control.h \
    include/xmlcontrol.h \
    include/serialport_utils.h

FORMS    += \
    forms/terminal.ui

win32{
QMAKE_CXXFLAGS += -std=c++11
}

unix{
QMAKE_CXXFLAGS += -std=c++11
}

RESOURCES += \
    resources.qrc

RC_FILE = rcfile.rc
