/**
 * @file WindowManager.h
 * @brief Объявление класса WindowManager.
 *
 *  Этот файл содержит объявление класса WindowManager, который отвечает
 *  за управление окнами приложения, включая окна аутентификации, регистрации,
 *  главное окно и игровое окно.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include "AuthWindow.h"
#include "RegisterWindow.h"
#include "MainWindow.h"
#include "GameWindow.h"

/**
 * @class WindowManager
 * @brief Класс для управления окнами приложения.
 *
 *  WindowManager создает, отображает и управляет переключением между
 *  окнами аутентификации, регистрации, главным окном и игровым окном.
 */
class WindowManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса WindowManager.
     *
     *  Инициализирует члены класса, создавая экземпляры окон
     *  аутентификации и регистрации. Главное и игровое окна
     *  инициализируются как nullptr.
     *
     * @param parent Указатель на родительский объект (по умолчанию nullptr).
     */
    explicit WindowManager(QObject *parent = nullptr);

    /**
     * @brief Деструктор класса WindowManager.
     *
     * Освобождает память, выделенную для окон приложения.
     */
    ~WindowManager();

    /**
     * @brief Отображает окно аутентификации.
     */
    void showAuthWindow();

    /**
     * @brief Отображает окно регистрации.
     */
    void showRegisterWindow();

    /**
     * @brief Отображает главное окно приложения.
     *
     * @param nickname Имя пользователя, которое будет отображено в главном окне.
     */
    void showMainWindow(const QString &nickname);

    /**
     * @brief Отображает игровое окно.
     */
    void showGameWindow();

private:
    /**
     * @brief Указатель на окно аутентификации.
     */
    AuthWindow *authWindow;

    /**
     * @brief Указатель на окно регистрации.
     */
    RegisterWindow *registerWindow;

    /**
     * @brief Указатель на главное окно приложения.
     */
    MainWindow *mainWindow;

    /**
     * @brief Указатель на игровое окно.
     */
    GameWindow *gameWindow;
};

#endif // WINDOWMANAGER_H