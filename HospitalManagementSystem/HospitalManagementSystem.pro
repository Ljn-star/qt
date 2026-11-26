QT += core gui widgets sql

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = HospitalManagementSystem

SOURCES += \
    DatabaseManager.cpp \
    MainWindow.cpp \
    PatientEditorDialog.cpp \
    PatientTableModel.cpp \
    SexDelegate.cpp \
    main.cpp

HEADERS += \
    DatabaseManager.h \
    MainWindow.h \
    PatientData.h \
    PatientEditorDialog.h \
    PatientTableModel.h \
    SexDelegate.h

QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8

