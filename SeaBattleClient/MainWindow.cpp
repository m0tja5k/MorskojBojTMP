#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NetworkClient.h"
#include <QPushButton>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Изначально кнопка старта неактивна
    ui->startButton->setEnabled(false);

    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::connectButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::sendButtonClicked);
    connect(ui->startButton, &QPushButton::clicked,
            this, &MainWindow::on_startButton_clicked);

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

void MainWindow::setUserInfo(const QString &nickname)
{
    currentNickname = nickname;
    ui->nicknameLabel->setText("Игрок: " + nickname);
}

void MainWindow::on_startButton_clicked()
{
    emit startGameRequested();
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
