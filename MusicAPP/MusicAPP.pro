#-------------------------------------------------
#
# Project created by QtCreator 2024-12-01T11:19:06
#
#-------------------------------------------------

QT       += core gui webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MusicAPP
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    about.cpp \
    debug.cpp \
    helpdialog.cpp \
    qtexteditstream.cpp \
    libs/tiny/tinyxml2.cpp \
    libs/miniz/miniz.c


HEADERS += \
        mainwindow.h \
    about.h \
    debug.h \
    helpdialog.h \
    qtexteditstream.h \
    libs/tiny/tinyxml2.h \
    libs/miniz/miniz.h

FORMS += \
        mainwindow.ui \
    about.ui \
    debug.ui \
    helpdialog.ui

guide.source = $$PWD/guide

CONFIG(debug, debug|release) {
    guide.target = $$OUT_PWD/debug/guide
    message(OUT_PWD/debug/guide = $$OUT_PWD/debug/guide)

} else {
    guide.target = $$OUT_PWD/release/guide
}

win32 {
    COPY_DIR = xcopy /E /I /Y
    QMAKE_POST_LINK += $$COPY_DIR \"$$guide.source\" \"$$guide.target\" >NUL
}

WEBENGINE_CONFIG += use_proprietary_codecs


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    files.qrc

#INCLUDEPATH += $$PWD/libs/miniz
#QMAKE_CXXFLAGS += miniz.c

QMAKE_CXXFLAGS_MINIZ = -w  # Игнорировать все предупреждения для miniz.c
QMAKE_CXXFLAGS += $$QMAKE_CXXFLAGS_MINIZ
INCLUDEPATH += "C:/Program Files/OpenSSL-Win64/bin"
QMAKE_CXXFLAGS += /utf-8
