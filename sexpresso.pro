# Qmake file for Sexpresso
TEMPLATE = lib
SEXPRESSO_OUT_ROOT = $${OUT_PWD}
CONFIG += c++14
CONFIG += staticlib

HEADERS += \
    sexpresso/sexpresso.hpp \


SOURCES += \
    sexpresso/sexpresso.cpp \
