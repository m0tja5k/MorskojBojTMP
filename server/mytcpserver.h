/**
 * @file mytcpserver.h
 * @brief Заголовочный файл для класса MyTcpServer, реализующего TCP-сервер.
 *
 *  Содержит объявления класса MyTcpServer, который управляет сетевыми подключениями,
 *  обработкой сообщений, игровой логикой и взаимодействием с клиентами.
 */
#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHash>
#include <QMutex>
#include <QVector>  //  Вектор игроков (не используется, но оставлен)
#include <QSet>   // Множество игроков, готовых к бою

/**
 * @brief Класс MyTcpServer, реализующий TCP-сервер.
 *
 *  Отвечает за обработку сетевых подключений, обмен сообщениями с клиентами,
 *  игровую логику и управление игроками.
 */
class MyTcpServer : public QObject
{
    Q_OBJECT  // Макрос для поддержки сигналов и слотов Qt.

public:
    /**
     * @brief Конструктор класса MyTcpServer.
     *
     * @param parent Указатель на родительский объект.
     */
    explicit MyTcpServer(QObject *parent = nullptr);
    /**
     * @brief Деструктор класса MyTcpServer.
     */
    ~MyTcpServer();

    // Методы для управления клиентами
    /**
     * @brief Отправляет сообщение конкретному пользователю.
     *
     * @param nickname Никнейм получателя.
     * @param message Сообщение для отправки.
     */
    void sendMessageToUser(const QString &nickname, const QByteArray &message);
    /**
     * @brief Регистрирует нового клиента.
     *
     * @param nickname Никнейм клиента.
     * @param socket Указатель на сокет клиента.
     */
    void registerClient(const QString &nickname, QTcpSocket *socket);
    /**
     * @brief Удаляет зарегистрированного клиента.
     *
     * @param socket Указатель на сокет клиента.
     */
    void unregisterClient(QTcpSocket *socket);
    /**
     * @brief Получает никнейм пользователя по сокету.
     *
     * @param socket Указатель на сокет.
     * @return QString Никнейм пользователя, или "" если не найден.
     */
    QString getNicknameBySocket(QTcpSocket *socket);

    // Методы для игровой логики
    /**
     * @brief Добавляет игрока в текущую игру.
     *
     * @param nickname Никнейм игрока.
     */
    void addPlayerToGame(const QString &nickname);
    /**
     * @brief Получает никнейм оппонента. (Константная версия)
     *
     * @param nickname Никнейм текущего игрока.
     * @return QString Никнейм оппонента, или "" если оппонент не найден.
     */
    QString getOpponent(const QString &nickname) const;
    /**
     * @brief Получает никнейм оппонента (внутренняя, не константная версия).
     *
     * @param nickname Никнейм текущего игрока.
     * @return QString Никнейм оппонента, или "" если оппонент не найден.
     */
    QString getOpponentInternal(const QString &nickname);
    /**
     * @brief Получает количество игроков в текущей игре. (Константная версия)
     *
     * @return int Количество игроков.
     */
    int getPlayerCount() const;
    /**
     * @brief Получает количество игроков в текущей игре (внутренняя, не константная версия).
     *
     * @return int Количество игроков.
     */
    int getPlayerCountInternal();
    /**
     * @brief Сбрасывает состояние игры.
     */
    void resetGame();
    /**
     * @brief Получает ID текущей игры.
     *
     * @return int ID текущей игры.
     */
    int getGameId() const;

    int currentGameId;  // ID текущей игры

public slots:
    /**
     * @brief Слот для обработки новых подключений клиентов.
     */
    void slotNewConnection();
    /**
     * @brief Слот для обработки данных, полученных от клиента.
     */
    void slotServerRead();
    /**
     * @brief Слот для обработки отключения клиента.
     */
    void slotClientDisconnected();

private:
    QTcpServer *mTcpServer;  // Указатель на QTcpServer.
    QHash<QString, QTcpSocket*> mClients;  //  Словарь: Никнейм -> Сокет.
    QHash<QTcpSocket*, QString> mSocketToNickname;  //  Словарь: Сокет -> Никнейм (для обратного поиска).
    QStringList players;  // Список игроков в текущей игре.
    QMutex mutex;  // Мьютекс для защиты доступа к общим данным.

    QSet<QString> readyPlayers;  // Множество никнеймов игроков, готовых к игре.
};

#endif // MYTCPSERVER_H