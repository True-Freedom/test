TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += c++11
#CONFIG += -std=c++0x
#CONFIG += -std=c++11
#CONFIG += -std=gnu++0x

SOURCES += main.cpp \
    mtest.cpp \
    memorypool.cpp \
    Configuration.cpp \
    stringfile.cpp \
    PluginConfig.cpp

HEADERS += \
    mtest.h \
    memorypool.h \
    Configuration.h \
    stringfile.h \
    PluginConfig.h \
    MT4ServerAPI.h
