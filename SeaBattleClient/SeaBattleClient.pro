QT += core gui network widgets
CONFIG += c++17
INCLUDEPATH += $$OUT_PWD
TARGET = SeaBattleClient
SOURCES += \
    main.cpp \
    NetworkClient.cpp \
    MainWindow.cpp
HEADERS += \
    NetworkClient.h \
    MainWindow.h
FORMS += \
    MainWindow.ui
