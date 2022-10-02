TEMPLATE = app

QT += widgets

TARGET = CQQuinto

DEPENDPATH += .

QMAKE_CXXFLAGS += -std=c++17

#CONFIG += debug

# Input
SOURCES += \
CQQuinto.cpp \
CQPixmapCache.cpp \

HEADERS += \
CQQuinto.h \
CQPixmapCache.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj
LIB_DIR     = ../lib

INCLUDEPATH += \
../include \
.

unix:LIBS += \
-L$$LIB_DIR \
