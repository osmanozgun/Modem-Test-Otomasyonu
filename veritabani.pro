QT += core gui widgets serialport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console
CONFIG -= app_bundle

TEMPLATE = app
TARGET = veritabani

SOURCES += \
    main.cpp \
    Veri_Ekle.cpp \
    Kayit_Listele.cpp \
    Testler.cpp \
    Programlama.cpp

HEADERS += \
    Veri_Ekle.h \
    Kayit_Listele.h \
    Testler.h \
    Programlama.h
