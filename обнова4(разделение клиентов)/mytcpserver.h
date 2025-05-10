#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QMutex>
#include <QVector>

class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

    // Методы для управления клиентами
    void sendMessageToUser(const QString &nickname, const QByteArray &message);
    void registerClient(const QString &nickname, QTcpSocket *socket);
    void unregisterClient(QTcpSocket *socket);
    QString getNicknameBySocket(QTcpSocket *socket); // Уже исправлено ранее

    // Методы для игровой логики
    void addPlayerToGame(const QString &nickname);
    QString getOpponent(const QString &nickname) const;
    QString getOpponentInternal(const QString &nickname); // Новый неконстантный метод
    int getPlayerCount() const;
    int getPlayerCountInternal(); // Новый неконстантный метод
    void resetGame();

public slots:
    void slotNewConnection();
    void slotServerRead();
    void slotClientDisconnected();

private:
    QTcpServer *mTcpServer;
    QHash<QString, QTcpSocket*> mClients; // Никнейм -> Сокет
    QHash<QTcpSocket*, QString> mSocketToNickname; // Сокет -> Никнейм (для обратного поиска)
    QStringList players; // Список игроков в игре
    QMutex mutex; // Для защиты доступа к общим данным
};

#endif // MYTCPSERVER_H
