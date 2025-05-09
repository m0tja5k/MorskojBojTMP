#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject> // Обязательно включаем QObject
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* getInstance();
    QSqlDatabase getDatabase();
    bool addUser(const QString &login, const QString &password);
    void printUsers();

private:
    DatabaseManager();  // Закрытый конструктор
    virtual ~DatabaseManager(); // Деструктор
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager* instance;
    QSqlDatabase db;
};

#endif
