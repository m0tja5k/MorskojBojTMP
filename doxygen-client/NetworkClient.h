/**
 * @file NetworkClient.h
 * @brief Объявление класса NetworkClient.
 *
 *  Этот файл содержит объявление класса NetworkClient, который управляет
 *  сетевым взаимодействием с сервером в приложении "Морской бой". Он
 *  включает методы для подключения, отправки и приема данных, а также
 *  обработки различных событий, связанных с сетевым взаимодействием.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QQueue>

/**
 * @struct Ship
 * @brief Структура, представляющая информацию о корабле.
 *
 *  Эта структура содержит данные о корабле, необходимые для его размещения
 *  и идентификации на сервере.
 */
struct Ship {
    int gameId;         ///< Идентификатор игры.
    int x;              ///< Координата X ячейки.
    int y;              ///< Координата Y ячейки.
    int size;           ///< Размер корабля.
    bool isHorizontal;  ///< True, если корабль расположен горизонтально, иначе - False.
};

/**
 * @class NetworkClient
 * @brief Класс, управляющий сетевым взаимодействием с сервером.
 *
 *  Этот класс реализует логику подключения к серверу, отправки и приема
 *  различных типов сообщений, а также обработки сетевых событий.
 */
class NetworkClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Получение экземпляра NetworkClient (Singleton).
     *
     *  Реализует паттерн Singleton для обеспечения единственного экземпляра
     *  класса NetworkClient.
     *
     * @return Ссылка на экземпляр NetworkClient.
     */
    static NetworkClient& instance();

    /**
     * @brief Подключение к серверу.
     *
     *  Устанавливает соединение с сервером по указанному адресу и порту.
     *
     * @param host Адрес сервера (по умолчанию "127.0.0.1").
     * @param port Порт сервера (по умолчанию 33333).
     */
    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 33333);

    /**
     * @brief Отключение от сервера.
     *
     *  Закрывает соединение с сервером. Останавливает таймер переподключения.
     */
    void disconnectFromServer();

    /**
     * @brief Отправка сообщения на сервер.
     *
     *  Отправляет сообщение на сервер.
     *
     * @param message Сообщение для отправки.
     */
    void sendMessage(const QString& message);

    /**
     * @brief Проверка состояния подключения.
     *
     *  Проверяет, подключен ли клиент к серверу.
     *
     * @return True, если клиент подключен, иначе - False.
     */
    bool isConnected() const;

    /**
     * @brief Регистрация пользователя на сервере.
     *
     * @param nickname Никнейм пользователя.
     * @param email Email пользователя.
     * @param password Пароль пользователя.
     */
    void registerUser(const QString &nickname, const QString &email, const QString &password);

    /**
     * @brief Авторизация пользователя на сервере.
     *
     * @param nickname Никнейм пользователя.
     * @param password Пароль пользователя.
     */
    void loginUser(const QString &nickname, const QString &password);

    /**
     * @brief Запрос на начало игры.
     */
    void requestStartGame();

    /**
     * @brief Отправка запроса на размещение корабля.
     *
     * @param gameId Идентификатор игры.
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @param size Размер корабля.
     * @param isHorizontal True, если корабль расположен горизонтально, иначе - False.
     */
    void placeShip(int gameId, int x, int y, int size, bool isHorizontal);

    /**
     * @brief Добавление корабля в очередь для отправки на сервер.
     *
     * @param gameId Идентификатор игры.
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @param size Размер корабля.
     * @param isHorizontal True, если корабль расположен горизонтально, иначе - False.
     */
    void queueShip(int gameId, int x, int y, int size, bool isHorizontal);

    /**
     * @brief Отправка запроса на готовность к битве.
     *
     * @param gameId Идентификатор игры.
     */
    void readyToBattle(int gameId);

    /**
     * @brief Отправка запроса на совершение хода.
     *
     * @param gameId Идентификатор игры.
     * @param x Координата X ячейки, по которой был произведен выстрел.
     * @param y Координата Y ячейки, по которой был произведен выстрел.
     */
    void sendMove(int gameId, int x, int y);

    /**
     * @brief Устанавливает текущий никнейм пользователя.
     *
     * @param nickname Новый никнейм пользователя.
     */
    void setCurrentNickname(const QString& nickname);

    /**
     * @brief Получает текущий идентификатор игры.
     *
     * @return Текущий идентификатор игры.
     */
    int getGameId() const { return currentGameId; }

    /**
     * @brief Получает текущий никнейм пользователя.
     *
     * @return Текущий никнейм пользователя.
     */
    QString getCurrentNickname() const { return currentNickname; }

