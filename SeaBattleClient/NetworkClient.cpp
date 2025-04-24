//NetworkClient.cpp
#include "NetworkClient.h"

NetworkClient& NetworkClient::instance()
{
    static NetworkClient instance;
    return instance;
}

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    // Настройка соединений сигналов
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onError);

    // Настройка таймера переподключения
    connect(&m_reconnectTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "Attempting to reconnect...";
        connectToServer();
    });
    m_reconnectTimer.setInterval(5000);
}

void NetworkClient::connectToServer(const QString& host, quint16 port) {
    QMutexLocker locker(&m_mutex);
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort(); // Сбросить состояние перед подключением
    }
    qDebug() << "Connecting to" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void NetworkClient::disconnectFromServer() {
    QMutexLocker locker(&m_mutex);
    m_reconnectTimer.stop(); // Остановить таймер
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->abort(); // Принудительно закрыть сокет
    }
    qDebug() << "-----------------------";
}

void NetworkClient::sendMessage(const QString& message)
{
    QMutexLocker locker(&m_mutex);
    if(m_socket->state() == QAbstractSocket::ConnectedState)
    {
        m_socket->write(message.toUtf8());
        m_socket->flush();
    }
}

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

// Слоты обработки событий
void NetworkClient::onConnected()
{
    qDebug() << "Connected to server!";
    m_reconnectTimer.stop();
    emit connectionChanged(true);
}

void NetworkClient::onDisconnected() {
    qDebug() << "Disconnected from server!";
    // Убрать m_reconnectTimer.start();
    emit connectionChanged(false);
}

void NetworkClient::onReadyRead()
{
    const QByteArray data = m_socket->readAll();
    emit messageReceived(QString::fromUtf8(data));
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    emit errorOccurred(m_socket->errorString());
    disconnectFromServer();
}
