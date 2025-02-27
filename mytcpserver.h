#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>

class MyTcpServer : public QObject {
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    void slotNewConnection();
    void slotServerRead();
    void slotClientDisconnected();
    void slotLogin(const QString &data);
private:
    QTcpServer *mTcpServer;
    QHash<QString, QString> mUsers;  // Хэш таблица для хранения пользователей
    bool isValidUsername(const QString &username);  // Проверка на валидность имени
    bool isPasswordCorrect(const QString &username, const QString &password); // Проверка на соответствие пароля
    void handleRegister(const QString &data);  // Регистрация пользователя
    void saveUserToFile(const QString &username, const QString &passwordHash);  // Сохранение пользователя
    void parseRegisterData(const QString &data, QString &username, QString &password);  // Парсинг данных
};

#endif // MYTCPSERVER_H
