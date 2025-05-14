/**
 * @file RegisterWindow.h
 * @brief Объявление класса RegisterWindow.
 *
 *  Этот файл содержит объявление класса RegisterWindow, который представляет
 *  окно регистрации пользователя.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterWindow; }
QT_END_NAMESPACE

/**
 * @class RegisterWindow
 * @brief Класс, представляющий окно регистрации пользователя.
 *
 *  Этот класс отвечает за отображение формы регистрации, сбор данных
 *  от пользователя и отправку запроса на регистрацию.
 */
class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса RegisterWindow.
     *
     *  Инициализирует окно регистрации пользователя.
     *
     * @param parent Указатель на родительский виджет (по умолчанию nullptr).
     */
    explicit RegisterWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса RegisterWindow.
     *
     *  Освобождает ресурсы, выделенные для пользовательского интерфейса.
     */
    ~RegisterWindow();

signals:
    /**
     * @brief Сигнал, генерируемый при запросе на регистрацию.
     *
     *  Этот сигнал генерируется при нажатии на кнопку "Зарегистрироваться"
     *  и содержит данные, введенные пользователем.
     *
     * @param nickname Никнейм пользователя.
     * @param email Email пользователя.
     * @param password Пароль пользователя.
     * @param confirmPassword Подтверждение пароля.
     */
    void registrationRequested(const QString &nickname, const QString &email,
                               const QString &password, const QString &confirmPassword);

    /**
     * @brief Сигнал, генерируемый при запросе на возврат к окну авторизации.
     *
     *  Этот сигнал генерируется при нажатии на кнопку "Назад".
     */
    void backToAuthRequested();

private slots:
    /**
     * @brief Слот для обработки нажатия на кнопку "Зарегистрироваться".
     */
    void on_registerButton_clicked();

    /**
     * @brief Слот для обработки нажатия на кнопку "Назад".
     */
    void on_backButton_clicked();

private:
    /**
     * @brief Указатель на объект пользовательского интерфейса (UI).
     */
    Ui::RegisterWindow *ui;
};

#endif // REGISTERWINDOW_H