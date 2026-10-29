QT += core gui widgets testlib
CONFIG += console c++17 testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET   = tst_editor

INCLUDEPATH += ..

FORMS += \
    ../mainwindow.ui \
    ../finddialog.ui \
    ../replacedialog.ui \
    ../aboutdialog.ui

SOURCES += \
    tst_editor.cpp \
    ../mainwindow.cpp \
    ../codeeditor.cpp \
    ../finddialog.cpp \
    ../replacedialog.cpp \
    ../aboutdialog.cpp

HEADERS += \
    ../mainwindow.h \
    ../codeeditor.h \
    ../finddialog.h \
    ../replacedialog.h \
    ../aboutdialog.h

RESOURCES += ../resources.qrc
