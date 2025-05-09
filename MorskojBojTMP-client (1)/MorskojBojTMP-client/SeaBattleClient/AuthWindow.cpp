#include "AuthWindow.h"
#include "ui_AuthWindow.h"
#include <QMessageBox>

AuthWindow::AuthWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::on_loginButton_clicked()
{
    QString email = ui->emailEdit->text();
    QString password = ui->passwordEdit->text();

    if(email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }

    emit loginRequested(email, password);
}

void AuthWindow::on_registerButton_clicked()
{
    emit registerRequested();
}
