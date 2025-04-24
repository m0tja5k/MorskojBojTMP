//MainWindow.cpp
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NetworkClient.h"
#include <QPushButton>

// Строка 8: Конструктор - исправлено выравнивание
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow) // Убедитесь, что Ui::MainWindow объявлен в ui_MainWindow.h
{
    // Строка 10: Инициализация UI
    ui->setupUi(this);

    // Строки 13-15: Подключение кнопок
    connect(ui->connectButton, &QPushButton::clicked, // Проверьте objectName кнопки в UI
            this, &MainWindow::connectButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::sendButtonClicked);

    // Подключение сигналов NetworkClient
    connect(&NetworkClient::instance(), &NetworkClient::messageReceived,
            this, &MainWindow::handleMessage);
    connect(&NetworkClient::instance(), &NetworkClient::connectionChanged, // Исправлено имя сигнала (было connectionChanged)
            this, &MainWindow::handleConnectionChanged);
    connect(&NetworkClient::instance(), &NetworkClient::errorOccurred,
            this, &MainWindow::handleError);

    updateUI();
}

// Строка 46: Деструктор
MainWindow::~MainWindow()
{
    delete ui; // Корректно, если ui инициализирован через new
}

// Строка 49: Слот кнопки подключения
void MainWindow::connectButtonClicked()
{
    if (NetworkClient::instance().isConnected()) {
        NetworkClient::instance().disconnectFromServer();
    } else {
        NetworkClient::instance().connectToServer(); // Проверьте реализацию connectToServer()
    }
    updateUI();
}

// Строка 55: Слот кнопки отправки
void MainWindow::sendButtonClicked()
{
    const QString message = ui->messageEdit->text(); // Проверьте objectName messageEdit в UI
    if (!message.isEmpty()) {
        NetworkClient::instance().sendMessage(message);
        ui->messageEdit->clear();
    }
}

// Строки 60-62: Обработка сообщений
void MainWindow::handleMessage(const QString& message)
{
    ui->chatLog->appendPlainText("Server: " + message); // Проверьте objectName chatLog
}

// Строки 67: Обработка изменения подключения
void MainWindow::handleConnectionChanged(bool connected)
{
    ui->statusLabel->setText(connected ? "Connected" : "Disconnected"); // Проверьте statusLabel
    ui->connectButton->setText(connected ? "Disconnect" : "Connect");
    ui->sendButton->setEnabled(connected);
}

// Строка 73: Обработка ошибок
void MainWindow::handleError(const QString& error)
{
    ui->chatLog->appendPlainText("Error: " + error);
}

// Строки 74-75: Обновление UI
void MainWindow::updateUI()
{
    const bool connected = NetworkClient::instance().isConnected();
    ui->sendButton->setEnabled(connected);
    ui->connectButton->setText(connected ? "Disconnect" : "Connect");
    ui->statusLabel->setText(connected ? "Connected" : "Disconnected");
}
