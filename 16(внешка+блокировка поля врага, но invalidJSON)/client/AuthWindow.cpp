#include "AuthWindow.h"
#include "ui_AuthWindow.h"
#include "WindowManager.h"
#include <QMessageBox>

AuthWindow::AuthWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AuthWindow)
{
    ui->setupUi(this);

    QPoint lastPos = WindowManager::getLastWindowPosition();
    if (!lastPos.isNull()) {
        this->move(lastPos);
    }
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::on_loginButton_clicked()
{
    QString nickname = ui->nicknameEdit->text();
    QString password = ui->passwordEdit->text();

    if(nickname.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }

    WindowManager::setLastWindowPosition(this->pos());

    emit loginRequested(nickname, password);
}

void AuthWindow::on_registerButton_clicked()
{
    WindowManager::setLastWindowPosition(this->pos());

    emit registerRequested();
}
