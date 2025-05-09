#include "func2serv.h"
#include "DatabaseManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// Функция формирования JSON-ответа
QByteArray createJsonResponse(const QString &type, const QString &status, const QString &message) {
    QJsonObject jsonObj;
    jsonObj["type"] = type;
    jsonObj["status"] = status;
    jsonObj["message"] = message;
    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact) + "\r\n";
}

// Функция парсинга команд
QByteArray parse(const QString &input) {
    qDebug() << "Received input:" << input;
    // Парсим JSON
    QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format, input:" << input;
        return createJsonResponse("error", "error", "Invalid JSON format");
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("type")) {
        qDebug() << "Missing type field in JSON, input:" << input;
        return createJsonResponse("error", "error", "Missing type field");
    }

    QString type = jsonObj["type"].toString();
    qDebug() << "Parsed type:" << type;
    if (type == "register") {
        return handleRegister(input);
    } else if (type == "login") {
        return slotLogin(input);
    }

    qDebug() << "Unknown command type:" << type;
    return createJsonResponse("error", "error", "Unknown command");
}

bool parseRegisterData(const QString &data, QString &nickname, QString &email, QString &password) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format";
        return false;
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("nickname") || !jsonObj.contains("email") || !jsonObj.contains("password")) {
        qDebug() << "Missing required fields in JSON";
        return false;
    }

    nickname = jsonObj["nickname"].toString();
    email = jsonObj["email"].toString();
    password = jsonObj["password"].toString();

    if (nickname.isEmpty() || email.isEmpty() || password.isEmpty()) {
        qDebug() << "Nickname, email, or password is empty";
        return false;
    }

    qDebug() << "Parsed register data - Nickname:" << nickname << "Email:" << email << "Password:" << password;
    return true;
}

bool parseLoginData(const QString &data, QString &nickname, QString &password) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format";
        return false;
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("nickname") || !jsonObj.contains("password")) {
        qDebug() << "Missing required fields in JSON";
        return false;
    }

    nickname = jsonObj["nickname"].toString();
    password = jsonObj["password"].toString();

    if (nickname.isEmpty() || password.isEmpty()) {
        qDebug() << "Nickname or password is empty";
        return false;
    }

    qDebug() << "Parsed login data - Nickname:" << nickname << "Password:" << password;
    return true;
}

QByteArray handleRegister(const QString &data) {
    QString nickname, email, password;
    if (!parseRegisterData(data, nickname, email, password)) {
        return createJsonResponse("register", "error", "Invalid registration data");
    }

    DatabaseManager *db = DatabaseManager::getInstance();
    QSqlDatabase database = db->getDatabase();
    if (!database.isOpen()) {
        qDebug() << "Database is not open in handleRegister";
        return createJsonResponse("register", "error", "Database is not open");
    }

    QSqlQuery query(database);

    // Проверка, существует ли пользователь с таким nickname или email
    query.prepare("SELECT COUNT(*) FROM User WHERE nickname = :nickname OR email = :email");
    query.bindValue(":nickname", nickname);
    query.bindValue(":email", email);

    qDebug() << "Executing query in handleRegister: SELECT COUNT(*) FROM User WHERE nickname =" << nickname << "OR email =" << email;

    if (!query.exec()) {
        qDebug() << "Database query failed (SELECT) in handleRegister:" << query.lastError().text();
        return createJsonResponse("register", "error", "Database query failed");
    }

    query.next();
    if (query.value(0).toInt() > 0) {
        return createJsonResponse("register", "error", "User already exists");
    }

    // Добавление нового пользователя
    bool success = db->addUser(nickname, email, password);
    if (success) {
        return createJsonResponse("register", "success", "User registered successfully");
    } else {
        return createJsonResponse("register", "error", "Registration failed");
    }
}

QByteArray slotLogin(const QString &data) {
    QString nickname, password;
    if (!parseLoginData(data, nickname, password)) {
        return createJsonResponse("login", "error", "Invalid login data");
    }

    DatabaseManager *db = DatabaseManager::getInstance();
    QSqlDatabase database = db->getDatabase();
    if (!database.isOpen()) {
        qDebug() << "Database is not open in slotLogin";
        return createJsonResponse("login", "error", "Database is not open");
    }

    QSqlQuery query(database);

    query.prepare("SELECT * FROM User WHERE nickname = :nickname AND password = :password");
    query.bindValue(":nickname", nickname);
    query.bindValue(":password", password);

    qDebug() << "Executing query in slotLogin: SELECT * FROM User WHERE nickname =" << nickname << "AND password =" << password;

    if (!query.exec()) {
        qDebug() << "Database query failed (SELECT) in slotLogin:" << query.lastError().text();
        return createJsonResponse("login", "error", "Database query failed");
    }

    if (query.next()) {
        return createJsonResponse("login", "success", "Login successful");
    } else {
        return createJsonResponse("login", "error", "Invalid nickname or password");
    }
}
