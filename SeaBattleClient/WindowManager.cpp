#include "WindowManager.h"
#include "NetworkClient.h"

WindowManager::WindowManager(QObject *parent) : QObject(parent),
    authWindow(new AuthWindow()),
    registerWindow(new RegisterWindow()),
    mainWindow(nullptr),
    gameWindow(nullptr)
{
    connect(authWindow, &AuthWindow::loginRequested, [this](const QString &email, const QString &password) {
        NetworkClient::instance().loginUser(email, password);
    });

    connect(authWindow, &AuthWindow::registerRequested, this, &WindowManager::showRegisterWindow);

    connect(registerWindow, &RegisterWindow::registrationRequested,
            [this](const QString &nick, const QString &email, const QString &pass, const QString &confirm) {
                NetworkClient::instance().registerUser(nick, email, pass);
            });

    connect(registerWindow, &RegisterWindow::backToAuthRequested, this, &WindowManager::showAuthWindow);

    connect(&NetworkClient::instance(), &NetworkClient::registrationSuccess, this, &WindowManager::showAuthWindow);
    connect(&NetworkClient::instance(), &NetworkClient::loginSuccess, this, &WindowManager::showMainWindow);
    connect(&NetworkClient::instance(), &NetworkClient::gameReady, this, &WindowManager::showGameWindow);
}

void WindowManager::showAuthWindow()
{
    if (mainWindow) mainWindow->hide();
    if (gameWindow) gameWindow->hide();
    if (registerWindow) registerWindow->hide();

    authWindow->show();
}

void WindowManager::showRegisterWindow()
{
    authWindow->hide();
    registerWindow->show();
}

void WindowManager::showMainWindow(const QString &nickname)
{
    if (!mainWindow) {
        mainWindow = new MainWindow();
        connect(mainWindow, &MainWindow::startGameRequested, &NetworkClient::instance(), &NetworkClient::requestStartGame);
    }

    authWindow->hide();
    registerWindow->hide();
    mainWindow->setUserInfo(nickname);
    mainWindow->show();
}

void WindowManager::showGameWindow()
{
    if (!gameWindow) {
        gameWindow = new GameWindow();
    }

    if (mainWindow) mainWindow->hide();
    gameWindow->show();
}
