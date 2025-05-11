QT += core gui network widgets
CONFIG += c++17
INCLUDEPATH += $$OUT_PWD
TARGET = SeaBattleClient

SOURCES += \
    AuthWindow.cpp \
    GameWindow.cpp \
    RegisterWindow.cpp \
    WindowManager.cpp \
    main.cpp \
    NetworkClient.cpp \
    MainWindow.cpp

HEADERS += \
    AuthWindow.h \
    GameWindow.h \
    NetworkClient.h \
    MainWindow.h \
    AuthWindow.h \
    RegisterWindow.h \
    WindowManager.h

FORMS += \
    AuthWindow.ui \
    GameWindow.ui \
    MainWindow.ui \
    AuthWindow.ui \
    RegisterWindow.ui
