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
        connect(mainWindow, &MainWindow::startGameRequested, []() {
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
    // Удаляем старое окно игры, если оно есть
    if (gameWindow) {
        gameWindow->deleteLater();
        gameWindow = nullptr;
    }

    // Создаем новое окно игры
    gameWindow = new GameWindow();

    // Подключаем сигналы для игры
    connect(&NetworkClient::instance(), &NetworkClient::gameStarted, gameWindow, &GameWindow::onGameStarted);
    connect(&NetworkClient::instance(), &NetworkClient::moveResult, gameWindow, &GameWindow::onMoveResult);
    connect(gameWindow, &GameWindow::backToMainMenu, this, &WindowManager::showMainWindow);

    if (mainWindow) mainWindow->hide();
    gameWindow->show();
}

namespace {
QPoint g_lastWindowPos;
}

QPoint WindowManager::getLastWindowPosition() {
    return g_lastWindowPos;
}

void WindowManager::setLastWindowPosition(const QPoint& pos) {
    g_lastWindowPos = pos;
}

