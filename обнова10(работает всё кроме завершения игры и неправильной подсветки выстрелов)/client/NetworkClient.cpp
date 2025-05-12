#include "NetworkClient.h"
#include <QJsonObject>
#include <QJsonDocument>

NetworkClient& NetworkClient::instance()
{
    static NetworkClient instance;
    return instance;
}

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onError);

    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "Attempting to reconnect...";
        connectToServer();
    });
    m_reconnectTimer.setInterval(5000);

    connect(this, &NetworkClient::shipPlacedSuccessfully, this, &NetworkClient::sendNextShip);
}

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

void NetworkClient::loginUser(const QString &nickname, const QString &password)
{
    QJsonObject json;
    json["type"] = "login";
    json["nickname"] = nickname;
    json["password"] = password;

    sendMessage(QJsonDocument(json).toJson());
    qDebug() << "Sent login request:" << QJsonDocument(json).toJson(QJsonDocument::Compact);
}

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
    if (shipQueue.size() == 1) {
        sendNextShip();
    }
}

void NetworkClient::sendNextShip()
{
    if (!shipQueue.isEmpty()) {
        Ship ship = shipQueue.head();
        placeShip(ship.gameId, ship.x, ship.y, ship.size, ship.isHorizontal);
    } else {
        emit allShipsPlaced();
    }
}

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

void NetworkClient::onReadyRead()
{
    while (m_socket->canReadLine()) {
        const QByteArray data = m_socket->readLine().trimmed();
        qDebug() << "Received raw data from server:" << data;

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull()) {
            qDebug() << "Invalid JSON received:" << QString::fromUtf8(data);
            emit messageReceived(QString::fromUtf8(data));
            continue;
        }

        QJsonObject json = doc.object();
        QString type = json["type"].toString();
        QString currentTurn = json["current_turn"].toString();
        qDebug() << "Parsed message type:" << type << "- current_turn:" << currentTurn << "- Full message:" << QString::fromUtf8(data);

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
        else if (type == "start_game") {
            if (json["status"] == "waiting") {
                qDebug() << "Start game: Waiting for opponent";
                emit startGameWaiting();
            } else {
                qDebug() << "Unexpected start_game response:" << json;
            }
        }
        else if (type == "game_ready") {
            int gameId = json["game_id"].toInt();
            currentGameId = gameId;
            qDebug() << "Game ready. Game ID:" << gameId << "Opponent:" << json["opponent"].toString();
            emit gameReady(gameId, json["opponent"].toString());
        }
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
        else if (type == "ready_to_battle") {
            if (json["status"] == "success") {
                qDebug() << "Ready to battle confirmed";
                emit readyToBattleConfirmed();
            } else {
                qDebug() << "Failed to confirm ready_to_battle. Reason:" << json["message"].toString();
                emit readyToBattleFailed(json["message"].toString());
            }
        }
        else if (type == "game_start") {
            qDebug() << "Game started. Current turn:" << currentTurn;
            emit gameStarted(currentTurn);
            emit updateUIEnabled(currentTurn == currentNickname);
            qDebug() << "UI enabled:" << (currentTurn == currentNickname);
        }
        else if (type == "make_move") {
            QString status = json["status"].toString();
            int x = json["x"].toInt();
            int y = json["y"].toInt();
            qDebug() << "Make move - Status:" << status << "at (" << x << "," << y << ") - Message:" << json["message"].toString();
            emit updateOpponentField(x, y, status);
            emit updateUIEnabled(currentTurn == currentNickname);
            qDebug() << "UI enabled:" << (currentTurn == currentNickname);
        }
        else if (type == "move_result") {
            QString status = json["status"].toString();
            int x = json["x"].toInt();
            int y = json["y"].toInt();
            qDebug() << "Move result - Status:" << status << "at (" << x << "," << y << ") - Message:" << json["message"].toString();
            emit moveResult(status, x, y, json["message"].toString());
            emit updateUIEnabled(currentTurn == currentNickname);
            qDebug() << "UI enabled:" << (currentTurn == currentNickname);
        }
        else if (type == "error") {
            qDebug() << "Error from server:" << json["message"].toString();
            if (json["message"].toString() == "Not your turn") {
                emit updateUIEnabled(false);
                qDebug() << "UI disabled due to 'Not your turn'";
            }
            emit messageReceived(QString::fromUtf8(data));
        }
        else if (type == "gameover") {
            qDebug() << "Game over. Message:" << json["message"].toString();
            emit gameOver(json["message"].toString());
            emit updateUIEnabled(false);
            qDebug() << "UI disabled due to game over";
        }
        else {
            qDebug() << "Unhandled message type:" << type << "- Full message:" << json;
            emit messageReceived(QString::fromUtf8(data));
        }
    }
}

void NetworkClient::connectToServer(const QString& host, quint16 port)
{
    QMutexLocker locker(&m_mutex);
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    qDebug() << "Connecting to" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void NetworkClient::disconnectFromServer()
{
    QMutexLocker locker(&m_mutex);
    m_reconnectTimer.stop();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort();
    }
    qDebug() << "Disconnected from server manually.";
}

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

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::onConnected()
{
    qDebug() << "Connected to server!";
    m_reconnectTimer.stop();
    emit connectionChanged(true);
}

void NetworkClient::onDisconnected()
{
    qDebug() << "Disconnected from server!";
    emit connectionChanged(false);
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    emit errorOccurred(m_socket->errorString());
    disconnectFromServer();
}

void NetworkClient::setCurrentNickname(const QString& nickname)
{
    currentNickname = nickname;
}
