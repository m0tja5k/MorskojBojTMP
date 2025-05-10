#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager() {
    // Проверка доступности драйвера SQLite
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "Error: SQLite driver not available!";
    } else {
        qDebug() << "SQLite driver is available.";
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("server_db.sqlite");
    qDebug() << "Attempting to open database at:" << db.databaseName();

    // Удаляем старую базу данных (для теста, можно убрать после отладки)
    if (QFile::exists("server_db.sqlite")) {
        qDebug() << "Removing existing database file.";
        QFile::remove("server_db.sqlite");
    }

    if (!db.open()) {
        qDebug() << "Error opening DB:" << db.lastError().text();
    } else {
        qDebug() << "Database connected successfully!";
        QSqlQuery query(db);
        bool success = query.exec("CREATE TABLE IF NOT EXISTS User ("
                                  "nickname TEXT PRIMARY KEY, "
                                  "email TEXT NOT NULL UNIQUE, "
                                  "password TEXT NOT NULL)");
        if (!success) {
            qDebug() << "Error creating table User:" << query.lastError().text();
        } else {
            qDebug() << "Table User created or already exists.";
        }
    }
}

QSqlDatabase DatabaseManager::getDatabase() {
    return db;
}

DatabaseManager::~DatabaseManager() {
    db.close();
}

DatabaseManager* DatabaseManager::getInstance() {
    if (!instance) {
        instance = new DatabaseManager();
    }
    return instance;
}

bool DatabaseManager::addUser(const QString &nickname, const QString &email, const QString &password) {
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password) VALUES (:nickname, :email, :password)");
    query.bindValue(":nickname", nickname);
    query.bindValue(":email", email);
    query.bindValue(":password", password);

    qDebug() << "Adding user - Nickname:" << nickname << "Email:" << email;

    if (!query.exec()) {
        qDebug() << "Error adding user:" << query.lastError().text();
        return false;
    }
    qDebug() << "User added successfully.";
    return true;
}

void DatabaseManager::printUsers() {
    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM User")) {
        qDebug() << "Error fetching users:" << query.lastError().text();
        return;
    }
    while (query.next()) {
        qDebug() << "Nickname:" << query.value("nickname").toString()
        << "Email:" << query.value("email").toString()
        << "Password:" << query.value("password").toString();
    }
}
