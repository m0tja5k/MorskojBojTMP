#include "mytcpserver.h"
#include "func2serv.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent), currentGameId(-1)
{
    mTcpServer = new QTcpServer(this);
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "Server is NOT started!";
    } else {
        qDebug() << "Server is started!";
    }
}

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();
    if (clientSocket) {
        if (getPlayerCount() >= 2) {
            QByteArray response = createJsonResponse("error", "error", "Server is full");
            clientSocket->write(response);
            clientSocket->flush();
            clientSocket->disconnectFromHost();
            return;
        }

        connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
        qDebug() << "New client connected from" << clientSocket->peerAddress().toString();
    }
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        QByteArray requestData = clientSocket->readAll();
        QString request = QString::fromUtf8(requestData).trimmed();

        qDebug() << "Received raw request:" << requestData.toHex();
        qDebug() << "Received input (parsed):" << request;
        QByteArray response = parse(request, this);

        QJsonDocument doc = QJsonDocument::fromJson(request.toUtf8());
        if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            QString type = jsonObj["type"].toString();
            QString nickname = jsonObj["nickname"].toString();
            if (type == "register" || type == "login") {
                if (!nickname.isEmpty()) {
                    registerClient(nickname, clientSocket);
                    qDebug() << "Clients registered:" << mClients.keys();
                }
            } else if (type == "ready_to_battle") {
                if (!nickname.isEmpty() && players.contains(nickname)) {
                    QMutexLocker locker(&mutex);
                    qDebug() << "Processing ready_to_battle for" << nickname << "- currentGameId:" << currentGameId << "- Socket state:" << clientSocket->state();
                    readyPlayers.insert(nickname);
                    qDebug() << "Player" << nickname << "is ready. Ready players:" << readyPlayers;
                    response = createJsonResponse("ready_to_battle", "success", "Ready status received");
                    if (readyPlayers.size() == 2 && currentGameId != -1) {
                        qDebug() << "Both players ready, starting game with gameId:" << currentGameId;
                        DatabaseManager *db = DatabaseManager::getInstance();
                        QString player1 = players[0];
                        db->updateTurn(currentGameId, player1);
                        QJsonObject startMsg;
                        startMsg["type"] = "game_start";
                        startMsg["status"] = "success";
                        startMsg["message"] = "Game started";
                        startMsg["current_turn"] = player1;
                        QByteArray startResponse = QJsonDocument(startMsg).toJson(QJsonDocument::Compact) + "\r\n";
                        qDebug() << "Prepared game_start message:" << startResponse;
                        // Отправка game_start всем клиентам с дополнительной проверкой
                        for (const QString &player : mClients.keys()) {
                            QTcpSocket *targetSocket = mClients[player];
                            qDebug() << "Attempting to send to" << player << "- Socket state:" << targetSocket->state();
                            if (targetSocket->state() == QAbstractSocket::ConnectedState) {
                                bool success = targetSocket->write(startResponse);
                                targetSocket->flush();
                                if (success) {
                                    qDebug() << "Successfully sent game_start to" << player;
                                } else {
                                    qDebug() << "Failed to send game_start to" << player << "- Error:" << targetSocket->errorString();
                                }
                            } else {
                                qDebug() << "Cannot send to" << player << "- Socket not connected, state:" << targetSocket->state();
                            }
                        }
                    }
                }
            }
        } else {
            qDebug() << "Failed to parse JSON for request:" << request;
            response = createJsonResponse("error", "error", "Invalid JSON format");
        }

        qDebug() << "Preparing to send response to" << getNicknameBySocket(clientSocket) << "- Socket state:" << clientSocket->state();
        if (clientSocket->state() == QAbstractSocket::ConnectedState) {
            qDebug() << "Sending response to client. Response:" << response;
            clientSocket->write(response);
            clientSocket->flush();
        } else {
            qDebug() << "Cannot send response to" << getNicknameBySocket(clientSocket) << ", socket state:" << clientSocket->state();
        }
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        QString nickname = getNicknameBySocket(clientSocket);
        if (!nickname.isEmpty()) {
            QMutexLocker locker(&mutex);
            readyPlayers.remove(nickname);
            unregisterClient(clientSocket);
            qDebug() << "Client" << nickname << "disconnected! Socket state:" << clientSocket->state();
        }
        clientSocket->deleteLater();
    }
}

void MyTcpServer::sendMessageToUser(const QString &nickname, const QByteArray &message)
{
    QMutexLocker locker(&mutex);
    if (mClients.contains(nickname)) {
        QTcpSocket *socket = mClients[nickname];
        if (socket->state() == QAbstractSocket::ConnectedState) {
            bool written = socket->write(message);
            socket->flush();
            if (written > 0) {
                qDebug() << "Sent message to" << nickname << ":" << message;
            } else {
                qDebug() << "Failed to write message to" << nickname << "- socket error:" << socket->errorString();
            }
        } else {
            qDebug() << "Socket for" << nickname << "is not connected. State:" << socket->state();
        }
    } else {
        qDebug() << "User" << nickname << "not found or not connected. Current clients:" << mClients.keys();
    }
}

void MyTcpServer::registerClient(const QString &nickname, QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);
    mClients.insert(nickname, socket);
    mSocketToNickname.insert(socket, nickname);
    qDebug() << "Registered client:" << nickname << "Socket state:" << socket->state();
}

void MyTcpServer::unregisterClient(QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);
    QString nickname = mSocketToNickname.value(socket, "");
    if (!nickname.isEmpty()) {
        mClients.remove(nickname);
        mSocketToNickname.remove(socket);
        players.removeAll(nickname);
        readyPlayers.remove(nickname);
        QString opponent = getOpponentInternal(nickname);
        if (!opponent.isEmpty()) {
            sendMessageToUser(opponent, createJsonResponse("gameover", "opponent_disconnected", "Opponent disconnected"));
            resetGame();
        }
    }
}

QString MyTcpServer::getNicknameBySocket(QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);
    return mSocketToNickname.value(socket, "");
}

void MyTcpServer::addPlayerToGame(const QString &nickname)
{
    QMutexLocker locker(&mutex);
    if (!players.contains(nickname)) {
        players.append(nickname);
        qDebug() << "Added player to game:" << nickname;
    }
}

QString MyTcpServer::getOpponent(const QString &nickname) const
{
    return const_cast<MyTcpServer*>(this)->getOpponentInternal(nickname);
}

QString MyTcpServer::getOpponentInternal(const QString &nickname)
{
    QMutexLocker locker(&mutex);
    for (const QString &player : players) {
        if (player != nickname) {
            return player;
        }
    }
    return "";
}

int MyTcpServer::getPlayerCount() const
{
    return const_cast<MyTcpServer*>(this)->getPlayerCountInternal();
}

int MyTcpServer::getPlayerCountInternal()
{
    QMutexLocker locker(&mutex);
    return players.size();
}

void MyTcpServer::resetGame()
{
    QMutexLocker locker(&mutex);
    players.clear();
    readyPlayers.clear();
    currentGameId = -1;
    qDebug() << "Game reset.";
}

int MyTcpServer::getGameId() const
{
    return currentGameId;
}
