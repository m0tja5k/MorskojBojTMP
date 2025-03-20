#include "func2serv.h"
#include "DatabaseManager.h"
#include <QCryptographicHash>
#include <QUrlQuery>
#include <QDebug>

// Функция парсинга команд
QByteArray parse(const QString &input) {
    // Убираем лишние пробелы
    QString trimmedInput = input.trimmed();

    if (trimmedInput.startsWith("/register")) {
        // Убираем "/register " и передаем оставшуюся часть запроса
        return handleRegister(trimmedInput.mid(9).trimmed());
    } else if (trimmedInput.startsWith("/login")) {
        return slotLogin(trimmedInput.mid(6).trimmed());
    }

    // Если команда не распознана
    return QByteArray("Unknown command\r\n");
}


QByteArray handleRegister(const QString &data) {
    QString username, password;
    if (!parseRegisterData(data, username, password)) {
        return QByteArray("Error: Invalid registration data\r\n");
    }

    DatabaseManager *db = DatabaseManager::getInstance();
    QSqlDatabase database = db->getDatabase();
    QSqlQuery query(database);

    // Проверка, существует ли уже пользователь с таким логином
    query.prepare("SELECT COUNT(*) FROM User WHERE login = :login");
    query.bindValue(":login", username);

    if (!query.exec()) {
        return QByteArray("Error: Database query failed\r\n");
    }

    query.next();
    if (query.value(0).toInt() > 0) {
        return QByteArray("Error: User already exists\r\n");
    }

    // Добавление нового пользователя
    bool success = db->addUser(username, password);
    if (success) {
        return QByteArray("Registration successful\r\n");
    } else {
        return QByteArray("Error: Registration failed\r\n");
    }
}


QByteArray slotLogin(const QString &data) {
    QString username, password;
    if (!parseRegisterData(data, username, password)) {
        return QByteArray("Error: Invalid login data\r\n");
    }

    DatabaseManager *db = DatabaseManager::getInstance();
    QSqlDatabase database = db->getDatabase();  // Получаем базу данных
    QSqlQuery query(database);  // Создаем запрос

    query.prepare("SELECT * FROM User WHERE login = :login AND password = :password");
    query.bindValue(":login", username);
    query.bindValue(":password", password);

    if (!query.exec()) {
        return QByteArray("Error: Database query failed\r\n");
    }

    if (query.next()) {
        return QByteArray("Login successful\r\n");
    } else {
        return QByteArray("Error: Invalid username or password\r\n");
    }
}
bool parseRegisterData(const QString &data, QString &username, QString &password) {
    // Преобразуем строку запроса в формат URL-encoded
    QUrlQuery query(QUrl::fromPercentEncoding(data.toUtf8()));
    username = query.queryItemValue("username");
    password = query.queryItemValue("password");

    // Если параметры пустые, возвращаем false
    return !(username.isEmpty() || password.isEmpty());
}

