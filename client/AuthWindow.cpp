/**
 * @file AuthWindow.cpp
 * @brief Реализация класса AuthWindow.
 *
 *  Этот файл содержит реализацию методов класса AuthWindow,
 *  управляющего окном аутентификации.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "AuthWindow.h"
#include "ui_AuthWindow.h"
#include <QMessageBox>

/**
 * @brief Конструктор класса AuthWindow.
 *
 * Инициализирует окно аутентификации, настраивая пользовательский интерфейс.
 *
 * @param parent Указатель на родительский виджет.
 */
AuthWindow::AuthWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
}

/**
 * @brief Деструктор класса AuthWindow.
 *
 * Освобождает память, выделенную для пользовательского интерфейса окна аутентификации.
 */
AuthWindow::~AuthWindow()
{
    delete ui;
}

/**
 * @brief Обработчик нажатия на кнопку "Войти".
 *
 *  Получает имя пользователя и пароль из полей ввода, проверяет их на пустоту
 *  и, если все поля заполнены, генерирует сигнал loginRequested с этими данными.
 */
void AuthWindow::on_loginButton_clicked()
{
    QString nickname = ui->emailEdit->text();
    QString password = ui->passwordEdit->text();

    /**
     * @brief Проверка на заполненность полей ввода.
     *
     * Если какое-либо из полей ввода пустое, отображается предупреждающее сообщение.
     */
    if(nickname.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля");
        return;
    }

    /**
     * @brief Генерация сигнала loginRequested.
     *
     * Этот сигнал передает имя пользователя и пароль для аутентификации.
     */
    emit loginRequested(nickname, password);
}

/**
 * @brief Обработчик нажатия на кнопку "Зарегистрироваться".
 *
 * Генерирует сигнал registerRequested, сигнализирующий о необходимости отобразить
 * окно регистрации.
 */
void AuthWindow::on_registerButton_clicked()
{
    /**
     * @brief Генерация сигнала registerRequested.
     *
     * Этот сигнал используется для запроса отображения окна регистрации.
     */
    emit registerRequested();
}