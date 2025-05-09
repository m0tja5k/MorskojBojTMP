#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>

///
/// \brief The MyTcpServer class
///
class MyTcpServer : public QObject {
    Q_OBJECT

public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    ///
    /// \brief slotNewConnection
    ///this is slot, which...
    void slotNewConnection();   // Обработчик нового соединения
    void slotServerRead();      // Обработчик чтения данных от клиента
    void slotClientDisconnected();  // Обработчик отключения клиента

private:
    QTcpServer *mTcpServer;
    QHash<QString, QString> mUsers;  // Хэш таблица для хранения пользователей
};

#endif // MYTCPSERVER_H
