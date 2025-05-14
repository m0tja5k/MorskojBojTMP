/**
 * @file NetworkClient.cpp
 * @brief Реализация класса NetworkClient.
 *
 *  Этот файл содержит реализацию класса NetworkClient, который отвечает
 *  за сетевое взаимодействие с сервером.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "NetworkClient.h"
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief Получение экземпляра NetworkClient (Singleton).
 *
 *  Возвращает единственный экземпляр класса NetworkClient, используя паттерн Singleton.
 *
 * @return Ссылка на экземпляр NetworkClient.
 */
NetworkClient& NetworkClient::instance()
{
    static NetworkClient instance;
    return instance;
}

/**
 * @brief Конструктор класса NetworkClient.
 *
 *  Инициализирует сокет, подключает сигналы и слоты для обработки сетевых событий,
 *  а также настраивает таймер для переподключения.
 *
 * @param parent Указатель на родительский объект (по умолчанию nullptr).
 */
NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    /**
     * @brief Подключение к сигналам сокета.
     *
     *  Подключает сигналы QTcpSocket к соответствующим слотам NetworkClient
     *  для обработки событий, таких как подключение, отключение, получение данных и ошибки.
     */
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onError);

    /**
     * @brief Подключение таймера переподключения.
     *
     *  Подключает таймер m_reconnectTimer к лямбда-функции, которая пытается
     *  переподключиться к серверу.
     */
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "Attempting to reconnect...";
        connectToServer();
    });
    m_reconnectTimer.setInterval(5000);

    connect(this, &NetworkClient::shipPlacedSuccessfully, this, &NetworkClient::sendNextShip);
}

/**
 * @brief Регистрация пользователя на сервере.
 *
 *  Отправляет запрос на регистрацию пользователя на сервер с указанным никнеймом,
 *  email и паролем.
 *
 * @param nickname Никнейм пользователя.
 * @param email Email пользователя.
 * @param password Пароль пользователя.
 */
void NetworkClient::registerUser(const QString &nickname, const QString &email,
                                 const QString &password)
{
    QJsonObject json;
    json["type"] = "register";
    json["nickname"] = nickname;
    json["email"] = email;
    json["password"] = password;

    sendMessage(QJsonDocument(json).toJson());
    qDebug() << "Sent register request:" << QJsonDocument(json).toJson(QJsonDocument::Compact);
}

/**
 * @brief Авторизация пользователя на сервере.
 *
 *  Отправляет запрос на авторизацию пользователя на сервер с указанным никнеймом
 *  и паролем.
 *
 * @param nickname Никнейм пользователя.
 * @param password Пароль пользователя.
 */
void NetworkClient::loginUser(const QString &nickname, const QString &password)
{
    QJsonObject json;
    json["type"] = "login";
    json["nickname"] = nickname;
    json["password"] = password;

    sendMessage(QJsonDocument(json).toJson());
    qDebug() << "Sent login request:" << QJsonDocument(json).toJson(QJsonDocument::Compact);
}

/**
 * @brief Запрос на начало игры.
 *
 *  Отправляет запрос на начало игры на сервер, включая никнейм текущего пользователя.
 */
void NetworkClient::requestStartGame()
{
    if (isConnected()) {
        QJsonObject json;
        json["type"] = "start_game";
        json["nickname"] = currentNickname;

        sendMessage(QJsonDocument(json).toJson());
        qDebug() << "Sent start_game request:" << QJsonDocument(json).toJson(QJsonDocument::Compact);
    }
}

/**
 * @brief Отправка запроса на размещение корабля.
 *
 *  Отправляет запрос на размещение корабля на сервер с указанными параметрами.
 *
 * @param gameId Идентификатор игры.
 * @param x Координата X ячейки.
 * @param y Координата Y ячейки.
 * @param size Размер корабля.
 * @param isHorizontal True, если корабль расположен горизонтально, иначе - False.
 */
void NetworkClient::placeShip(int gameId, int x, int y, int size, bool isHorizontal)
{
    if (isConnected()) {
        QJsonObject json;
        json["type"] = "place_ship";
        json["nickname"] = currentNickname;
        json["game_id"] = gameId;
        json["x"] = x;
        json["y"] = y;
        json["size"] = size;
        json["is_horizontal"] = isHorizontal;

        sendMessage(QJsonDocument(json).toJson());
        qDebug() << "Sent place_ship request: game_id=" << gameId << ", x=" << x << ", y=" << y << ", size=" << size << ", is_horizontal=" << isHorizontal;
    }
}

