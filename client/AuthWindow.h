/**
 * @file AuthWindow.h
 * @brief Объявление класса AuthWindow.
 *
 *  Этот файл содержит объявление класса AuthWindow, который представляет
 *  виджет окна аутентификации, позволяющего пользователю войти в систему.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class AuthWindow; }
QT_END_NAMESPACE

/**
 * @class AuthWindow
 * @brief Класс, представляющий окно аутентификации.
 *
 *  Этот класс предоставляет интерфейс для пользователя для ввода имени пользователя
 *  (email) и пароля, а также для перехода к окну регистрации.
 */
class AuthWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса AuthWindow.
     *
     *  Создает окно аутентификации и настраивает его пользовательский интерфейс.
     *
     * @param parent Указатель на родительский виджет (по умолчанию nullptr).
     */
    explicit AuthWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса AuthWindow.
     *
     * Освобождает ресурсы, выделенные для пользовательского интерфейса.
     */
    ~AuthWindow();

signals:
    /**
     * @brief Сигнал, генерируемый при запросе входа в систему.
     *
     *  Этот сигнал генерируется при нажатии кнопки "Войти" и содержит
     *  имя пользователя (email) и пароль, введенные пользователем.
     *
     * @param email Имя пользователя (email).
     * @param password Пароль.
     */
    void loginRequested(const QString &email, const QString &password);

    /**
     * @brief Сигнал, генерируемый при запросе регистрации.
     *
     *  Этот сигнал генерируется при нажатии кнопки "Зарегистрироваться"
     *  и сигнализирует о необходимости отобразить окно регистрации.
     */
    void registerRequested();

private slots:
    /**
     * @brief Слот, вызываемый при нажатии на кнопку "Войти".
     */
    void on_loginButton_clicked();

    /**
     * @brief Слот, вызываемый при нажатии на кнопку "Зарегистрироваться".
     */
    void on_registerButton_clicked();

private:
    /**
     * @brief Указатель на объект пользовательского интерфейса (UI).
     */
    Ui::AuthWindow *ui;
};

#endif // AUTHWINDOW_H