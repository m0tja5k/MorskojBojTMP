#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

class NetworkClient : public QObject
{
    Q_OBJECT
public:
    void setCurrentNickname(const QString& nickname);
    static NetworkClient& instance();

    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 33333);
    void disconnectFromServer();
    void sendMessage(const QString& message);

    bool isConnected() const;

    void registerUser(const QString &nickname, const QString &email,
                      const QString &password);
    void loginUser(const QString &email, const QString &password);

signals:
    void messageReceived(const QString&);
    void connectionChanged(bool);
    void errorOccurred(const QString&);
    void registrationSuccess();
    void registrationFailed(const QString &reason);
    void loginSuccess(const QString &nickname);
    void loginFailed(const QString &reason);
    void gameReady();

public slots:  // Изменено с protected на public
    void requestStartGame();

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
    QString currentNickname;  // Добавьте это поле
};

#endif // NETWORKCLIENT_H