/**
 * @brief Добавление корабля в очередь для отправки на сервер.
 *
 *  Добавляет информацию о корабле в очередь для последующей отправки на сервер.
 *
 * @param gameId Идентификатор игры.
 * @param x Координата X ячейки.
 * @param y Координата Y ячейки.
 * @param size Размер корабля.
 * @param isHorizontal True, если корабль расположен горизонтально, иначе - False.
 */
void NetworkClient::queueShip(int gameId, int x, int y, int size, bool isHorizontal)
{
    Ship ship;
    ship.gameId = gameId;
    ship.x = x;
    ship.y = y;
    ship.size = size;
    ship.isHorizontal = isHorizontal;
    shipQueue.enqueue(ship);
}

/**
 * @file NetworkClient.cpp
 * @brief Реализация класса NetworkClient (продолжение).
 *
 *  Этот файл содержит реализацию класса NetworkClient, который отвечает
 *  за сетевое взаимодействие с сервером (продолжение).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "NetworkClient.h"
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief Добавление корабля в очередь для отправки на сервер.
 *
 *  Добавляет информацию о корабле в очередь для последующей отправки на сервер.
 *  Если это первый корабль в очереди, немедленно отправляет его.
 *
 * @param gameId Идентификатор игры.
 * @param x Координата X ячейки.
 * @param y Координата Y ячейки.
 * @param size Размер корабля.
 * @param isHorizontal True, если корабль расположен горизонтально, иначе - False.
 */
void NetworkClient::queueShip(int gameId, int x, int y, int size, bool isHorizontal)
{
    Ship ship;
    ship.gameId = gameId;
    ship.x = x;
    ship.y = y;
    ship.size = size;
    ship.isHorizontal = isHorizontal;
    shipQueue.enqueue(ship);
    qDebug() << "Queued ship: game_id=" << gameId << ", x=" << x << ", y=" << y << ", size=" << size << ", is_horizontal=" << isHorizontal;
    /**
     * @brief Отправка первого корабля из очереди.
     *
     *  Если это первый корабль в очереди, отправляет его немедленно.
     */
    if (shipQueue.size() == 1) {
        sendNextShip();
    }
}

/**
 * @brief Отправка следующего корабля из очереди.
 *
 *  Отправляет следующий корабль из очереди, если очередь не пуста.
 *  Генерирует сигнал allShipsPlaced, если очередь пуста.
 */
void NetworkClient::sendNextShip()
{
    if (!shipQueue.isEmpty()) {
        Ship ship = shipQueue.head();
        placeShip(ship.gameId, ship.x, ship.y, ship.size, ship.isHorizontal);
    } else {
        emit allShipsPlaced();
    }
}

/**
 * @brief Отправка запроса на готовность к битве.
 *
 *  Отправляет запрос на сервер, указывающий, что игрок готов к началу боя.
 *
 * @param gameId Идентификатор игры.
 */
void NetworkClient::readyToBattle(int gameId)
{
    if (isConnected()) {
        QJsonObject json;
        json["type"] = "ready_to_battle";
        json["nickname"] = currentNickname;
        json["game_id"] = gameId;

        sendMessage(QJsonDocument(json).toJson());
        qDebug() << "Sent ready_to_battle request:" << QJsonDocument(json).toJson(QJsonDocument::Compact);
    }
}

/**
 * @brief Отправка запроса на совершение хода.
 *
 *  Отправляет запрос на сервер с информацией о ходе игрока.
 *
 * @param gameId Идентификатор игры.
 * @param x Координата X ячейки, по которой был произведен выстрел.
 * @param y Координата Y ячейки, по которой был произведен выстрел.
 */
void NetworkClient::sendMove(int gameId, int x, int y)
{
    if (isConnected()) {
        QJsonObject json;
        json["type"] = "make_move";
        json["nickname"] = currentNickname;
        json["game_id"] = gameId;
        json["x"] = x;
        json["y"] = y;

        sendMessage(QJsonDocument(json).toJson());
        qDebug() << "Sent make_move request: game_id=" << gameId << ", x=" << x << ", y=" << y;
    }
}

/**
 * @brief Обработчик события "Данные готовы к чтению".
 *
 *  Считывает данные из сокета, обрабатывает их как JSON и генерирует соответствующие сигналы.
 */
