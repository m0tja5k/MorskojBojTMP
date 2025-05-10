#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
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
    bool addUser(const QString &nickname, const QString &email, const QString &password);
    void printUsers();

private:
    DatabaseManager();
    virtual ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    static DatabaseManager* instance;
    QSqlDatabase db;
};

#endif
