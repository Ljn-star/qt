QT += core sql testlib

CONFIG += c++17 console testcase
CONFIG -= app_bundle

TEMPLATE = app
TARGET = HospitalTests

SOURCES += \
    tst_hospital.cpp \
    ../DatabaseManager.cpp \
    ../PatientTableModel.cpp

HEADERS += \
    ../DatabaseManager.h \
    ../PatientData.h \
    ../PatientTableModel.h

INCLUDEPATH += ..
QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8

