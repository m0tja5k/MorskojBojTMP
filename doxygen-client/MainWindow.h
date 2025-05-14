/**
 * @file MainWindow.h
 * @brief Объявление класса MainWindow.
 *
 *  Этот файл содержит объявление класса MainWindow, который представляет
 *  главное окно приложения. Он содержит методы для управления UI, подключения
 *  к серверу, отправки сообщений и отображения информации.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Класс, представляющий главное окно приложения.
 *
 *  MainWindow отвечает за управление пользовательским интерфейсом,
 *  взаимодействие с сервером и отображение информации пользователю.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса MainWindow.
     *
     * Инициализирует главное окно приложения.
     *
     * @param parent Указатель на родительский виджет (по умолчанию nullptr).
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса MainWindow.
     *
     * Освобождает ресурсы, выделенные для пользовательского интерфейса.
     */
    ~MainWindow();

    /**
     * @brief Устанавливает информацию о пользователе (никнейм).
     *
     * @param nickname Никнейм пользователя.
     */
    void setUserInfo(const QString &nickname);

signals:
    /**
     * @brief Сигнал, генерируемый при запросе на начало игры.
     *
     *  Этот сигнал генерируется при нажатии на кнопку "Начать игру".
     */
    void startGameRequested();

private slots:
    /**
     * @brief Слот для обработки нажатия на кнопку "Начать игру".
     */
    void on_startButton_clicked();

    /**
     * @brief Слот для обработки нажатия на кнопку "Подключиться/Отключиться".
     */
    void connectButtonClicked();

    /**
     * @brief Слот для обработки нажатия на кнопку "Отправить".
     */
    void sendButtonClicked();

    /**
     * @brief Слот для обработки полученного сообщения от сервера.
     *
     * @param message Сообщение, полученное от сервера.
     */
    void handleMessage(const QString& message);

    /**
     * @brief Слот для обработки изменения состояния подключения.
     *
     * @param connected True, если клиент подключен к серверу, False - если нет.
     */
    void handleConnectionChanged(bool connected);

    /**
     * @brief Слот для обработки ошибок.
     *
     * @param error Сообщение об ошибке.
     */
    void handleError(const QString& error);

private:
    /**
     * @brief Указатель на объект пользовательского интерфейса (UI).
     */
    Ui::MainWindow *ui;

    /**
     * @brief Обновляет UI в зависимости от состояния подключения.
     */
    void updateUI();

    /**
     * @brief Текущий никнейм пользователя.
     */
    QString currentNickname;
};

#endif // MAINWINDOW_H