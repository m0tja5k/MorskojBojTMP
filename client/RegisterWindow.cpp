/**
 * @file RegisterWindow.cpp
 * @brief Реализация класса RegisterWindow.
 *
 *  Этот файл содержит реализацию класса RegisterWindow, который представляет
 *  окно регистрации пользователя.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#include "RegisterWindow.h"
#include "ui_RegisterWindow.h"
#include <QMessageBox>
#include "NetworkClient.h"

/**
 * @brief Конструктор класса RegisterWindow.
 *
 *  Инициализирует окно регистрации пользователя, настраивает UI и подключается к серверу.
 *
 * @param parent Указатель на родительский виджет (по умолчанию nullptr).
 */
RegisterWindow::RegisterWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWindow)
{
    ui->setupUi(this);
    NetworkClient::instance().connectToServer();
}

/**
 * @brief Деструктор класса RegisterWindow.
 *
 *  Освобождает ресурсы, выделенные для пользовательского интерфейса.
 */
RegisterWindow::~RegisterWindow()
{
    delete ui;
}

/**
 * @brief Слот для обработки нажатия на кнопку "Зарегистрироваться".
 *
 *  Получает данные из полей ввода (никнейм, email, пароль, подтверждение пароля),
 *  проверяет совпадение паролей и генерирует сигнал registrationRequested.
 */
void RegisterWindow::on_registerButton_clicked()
{
    QString nickname = ui->nicknameEdit->text();
    QString email = ui->emailEdit->text();
    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmPasswordEdit->text();

    /**
     * @brief Проверка совпадения паролей.
     *
     *  Если пароли не совпадают, выводится сообщение об ошибке.
     */
    if(password != confirm) {
        QMessageBox::warning(this, "Ошибка", "Пароли не совпадают");
        return;
    }

    emit registrationRequested(nickname, email, password, confirm);
}

/**
 * @brief Слот для обработки нажатия на кнопку "Назад".
 *
 *  Генерирует сигнал backToAuthRequested для возврата к окну авторизации.
 */
void RegisterWindow::on_backButton_clicked()
{
    emit backToAuthRequested();
}