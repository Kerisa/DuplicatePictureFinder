#-------------------------------------------------
#
# Project created by QtCreator 2018-01-21T14:05:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_windows
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
         UNICODE
         _UNICODE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    customtreewidget.cpp \
        main.cpp \
        mainwindow.cpp \
    crc.cpp \
    featurevector.cpp \
    OpenFiles.cpp \
    picture.cpp \
    customgraphicsview.cpp \
    movetorecyclebin.cpp \
    qpicthread.cpp

HEADERS += \
        mainwindow.h \
    crc.h \
    featurevector.h \
    OpenFiles.h \
    picture.h \
    types.h \
    Utility.h \
    customtreewidget.h \
    customgraphicsview.h \
    movetorecyclebin.h \
    qpicthread.h

FORMS += \
        mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -llibpng16
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -llibpng16_d

INCLUDEPATH += $$PWD/include/libpng
DEPENDPATH += $$PWD/include/libpng

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -ljpeg
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -ljpeg_d

INCLUDEPATH += $$PWD/include/libjpeg
DEPENDPATH += $$PWD/include/libjpeg

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/libjpeg.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/libjpeg_d.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/jpeg.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/jpeg_d.lib

RESOURCES += \
    resource.qrc
