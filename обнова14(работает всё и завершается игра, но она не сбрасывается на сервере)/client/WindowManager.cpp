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
    connect(gameWindow, &GameWindow::backToMainMenu, this, &WindowManager::showMainWindow);
}

WindowManager::~WindowManager() {
    delete authWindow;
    delete registerWindow;
    delete mainWindow;
    delete gameWindow;
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

void WindowManager::showMainWindow()
{
    QString nickname = NetworkClient::instance().getCurrentNickname();
    if (!mainWindow) {
        mainWindow = new MainWindow();
        connect(mainWindow, &MainWindow::startGameRequested, [this]() {
            NetworkClient::instance().requestStartGame();
        });
    }

    authWindow->hide();
    registerWindow->hide();
    if (gameWindow) gameWindow->hide();
    mainWindow->setUserInfo(nickname);
    mainWindow->show();
}

void WindowManager::showGameWindow()
{
    if (!gameWindow) {
        gameWindow = new GameWindow();
        // Подключаем сигналы для игры
        connect(&NetworkClient::instance(), &NetworkClient::gameStarted, gameWindow, &GameWindow::onGameStarted);
        connect(&NetworkClient::instance(), &NetworkClient::moveResult, gameWindow, &GameWindow::onMoveResult);
    }

    if (mainWindow) mainWindow->hide();
    gameWindow->show();
}
