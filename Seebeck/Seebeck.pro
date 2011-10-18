#-------------------------------------------------
#
# Project created by QtCreator 2011-10-14T14:02:24
#
#-------------------------------------------------

QT       += core gui

TARGET = Seebeck
TEMPLATE = app

LIBS +=  \
    -L../msdptool/build -lmsdptool \
    -L../QCSVWriter -lQCSVWriter \
    -L../QSCPIDev -lQSCPIDev \
    -lm -lmodbus

SOURCES += main.cpp\
        mainwindow.cpp \
    experiment.cpp \
    config.cpp \
    configui.cpp \
    qmodbus.cpp

HEADERS  += mainwindow.h \
    experiment.h \
    config.h \
    configui.h \
    qmodbus.h

FORMS    += mainwindow.ui \
    configui.ui

OTHER_FILES += \
    ../README \
    ../INSTALL.txt
