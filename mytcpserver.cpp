#include "mytcpserver.h"
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

        // Если это запрос на регистрацию
        if (request.startsWith("POST /register")) {
            handleRegister(request.mid(13));  // Убираем POST /register
        }
        // Если это запрос на логин
        else if (request.startsWith("POST /login")) {
            slotLogin(request.mid(11));  // Убираем POST /login
        }
        // Другие команды
        else {
            clientSocket->write("Unknown command\r\n");
        }
    }
}
void MyTcpServer::slotLogin(const QString &data) {
    QString username, password;
    parseRegisterData(data, username, password);

    // Проверка на существование пользователя
    if (!mUsers.contains(username)) {
        qDebug() << "User not found!";
        return;
    }

    // Проверка пароля
    if (!isPasswordCorrect(username, password)) {
        qDebug() << "Incorrect password!";
        return;
    }

    // Отправляем клиенту успешный логин
    qDebug() << "User logged in: " << username;
}

bool MyTcpServer::isPasswordCorrect(const QString &username, const QString &password) {
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return mUsers.value(username) == passwordHash.toHex();
}
void MyTcpServer::parseRegisterData(const QString &data, QString &username, QString &password) {


    // Убираем всё перед "username="
    QString cleanedData = data;
    int index = cleanedData.indexOf("username=");
    if (index != -1) {
        cleanedData = cleanedData.mid(index);
    } else {
        qDebug() << "Username parameter not found";
        return;
    }

    // Декодируем строку запроса
    QString decodedData = QUrl::fromPercentEncoding(cleanedData.toUtf8());
    qDebug() << "Decoded data: " << decodedData;  // Логируем декодированную строку

    QUrlQuery query(decodedData);
    username = query.queryItemValue("username");
    password = query.queryItemValue("password");

    qDebug() << "Parsed username: " << username << ", password: " << password;
}


void MyTcpServer::handleRegister(const QString &data) {
    QString username, password;
    parseRegisterData(data, username, password);

    // Проверка на валидность имени пользователя
    if (!isValidUsername(username)) {
        qDebug() << "Invalid username format";
        return;
    }

    // Проверка на существование пользователя
    if (mUsers.contains(username)) {
        qDebug() << "Username already exists!";
        return;
    }

    // Хэширование пароля
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

    // Сохранение пользователя
    saveUserToFile(username, passwordHash.toHex());

    // Добавление пользователя в хэш таблицу
    mUsers.insert(username, passwordHash.toHex());

    qDebug() << "User registered: " << username;
}

bool MyTcpServer::isValidUsername(const QString &username) {
    return username.length() >= 3;
}

void MyTcpServer::saveUserToFile(const QString &username, const QString &passwordHash) {
    QFile file("users.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << username << " " << passwordHash << "\n";
        file.close();
    }
}
void MyTcpServer::slotClientDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        qDebug() << "Client disconnected!";
        clientSocket->deleteLater();
    }
}


