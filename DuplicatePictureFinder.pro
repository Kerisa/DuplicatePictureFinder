#-------------------------------------------------
#
# Project created by QtCreator 2018-01-21T14:05:31
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DuplicatePictureFinder
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
         UNICODE
         _UNICODE

win32:CONFIG(release, debug|release): DEFINES += NDEBUG
else:win32:CONFIG(debug, debug|release): DEFINES += _DEBUG



# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    $$PWD/src/customtreewidget.cpp \
    $$PWD/src/main.cpp \
    $$PWD/src/mainwindow.cpp \
    $$PWD/src/crc.cpp \
    $$PWD/src/featurevector.cpp \
    $$PWD/src/OpenFiles.cpp \
    $$PWD/src/picture.cpp \
    $$PWD/src/customgraphicsview.cpp \
    $$PWD/src/movetorecyclebin.cpp \
    $$PWD/src/qpicthread.cpp \
    $$PWD/src/optionsdialog.cpp

HEADERS += \
    $$PWD/src/mainwindow.h \
    $$PWD/src/crc.h \
    $$PWD/src/featurevector.h \
    $$PWD/src/OpenFiles.h \
    $$PWD/src/picture.h \
    $$PWD/src/types.h \
    $$PWD/src/Utility.h \
    $$PWD/src/customtreewidget.h \
    $$PWD/src/customgraphicsview.h \
    $$PWD/src/movetorecyclebin.h \
    $$PWD/src/qpicthread.h \
    $$PWD/src/optionsdialog.h

FORMS += \
    $$PWD/src/mainwindow.ui \
    $$PWD/src/options.ui

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
    $$PWD/src/resource.qrc

RC_FILE += \
    $$PWD/src/resource.rc
