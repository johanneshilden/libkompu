TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.c \
    comp.c \
    tmachine.c \
    lcalc.c \
    buf.c \
    comp_serialize.c

HEADERS += \
    comp.h \
    tmachine.h \
    lcalc.h \
    buf.h \
    comp_serialize.h

