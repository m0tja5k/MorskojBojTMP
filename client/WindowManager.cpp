/**
 * @file WindowManager.cpp
 * @brief Реализация класса WindowManager.
 *
 *  Этот файл содержит реализацию методов класса WindowManager,
 *  отвечающего за управление окнами приложения.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "WindowManager.h"
#include "NetworkClient.h"

/**
 * @brief Конструктор класса WindowManager.
 *
 * Инициализирует окна аутентификации, регистрации, главное и игровое окна,
 * а также устанавливает соединения между сигналами и слотами для обработки событий.
 *
 * @param parent Указатель на родительский объект.
 */
WindowManager::WindowManager(QObject *parent) : QObject(parent),
    authWindow(new AuthWindow()),
    registerWindow(new RegisterWindow()),
    mainWindow(nullptr),
    gameWindow(nullptr)
{
    /**
     * @brief Соединение сигнала loginRequested окна аутентификации со слотом loginUser класса NetworkClient.
     *
     *  При запросе входа в систему (сигнал `loginRequested`), отправляет email и пароль
     *  в класс `NetworkClient` для аутентификации пользователя.
     */
    connect(authWindow, &AuthWindow::loginRequested, [this](const QString &email, const QString &password) {
        NetworkClient::instance().loginUser(email, password);
    });

    /**
     * @brief Соединение сигнала registerRequested окна аутентификации со слотом showRegisterWindow класса WindowManager.
     *
     *  При запросе регистрации (сигнал `registerRequested`) отображает окно регистрации.
     */
    connect(authWindow, &AuthWindow::registerRequested, this, &WindowManager::showRegisterWindow);

    /**
     * @brief Соединение сигнала registrationRequested окна регистрации со слотом registerUser класса NetworkClient.
     *
     *  При запросе регистрации (сигнал `registrationRequested`) отправляет ник, email, пароль и подтверждение
     *  в класс `NetworkClient` для регистрации нового пользователя.
     */
    connect(registerWindow, &RegisterWindow::registrationRequested,
            [this](const QString &nick, const QString &email, const QString &pass, const QString &confirm) {
                NetworkClient::instance().registerUser(nick, email, pass);
            });

    /**
     * @brief Соединение сигнала backToAuthRequested окна регистрации со слотом showAuthWindow класса WindowManager.
     *
     *  При запросе возврата к окну аутентификации (сигнал `backToAuthRequested`) отображает окно аутентификации.
     */
    connect(registerWindow, &RegisterWindow::backToAuthRequested, this, &WindowManager::showAuthWindow);

    /**
     * @brief Соединение сигнала registrationSuccess класса NetworkClient со слотом showAuthWindow класса WindowManager.
     *
     *  При успешной регистрации (сигнал `registrationSuccess`) отображает окно аутентификации.
     */
    connect(&NetworkClient::instance(), &NetworkClient::registrationSuccess, this, &WindowManager::showAuthWindow);

    /**
     * @brief Соединение сигнала loginSuccess класса NetworkClient со слотом showMainWindow класса WindowManager.
     *
     *  При успешном входе в систему (сигнал `loginSuccess`) отображает главное окно приложения.
     */
    connect(&NetworkClient::instance(), &NetworkClient::loginSuccess, this, &WindowManager::showMainWindow);

    /**
     * @brief Соединение сигнала gameReady класса NetworkClient со слотом showGameWindow класса WindowManager.
     *
     *  При готовности к игре (сигнал `gameReady`) отображает игровое окно.
     */
    connect(&NetworkClient::instance(), &NetworkClient::gameReady, this, &WindowManager::showGameWindow);
}

/**
 * @brief Деструктор класса WindowManager.
 *
 * Освобождает память, выделенную для окон приложения.
 */
WindowManager::~WindowManager() {
    delete authWindow;
    delete registerWindow;
    delete mainWindow;
    delete gameWindow;
}

/**
 * @brief Отображает окно аутентификации.
 *
 *  Скрывает главное и игровое окна (если они отображены) и отображает окно аутентификации.
 */
void WindowManager::showAuthWindow()
{
    if (mainWindow) mainWindow->hide();
    if (gameWindow) gameWindow->hide();
    if (registerWindow) registerWindow->hide();

    authWindow->show();
}

/**
 * @brief Отображает окно регистрации.
 *
 * Скрывает окно аутентификации и отображает окно регистрации.
 */
void WindowManager::showRegisterWindow()
{
    authWindow->hide();
    registerWindow->show();
}

/**
 * @brief Отображает главное окно приложения.
 *
 * Создает главное окно (если оно еще не создано), скрывает окна аутентификации и регистрации
 * и отображает главное окно.
 *
 * @param nickname Имя пользователя, полученное от сервера.
 */
void WindowManager::showMainWindow(const QString &nickname)
{
    if (!mainWindow) {
        mainWindow = new MainWindow();
        /**
         * @brief Соединение сигнала startGameRequested главного окна со слотом requestStartGame класса NetworkClient.
         *
         *  При запросе начала игры (сигнал `startGameRequested`) отправляет запрос на сервер
         *  через класс `NetworkClient`.
         */
        connect(mainWindow, &MainWindow::startGameRequested, [this]() {
            NetworkClient::instance().requestStartGame();
        });
    }

    authWindow->hide();
    registerWindow->hide();
    mainWindow->setUserInfo(nickname);
    mainWindow->show();
}

/**
 * @brief Отображает игровое окно.
 *
 * Создает игровое окно (если оно еще не создано), скрывает главное окно (если оно отображено)
 * и отображает игровое окно.
 */
void WindowManager::showGameWindow()
{
    if (!gameWindow) {
        gameWindow = new GameWindow();
        /**
         * @brief Соединение сигнала gameStarted класса NetworkClient со слотом onGameStarted класса GameWindow.
         *
         *  При начале игры (сигнал `gameStarted`) передает информацию в игровое окно.
         */
        connect(&NetworkClient::instance(), &NetworkClient::gameStarted, gameWindow, &GameWindow::onGameStarted);

        /**
         * @brief Соединение сигнала moveResult класса NetworkClient со слотом onMoveResult класса GameWindow.
         *
         *  При получении результата хода (сигнал `moveResult`) передает информацию в игровое окно.
         */
        connect(&NetworkClient::instance(), &NetworkClient::moveResult, gameWindow, &GameWindow::onMoveResult);
    }

    if (mainWindow) mainWindow->hide();
    gameWindow->show();
}