signals:
    /**
     * @brief Сигнал, генерируемый при изменении состояния подключения.
     *
     * @param connected True, если клиент подключен, иначе - False.
     */
    void connectionChanged(bool connected);

    /**
     * @brief Сигнал, генерируемый при возникновении ошибки.
     *
     * @param error Сообщение об ошибке.
     */
    void errorOccurred(const QString &error);

    /**
     * @brief Сигнал, генерируемый при получении сообщения от сервера.
     *
     * @param message Полученное сообщение.
     */
    void messageReceived(const QString &message);

    /**
     * @brief Сигнал, генерируемый при успешной регистрации.
     */
    void registrationSuccess();

    /**
     * @brief Сигнал, генерируемый при неудачной регистрации.
     *
     * @param reason Причина неудачи.
     */
    void registrationFailed(const QString &reason);

    /**
     * @brief Сигнал, генерируемый при успешной авторизации.
     *
     * @param nickname Никнейм пользователя.
     */
    void loginSuccess(const QString &nickname);

    /**
     * @brief Сигнал, генерируемый при неудачной авторизации.
     *
     * @param reason Причина неудачи.
     */
    void loginFailed(const QString &reason);

    /**
     * @brief Сигнал, генерируемый при ожидании начала игры.
     */
    void startGameWaiting();

    /**
     * @brief Сигнал, генерируемый при готовности к игре.
     *
     * @param gameId Идентификатор игры.
     * @param opponent Никнейм противника.
     */
    void gameReady(int gameId, const QString &opponent);

    /**
     * @brief Сигнал, генерируемый при успешном размещении корабля.
     */
    void shipPlacedSuccessfully();

    /**
     * @brief Сигнал, генерируемый при неудачном размещении корабля.
     *
     * @param reason Причина неудачи.
     */
    void shipPlacementFailed(const QString &reason);

    /**
     * @brief Сигнал, генерируемый при успешной установке всех кораблей.
     */
    void allShipsPlaced();

    /**
     * @brief Сигнал, генерируемый при подтверждении готовности к битве.
     */
    void readyToBattleConfirmed();

    /**
     * @brief Сигнал, генерируемый при неудачном подтверждении готовности к битве.
     *
     * @param reason Причина неудачи.
     */
    void readyToBattleFailed(const QString &reason);

    /**
     * @brief Сигнал, генерируемый при начале игры.
     *
     * @param tTurn Никнейм игрока, чей сейчас ход.
     */
    void gameStarted(const QString &tTurn);

    /**
     * @brief Сигнал, генерируемый при получении результата хода.
     *
     * @param status "hit" или "miss", указывающий результат хода.
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @param message Сообщение о результате хода.
     */
    void moveResult(const QString &status, int x, int y, const QString &message);

    /**
     * @brief Сигнал, генерируемый при получении результата собственного хода.
     *
     * @param status "hit" или "miss", указывающий результат хода.
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @param message Сообщение о результате хода.
     */
    void ownMoveResult(const QString &status, int x, int y, const QString &message);

    /**
     * @brief Сигнал, генерируемый при окончании игры.
     *
     * @param message Сообщение об окончании игры.
     */
    void gameOver(const QString &message);

    /**
     * @brief Сигнал, генерируемый при необходимости обновить доступность UI.
     *
     * @param enabled True, если UI должен быть активен, иначе - False.
     */
    void updateUIEnabled(bool enabled);

    /**
     * @brief Сигнал, генерируемый при необходимости обновить поле противника.
     *
     * @param x Координата X ячейки.
     * @param y Координата Y ячейки.
     * @param status Статус ячейки.
     */
    void updateOpponentField(int x, int y, QString status);

public slots:
    /**
     * @brief Слот, вызываемый при успешном подключении к серверу.
     */
    void onConnected();

    /**
     * @brief Слот, вызываемый при отключении от сервера.
     */
    void onDisconnected();

    /**
     * @brief Слот, вызываемый при получении данных от сервера.
     */
    void onReadyRead();

    /**
     * @brief Слот, вызываемый при возникновении ошибки сокета.
     *
     * @param socketError Код ошибки сокета.
     */
    void onError(QAbstractSocket::SocketError socketError);

    /**
     * @brief Слот, вызываемый для отправки следующего корабля из очереди.
     */
    void sendNextShip();

private:
    /**
     * @brief Конструктор класса NetworkClient (private).
     *
     *  Используется для реализации паттерна Singleton.
     *
     * @param parent Указатель на родительский объект (по умолчанию nullptr).
     */
    NetworkClient(QObject* parent = nullptr);

    /**
     * @brief Деструктор класса NetworkClient (default).
     */
    ~NetworkClient() = default;

    /**
     * @brief Запрет копирования класса NetworkClient.
     */
    NetworkClient(const NetworkClient&) = delete;

    /**
     * @brief Запрет оператора присваивания для класса NetworkClient.
     */
    NetworkClient& operator=(const NetworkClient&) = delete;

    /**
     * @brief Указатель на сокет TCP.
     */
    QTcpSocket* m_socket;

    /**
     * @brief Мьютекс для обеспечения потокобезопасности.
     */
    QMutex m_mutex;

    /**
     * @brief Таймер для переподключения к серверу.
     */
    QTimer m_reconnectTimer;

    /**
     * @brief Текущий никнейм пользователя.
     */
    QString currentNickname;

    /**
     * @brief Текущий идентификатор игры.
     */
    int currentGameId = -1;

    /**
     * @brief Очередь кораблей для отправки на сервер.
     */
    QQueue<Ship> shipQueue;
};

#endif // NETWORKCLIENT_H