QT += testlib sql network
QT -= gui
CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
TEMPLATE = app
DEFINES += TEST_MODE
SOURCES += tst_server_test.cpp \
    server/func2serv.cpp \
    server/DatabaseManager.cpp \
    server/mytcpserver.cpp
HEADERS += \
    server/func2serv.h \
    server/DatabaseManager.h \
    server/mytcpserver.h
