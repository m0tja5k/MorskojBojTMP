#include "RegisterWindow.h"
#include "ui_RegisterWindow.h"
#include <QMessageBox>
#include "NetworkClient.h"
#include "WindowManager.h"

RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
    NetworkClient::instance().connectToServer();

    QPoint lastPos = WindowManager::getLastWindowPosition();
    if (!lastPos.isNull()) {
        this->move(lastPos);
    }
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

void RegisterWindow::on_registerButton_clicked()
{
    QString nickname = ui->nicknameEdit->text();
    QString email = ui->emailEdit->text();
    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmPasswordEdit->text();


    if(password != confirm) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают");
        return;
    }
    WindowManager::setLastWindowPosition(this->pos());

    emit registrationRequested(nickname, email, password, confirm);
}

void RegisterWindow::on_backButton_clicked()
{
    WindowManager::setLastWindowPosition(this->pos());

    emit backToAuthRequested();
}