void NetworkClient::onReadyRead()
{
    /**
     * @brief Чтение данных из сокета по строкам.
     */
    while (m_socket->canReadLine()) {
        const QByteArray data = m_socket->readLine().trimmed();
        qDebug() << "Received raw data from server:" << data;

        /**
         * @brief Преобразование полученных данных в JSON-документ.
         */
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qDebug() << "Invalid JSON received:" << QString::fromUtf8(data);
            emit messageReceived(QString::fromUtf8(data));
            continue;
        }

        /**
         * @brief Обработка JSON-объекта.
         *
         *  Получение типа сообщения и текущего хода из полученного JSON.
         */
        QJsonObject json = doc.object();
        QString type = json["type"].toString();
        QString currentTurn = json["current_turn"].toString();
        qDebug() << "Parsed message type:" << type << "- current_turn:" << currentTurn << "- Full message:" << QString::fromUtf8(data);

        /**
         * @brief Обработка сообщения о регистрации.
         */
        if (type == "register") {
            if (json["status"] == "success") {
                currentNickname = json["nickname"].toString();
                qDebug() << "Registration successful for nickname:" << currentNickname;
                emit registrationSuccess();
            } else {
                qDebug() << "Registration failed. Reason:" << json["message"].toString();
                emit registrationFailed(json["message"].toString());
            }
        }
        /**
         * @brief Обработка сообщения об авторизации.
         */
        else if (type == "login") {
            if (json["status"] == "success") {
                currentNickname = json["nickname"].toString();
                qDebug() << "Login successful for nickname:" << currentNickname;
                emit loginSuccess(json["nickname"].toString());
            } else {

/**
 * @file NetworkClient.cpp
 * @brief Реализация класса NetworkClient (окончание).
 *
 *  Этот файл содержит реализацию класса NetworkClient, который отвечает
 *  за сетевое взаимодействие с сервером (окончание).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "NetworkClient.h"
#include <QJsonObject>
#include <QJsonDocument>

// ... (Предыдущий код NetworkClient) ...

        /**
         * @brief Обработка сообщения об авторизации.
         */
        else if (type == "login") {
            if (json["status"] == "success") {
                currentNickname = json["nickname"].toString();
                qDebug() << "Login successful for nickname:" << currentNickname;
                emit loginSuccess(json["nickname"].toString());
            } else {
                qDebug() << "Login failed. Reason:" << json["message"].toString();
                emit loginFailed(json["message"].toString());
            }
        }
        /**
         * @brief Обработка сообщения о начале игры (ожидание противника).
         */
        else if (type == "start_game") {
            if (json["status"] == "waiting") {
                qDebug() << "Start game: Waiting for opponent";
                emit startGameWaiting();
            } else {
                qDebug() << "Unexpected start_game response:" << json;
            }
        }
        /**
         * @brief Обработка сообщения о готовности к игре.
         */
        else if (type == "game_ready") {
            int gameId = json["game_id"].toInt();
            currentGameId = gameId;
            qDebug() << "Game ready. Game ID:" << gameId << "Opponent:" << json["opponent"].toString();
            emit gameReady(gameId, json["opponent"].toString());
        }
        /**
         * @brief Обработка сообщения об успешной установке корабля.
         */
        else if (type == "place_ship") {
            if (json["status"] == "success") {
                qDebug() << "Ship placed successfully";
                shipQueue.dequeue();
                emit shipPlacedSuccessfully();
            } else {
                qDebug() << "Failed to place ship. Reason:" << json["message"].toString();
                emit shipPlacementFailed(json["message"].toString());
            }
        }
        /**
         * @brief Обработка сообщения об подтверждении готовности к битве.
         */
        else if (type == "ready_to_battle") {
            if (json["status"] == "success") {
                qDebug() << "Ready to battle confirmed";
                emit readyToBattleConfirmed();
            } else {
                qDebug() << "Failed to confirm ready_to_battle. Reason:" << json["message"].toString();
                emit readyToBattleFailed(json["message"].toString());
            }
        }
        /**
         * @brief Обработка сообщения о начале игры.
         */
        else if (type == "game_start") {
            qDebug() << "Game started. Current turn:" << currentTurn;
            emit gameStarted(currentTurn);
            emit updateUIEnabled(currentTurn == currentNickname);
            qDebug() << "UI enabled:" << (currentTurn == currentNickname);
        }
        /**
         * @brief Обработка сообщения о результате собственного хода.
         */
        else if (type == "make_move") {
            QString status = json["status"].toString();
            int x = json["x"].toInt();
            int y = json["y"].toInt();
            QString message = json["message"].toString();
            qDebug() << "Make move - Status:" << status << "at (" << x << "," << y << ") - Message:" << message;
            emit ownMoveResult(status, x, y, message); // Изменено: ownMoveResult вместо updateOpponentField
            emit updateUIEnabled(currentTurn == currentNickname);
            qDebug() << "UI enabled:" << (currentTurn == currentNickname);
        }
        /**
         * @brief Обработка сообщения о результате хода противника.
         */
        else if (type == "move_result") {
            QString status = json["status"].toString();
            int x = json["x"].toInt();
            int y = json["y"].toInt();
            QString message = json["message"].toString();
            qDebug() << "Move result - Status:" << status << "at (" << x << "," << y << ") - Message:" << message;
            emit moveResult(status, x, y, message);
        }
        /**
         * @brief Обработка прочих сообщений.
         */
        else {
            emit messageReceived(QString::fromUtf8(data));
        }
    }
}

/**
 * @file NetworkClient.cpp
 * @brief Реализация класса NetworkClient (окончание).
 *
 *  Этот файл содержит реализацию класса NetworkClient, который отвечает
 *  за сетевое взаимодействие с сервером (окончание).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "NetworkClient.h"
#include <QJsonObject>
#include <QJsonDocument>

// ... (Предыдущий код NetworkClient) ...

        /**
         * @brief Обработка сообщения о результате хода противника.
         */
        else if (type == "move_result") {
            QString status = json["status"].toString();
            int x = json["x"].toInt();
            int y = json["y"].toInt();
            QString message = json["message"].toString();
            qDebug() << "Move result - Status:" << status << "at (" << x << "," << y << ") - Message:" << message;
            emit moveResult(status, x, y, message);
            emit updateUIEnabled(currentTurn == currentNickname);
            qDebug() << "UI enabled:" << (currentTurn == currentNickname);
        }
        /**
         * @brief Обработка сообщения об ошибке от сервера.
         */
        else if (type == "error") {
            qDebug() << "Error from server:" << json["message"].toString();
            /**
             * @brief Деактивация UI, если ход игрока не его.
             */
            if (json["message"].toString() == "Not your turn") {
                emit updateUIEnabled(false);
                qDebug() << "UI disabled due to 'Not your turn'";
            }
            emit messageReceived(QString::fromUtf8(data));
        }
        /**
         * @brief Обработка сообщения об окончании игры.
         */
        else if (type == "gameover") {
            qDebug() << "Game over. Message:" << json["message"].toString();
            emit gameOver(json["message"].toString());
            emit updateUIEnabled(false);
            qDebug() << "UI disabled due to game over";
        }
        /**
         * @brief Обработка необработанных сообщений.
         */
        else {
            qDebug() << "Unhandled message type:" << type << "- Full message:" << json;
            emit messageReceived(QString::fromUtf8(data));
        }
    }
}

