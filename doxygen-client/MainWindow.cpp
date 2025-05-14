/**
 * @file MainWindow.cpp
 * @brief Реализация класса MainWindow.
 *
 *  Этот файл содержит реализацию класса MainWindow, который представляет главное
 *  окно приложения. Он отвечает за управление UI, подключение к серверу,
 *  отправку сообщений и отображение информации.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NetworkClient.h"
#include <QPushButton>
#include <QMessageBox>

/**
 * @brief Конструктор класса MainWindow.
 *
 * Инициализирует главное окно приложения, настраивает UI, подключает сигналы и слоты.
 *
 * @param parent Указатель на родительский виджет (по умолчанию nullptr).
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /**
     * @brief Настройка кнопки "Начать игру".
     *
     * Кнопка "Начать игру" изначально активируется и устанавливается текст "Начать игру".
     */
    ui->startButton->setEnabled(true);
    ui->startButton->setText("Начать игру");

    /**
     * @brief Подключение сигналов и слотов для управления UI.
     */
    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::connectButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::sendButtonClicked);
    connect(ui->startButton, &QPushButton::clicked,
            this, &MainWindow::on_startButton_clicked);

    /**
     * @brief Подключение сигналов NetworkClient к слотам MainWindow.
     */
    connect(&NetworkClient::instance(), &NetworkClient::messageReceived,
            this, &MainWindow::handleMessage);
    connect(&NetworkClient::instance(), &NetworkClient::connectionChanged,
            this, &MainWindow::handleConnectionChanged);
    connect(&NetworkClient::instance(), &NetworkClient::errorOccurred,
            this, &MainWindow::handleError);
    connect(&NetworkClient::instance(), &NetworkClient::gameReady,
            this, [this]() { ui->startButton->setEnabled(true); });

    updateUI();
}

/**
 * @brief Устанавливает информацию о пользователе (никнейм).
 *
 * Устанавливает никнейм текущего пользователя в NetworkClient и отображает его в UI.
 *
 * @param nickname Никнейм пользователя.
 */
void MainWindow::setUserInfo(const QString &nickname)
{
    NetworkClient::instance().setCurrentNickname(nickname);
    ui->nicknameLabel->setText("Игрок: " + nickname);
}

/**
 * @brief Слот для обработки нажатия на кнопку "Начать игру".
 *
 * Генерирует сигнал startGameRequested, уведомляющий о запросе на начало игры.
 */
void MainWindow::on_startButton_clicked()
{
    emit startGameRequested();
}

/**
 * @brief Деструктор класса MainWindow.
 *
 * Освобождает ресурсы, выделенные для пользовательского интерфейса.
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Слот для обработки нажатия на кнопку "Подключиться/Отключиться".
 *
 *  Если клиент подключен к серверу, отключается от него. В противном случае
 *  подключается к серверу. После подключения/отключения обновляет UI.
 */
void MainWindow::connectButtonClicked()
{
    if (NetworkClient::instance().isConnected()) {
        NetworkClient::instance().disconnectFromServer();
    } else {
        NetworkClient::instance().connectToServer();
    }
    updateUI();
}

/**
 * @brief Слот для обработки нажатия на кнопку "Отправить".
 *
 *  Отправляет сообщение, введенное в поле messageEdit, на сервер.
 *  Очищает поле messageEdit после отправки сообщения.
 */
void MainWindow::sendButtonClicked()
{
    const QString message = ui->messageEdit->text();
    if (!message.isEmpty()) {
        NetworkClient::instance().sendMessage(message);
        ui->messageEdit->clear();
    }
}

/**
 * @brief Слот для обработки полученного сообщения от сервера.
 *
 *  Отображает полученное сообщение в поле chatLog.
 *
 * @param message Сообщение, полученное от сервера.
 */
void MainWindow::handleMessage(const QString& message)
{
    ui->chatLog->appendPlainText("Server: " + message);
}

/**
 * @brief Слот для обработки изменения состояния подключения.
 *
 *  Обновляет текст на statusLabel и connectButton в зависимости от состояния подключения.
 *
 * @param connected True, если клиент подключен к серверу, False - если нет.
 */
void MainWindow::handleConnectionChanged(bool connected)
{
    ui->statusLabel->setText(connected ? "Connected" : "Disconnected");
    ui->connectButton->setText(connected ? "Disconnect" : "Connect");
    ui->sendButton->setEnabled(connected);
}

/**
 * @brief Слот для обработки ошибок, возникающих в NetworkClient.
 *
 *  Отображает сообщение об ошибке в поле chatLog.
 *
 * @param error Сообщение об ошибке.
 */
void MainWindow::handleError(const QString& error)
{
    ui->chatLog->appendPlainText("Error: " + error);
}

/**
 * @brief Обновляет UI в зависимости от состояния подключения.
 *
 *  Выполняет обновление UI, в зависимости от того, подключен ли клиент к серверу.
 */
void MainWindow::updateUI()
{
    // Можно добавить любую логику обновления UI здесь
}

/**
 * @brief Обновляет UI в зависимости от состояния подключения.
 *
 *  Выполняет обновление UI, в зависимости от того, подключен ли клиент к серверу.
 *  Включает или отключает кнопку отправки, устанавливает текст на кнопке
 *  подключения и обновляет текст на statusLabel.
 */
void MainWindow::updateUI()
{
    /**
     * @brief Проверка состояния подключения.
     *
     *  Получает текущее состояние подключения от NetworkClient.
     */
    const bool connected = NetworkClient::instance().isConnected();

    /**
     * @brief Обновление состояния кнопки отправки.
     *
     *  Включает кнопку отправки, если клиент подключен, и отключает в противном случае.
     */
    ui->sendButton->setEnabled(connected);

    /**
     * @brief Обновление текста на кнопке подключения.
     *
     *  Устанавливает текст на кнопке подключения в зависимости от состояния подключения.
     */
    ui->connectButton->setText(connected ? "Disconnect" : "Connect");

    /**
     * @brief Обновление текста на statusLabel.
     *
     *  Устанавливает текст на statusLabel в зависимости от состояния подключения.
     */
    ui->statusLabel->setText(connected ? "Connected" : "Disconnected");
}