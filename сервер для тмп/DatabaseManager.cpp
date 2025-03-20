#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("server_db.sqlite");

    if (!db.open()) {
        qDebug() << "Error opening DB:" << db.lastError().text();
    } else {
        qDebug() << "Database connected!";
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS User ("
                   "login TEXT PRIMARY KEY, "
                   "password TEXT NOT NULL)");
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

bool DatabaseManager::addUser(const QString &login, const QString &password) {
    QSqlQuery query;
    query.prepare("INSERT INTO User (login, password) VALUES (:login, :password)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);

    if (!query.exec()) {
        qDebug() << "Error adding user:" << query.lastError().text();
        return false;
    }
    return true;
}