/**
 * @brief Подключение к серверу.
 *
 *  Устанавливает соединение с сервером по указанному адресу и порту.
 *
 * @param host Адрес сервера.
 * @param port Порт сервера.
 */
void NetworkClient::connectToServer(const QString& host, quint16 port)
{
    QMutexLocker locker(&m_mutex);
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    qDebug() << "Connecting to" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

/**
 * @brief Отключение от сервера.
 *
 *  Закрывает соединение с сервером. Останавливает таймер переподключения.
 */
void NetworkClient::disconnectFromServer()
{
    QMutexLocker locker(&m_mutex);
    m_reconnectTimer.stop();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    qDebug() << "Disconnected from server manually.";
}

/**
 * @brief Отправка сообщения на сервер.
 *
 *  Отправляет сообщение на сервер, добавляя к нему символы конца строки.
 *
 * @param message Сообщение для отправки.
 */
void NetworkClient::sendMessage(const QString& message)
{
    QMutexLocker locker(&m_mutex);
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QByteArray data = message.toUtf8() + "\r\n";
        qDebug() << "Sending message:" << data;
        m_socket->write(data);
        m_socket->flush();
    } else {
        qDebug() << "Cannot send message, socket not connected. State:" << m_socket->state();
    }
}

/**
 * @brief Проверка состояния подключения.
 *
 *  Возвращает true, если сокет подключен к серверу, иначе - false.
 *
 * @return True, если подключено, иначе - false.
 */
bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

/**
 * @brief Слот, вызываемый при успешном подключении к серверу.
 *
 *  Останавливает таймер переподключения и генерирует сигнал connectionChanged.
 */
void NetworkClient::onConnected()
{
    qDebug() << "Connected to server!";
    m_reconnectTimer.stop();
    emit connectionChanged(true);
}

/**
 * @brief Слот, вызываемый при отключении от сервера.
 *
 *  Генерирует сигнал connectionChanged.
 */
void NetworkClient::onDisconnected()
{
    qDebug() << "Disconnected from server!";
    emit connectionChanged(false);
}

/**
 * @brief Слот, вызываемый при возникновении ошибки сокета.
 *
 *  Генерирует сигнал errorOccurred и отключается от сервера.
 *
 * @param socketError Код ошибки сокета.
 */
void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    emit errorOccurred(m_socket->errorString());
    disconnectFromServer();
}

/**
 * @brief Устанавливает текущий никнейм пользователя.
 *
 * @param nickname Новый никнейм пользователя.
 */
void NetworkClient::setCurrentNickname(const QString& nickname)
{
    currentNickname = nickname;
}





























         