#include "mytcpserver.h"
#include "func2serv.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
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
            // Отправляем сообщение об ошибке, если сервер полон
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

        // Парсим запрос
        QByteArray response = parse(request, this);

        // Проверяем, если это запрос login или register, и регистрируем клиента
        QJsonDocument doc = QJsonDocument::fromJson(request.toUtf8());
        if (doc.isObject()) {
            QJsonObject jsonObj = doc.object();
            QString type = jsonObj["type"].toString();
            if ((type == "register" || type == "login") && jsonObj["status"].toString() == "success") {
                QString nickname = jsonObj["nickname"].toString();
                if (!nickname.isEmpty()) {
                    registerClient(nickname, clientSocket);
                }
            }
        }

        clientSocket->write(response);
        clientSocket->flush();
    }
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        QString nickname = getNicknameBySocket(clientSocket);
        if (!nickname.isEmpty()) {
            unregisterClient(clientSocket);
            qDebug() << "Client" << nickname << "disconnected!";
        }
        clientSocket->deleteLater();
    }
}

void MyTcpServer::sendMessageToUser(const QString &nickname, const QByteArray &message)
{
    QMutexLocker locker(&mutex);
    if (mClients.contains(nickname)) {
        QTcpSocket *socket = mClients[nickname];
        socket->write(message);
        socket->flush();
        qDebug() << "Sent message to" << nickname << ":" << message;
    } else {
        qDebug() << "User" << nickname << "not found or not connected.";
    }
}

void MyTcpServer::registerClient(const QString &nickname, QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);
    mClients.insert(nickname, socket);
    mSocketToNickname.insert(socket, nickname);
    qDebug() << "Registered client:" << nickname;
}

void MyTcpServer::unregisterClient(QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);
    QString nickname = mSocketToNickname.value(socket, "");
    if (!nickname.isEmpty()) {
        mClients.remove(nickname);
        mSocketToNickname.remove(socket);
        players.removeAll(nickname);
        // Если второй игрок остался один, уведомляем его и сбрасываем игру
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
    qDebug() << "Game reset.";
}
