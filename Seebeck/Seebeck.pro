#-------------------------------------------------
#
# Project created by QtCreator 2011-10-14T14:02:24
#
#-------------------------------------------------

QT       += core gui

TARGET = Seebeck
TEMPLATE = app

LIBS += -L../msdptool/build -lmsdptool \
    -L../QCSVWriter -lQCSVWriter \
    -L../QSCPIDev -lQSCPIDev

SOURCES += main.cpp\
        mainwindow.cpp \
    experiment.cpp \
    config.cpp \
    configui.cpp \
    qeurotherm.cpp

HEADERS  += mainwindow.h \
    experiment.h \
    config.h \
    configui.h \
    qeurotherm.h

FORMS    += mainwindow.ui \
    configui.ui
