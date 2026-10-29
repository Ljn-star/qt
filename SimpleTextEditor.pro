QT += core gui widgets

CONFIG += c++17

TEMPLATE = app
TARGET   = SimpleTextEditor

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    finddialog.cpp \
    replacedialog.cpp \
    aboutdialog.cpp

HEADERS += \
    mainwindow.h \
    codeeditor.h \
    finddialog.h \
    replacedialog.h \
    aboutdialog.h

FORMS += \
    mainwindow.ui \
    finddialog.ui \
    replacedialog.ui \
    aboutdialog.ui

RESOURCES += resources.qrc
