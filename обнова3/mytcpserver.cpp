#include "mytcpserver.h"
#include "func2serv.h"
#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QCryptographicHash>
#include <QUrlQuery>

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent) {
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "Server is NOT started!";
    } else {
        qDebug() << "Server is started!";
    }
}

MyTcpServer::~MyTcpServer() {
    mTcpServer->close();
}

void MyTcpServer::slotNewConnection() {
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();

    if (clientSocket) {
        connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);
        qDebug() << "New client connected!";
    }
}

void MyTcpServer::slotServerRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        QByteArray requestData = clientSocket->readAll();
        QString request = QString::fromUtf8(requestData).trimmed();  // Убираем лишние символы

        // Обрабатываем команду
        QByteArray response = parse(request);  // Парсим запрос
        clientSocket->write(response);  // Отправляем ответ клиенту
        clientSocket->flush();  // Принудительная отправка данных
    }
}

void MyTcpServer::slotClientDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        qDebug() << "Client disconnected!";
        clientSocket->deleteLater();
    }
}
