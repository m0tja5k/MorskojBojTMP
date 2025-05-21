#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "NetworkClient.h"
#include <QPushButton>
#include <QMessageBox>
#include "WindowManager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QPoint lastPos = WindowManager::getLastWindowPosition();
    if (!lastPos.isNull()) {
        this->move(lastPos);
    }

    ui->startButton->setEnabled(true);
    ui->startButton->setText("Начать игру");

    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::connectButtonClicked);
    connect(ui->startButton, &QPushButton::clicked,
            this, &MainWindow::on_startButton_clicked);

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
    NetworkClient::instance().setCurrentNickname(nickname);
    ui->nicknameLabel->setText("Игрок: " + nickname);
}

void MainWindow::on_startButton_clicked()
{
    WindowManager::setLastWindowPosition(this->pos());
    emit startGameRequested();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectButtonClicked()
{
    if (NetworkClient::instance().isConnected()) {
        NetworkClient::instance().disconnectFromServer();
    } else {
        NetworkClient::instance().connectToServer();
    }
    updateUI();
}

void MainWindow::handleConnectionChanged(bool connected)
{
    ui->statusLabel->setText(connected ? "Connected" : "Disconnected");
    ui->connectButton->setText(connected ? "Disconnect" : "Connect");
}

void MainWindow::handleError(const QString& error)
{
    QMessageBox::critical(this, "Ошибка", error);
}

void MainWindow::updateUI()
{
    const bool connected = NetworkClient::instance().isConnected();
    ui->connectButton->setText(connected ? "Disconnect" : "Connect");
    ui->statusLabel->setText(connected ? "Connected" : "Disconnected");
}
