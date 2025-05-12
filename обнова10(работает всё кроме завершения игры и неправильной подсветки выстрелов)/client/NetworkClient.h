#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QQueue>

struct Ship {
    int gameId;
    int x;
    int y;
    int size;
    bool isHorizontal;
};

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    static NetworkClient& instance();

    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 33333);
    void disconnectFromServer();
    void sendMessage(const QString& message);

    bool isConnected() const;

    void registerUser(const QString &nickname, const QString &email, const QString &password);
    void loginUser(const QString &nickname, const QString &password);
    void requestStartGame();
    void placeShip(int gameId, int x, int y, int size, bool isHorizontal);
    void queueShip(int gameId, int x, int y, int size, bool isHorizontal);
    void readyToBattle(int gameId);
    void sendMove(int gameId, int x, int y);
    void setCurrentNickname(const QString& nickname);
    int getGameId() const { return currentGameId; }
    QString getCurrentNickname() const { return currentNickname; }

signals:
    void connectionChanged(bool connected);
    void errorOccurred(const QString &error);
    void messageReceived(const QString &message);

    void registrationSuccess();
    void registrationFailed(const QString &reason);
    void loginSuccess(const QString &nickname);
    void loginFailed(const QString &reason);
    void startGameWaiting();
    void gameReady(int gameId, const QString &opponent);
    void shipPlacedSuccessfully();
    void shipPlacementFailed(const QString &reason);
    void allShipsPlaced();
    void readyToBattleConfirmed();
    void readyToBattleFailed(const QString &reason);
    void gameStarted(const QString &tTurn);
    void moveResult(const QString &status, int x, int y, const QString &message);
    void gameOver(const QString &message);
    void updateUIEnabled(bool enabled);
    void updateOpponentField(int x, int y, QString status);

public slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);
    void sendNextShip();

private:
    NetworkClient(QObject* parent = nullptr);
    ~NetworkClient() = default;
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    QTcpSocket* m_socket;
    QMutex m_mutex;
    QTimer m_reconnectTimer;
    QString currentNickname;
    int currentGameId = -1;
    QQueue<Ship> shipQueue;
};

#endif // NETWORKCLIENT_H
