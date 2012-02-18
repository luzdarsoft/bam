#-------------------------------------------------
#
# Project created by QtCreator 2011-12-26T20:48:07
#
#-------------------------------------------------

QT       += core gui\
            network

TARGET = bam
TEMPLATE = app


SOURCES += main.cpp \
    mainwindow.cpp \
    bamlistobject.cpp \
    bammodel.cpp \
    quazip/zip.c \
    quazip/unzip.c \
    quazip/quazipnewinfo.cpp \
    quazip/quazipfile.cpp \
    quazip/quazip.cpp \
    quazip/quacrc32.cpp \
    quazip/quaadler32.cpp \
    quazip/JlCompress.cpp \
    quazip/ioapi.c \
    settings.cpp


HEADERS  += mainwindow.h \
    bamlistobject.h \
    bammodel.h \
    quazip/zip.h \
    quazip/unzip.h \
    quazip/quazipnewinfo.h \
    quazip/quazipfileinfo.h \
    quazip/quazipfile.h \
    quazip/quazip.h \
    quazip/quacrc32.h \
    quazip/quachecksum32.h \
    quazip/quaadler32.h \
    quazip/JlCompress.h \
    quazip/ioapi.h \
    quazip/crypt.h \
    settings.h


FORMS    += mainwindow.ui \
    installationProgress.ui \
    settings.ui

RESOURCES += \
    resources/resources.qrc

win32 {
    INCLUDEPATH += C:\\QtSDK\\zlib-1.2.3
    LIBS += -LC:\\QtSDK\\zlib-1.2.3
}

unix {
    LIBS += -lz

    installfiles.files += bam
    installfiles.path = /usr/bin/
    INSTALLS += installfiles
}
