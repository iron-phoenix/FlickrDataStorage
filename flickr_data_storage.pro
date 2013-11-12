#-------------------------------------------------
#
# Project created by QtCreator 2013-11-11T11:26:31
#
#-------------------------------------------------

QT       += core gui webkit network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = flickr_data_storage
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    flickrapi.cpp \
    JPEGConverter.cpp \
    flickrfileview.cpp

HEADERS  += mainwindow.h \
    flickrapi.h \
    BMPConverter.h \
    JPEGConverter.h \
    flickrfileview.h

#CONFIG += release

RESOURCES += \
    resources.qrc
