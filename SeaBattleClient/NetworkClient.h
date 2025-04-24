//NetworkClient.h
#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <QDebug>

class NetworkClient : public QObject
{
    Q_OBJECT
public:
    static NetworkClient& instance();

    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 8888);
    void disconnectFromServer();
    void sendMessage(const QString& message);

    bool isConnected() const;

signals:
    void messageReceived(const QString&);
    void connectionChanged(bool); // Было connectionChanged → исправлено на connectionChanged
    void errorOccurred(const QString&);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    explicit NetworkClient(QObject* parent = nullptr);
    ~NetworkClient() = default;
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    QTcpSocket* m_socket;
    QMutex m_mutex;
    QTimer m_reconnectTimer;
};

#endif // NETWORKCLIENT_H
