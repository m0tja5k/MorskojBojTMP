#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include "AuthWindow.h"
#include "RegisterWindow.h"
#include "MainWindow.h"
#include "GameWindow.h"

class WindowManager : public QObject
{
    Q_OBJECT

public:
    static QPoint getLastWindowPosition();
    static void setLastWindowPosition(const QPoint& pos);
    explicit WindowManager(QObject *parent = nullptr);
    ~WindowManager();
    void showAuthWindow();
    void showRegisterWindow();
    void showMainWindow();
    void showGameWindow();

private:
    AuthWindow *authWindow;
    RegisterWindow *registerWindow;
    MainWindow *mainWindow;
    GameWindow *gameWindow;
};

#endif // WINDOWMANAGER_H
