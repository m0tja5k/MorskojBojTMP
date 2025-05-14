#include "mytcpserver.h"
#include "func2serv.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket> // Включено для работы с QTcpSocket
#include <QHostAddress> // Включено для работы с QHostAddress
#include <QMutexLocker> // Включено для работы с QMutexLocker

/**
 * @brief Конструктор класса MyTcpServer.
 *
 * Инициализирует TCP сервер, устанавливает соединение для обработки новых подключений
 * и запускает прослушивание входящих соединений на порту 33333.
 *
 * @param parent Указатель на родительский объект.
 */
MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent), currentGameId(-1)  // Инициализация членов класса.
{
    mTcpServer = new QTcpServer(this);  // Создаем экземпляр TCP сервера.
    connect(mTcpServer, &QTcpServer::newConnection, this, &MyTcpServer::slotNewConnection);  // Подключаем сигнал newConnection к слоту slotNewConnection.

    // Пытаемся запустить прослушивание входящих соединений на порту 33333.
    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "Server is NOT started!";  // Выводим сообщение об ошибке, если не удалось запустить сервер.
    } else {
        qDebug() << "Server is started!";  // Выводим сообщение об успешном запуске сервера.
    }
}

/**
 * @brief Деструктор класса MyTcpServer.
 *
 * Закрывает TCP сервер.
 */
MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();  // Закрываем TCP сервер.
}

/**
 * @brief Слот для обработки новых подключений клиентов.
 *
 * Принимает входящее соединение, проверяет количество подключенных игроков,
 * и, если лимит не превышен, устанавливает соединения для обработки данных
 * и отключения клиента.
 */
void MyTcpServer::slotNewConnection()
{
    QTcpSocket *clientSocket = mTcpServer->nextPendingConnection();  // Получаем сокет для нового подключения.
    if (clientSocket) {
        // Если количество игроков достигло максимума (2), отклоняем новое подключение.
        if (getPlayerCount() >= 2) {
            QByteArray response = createJsonResponse("error", "error", "Server is full");  // Создаем JSON ответ об ошибке.
            clientSocket->write(response);  // Отправляем ответ клиенту.
            clientSocket->flush();  // Отправляем данные немедленно.
            clientSocket->disconnectFromHost();  // Отключаем клиента.
            return;  // Завершаем обработку.
        }

        // Подключаем сигналы сокета к соответствующим слотам для обработки данных и отключения.
        connect(clientSocket, &QTcpSocket::readyRead, this, &MyTcpServer::slotServerRead);  // Подключаем сигнал readyRead к слоту slotServerRead.
        connect(clientSocket, &QTcpSocket::disconnected, this, &MyTcpServer::slotClientDisconnected);  // Подключаем сигнал disconnected к слоту slotClientDisconnected.
        qDebug() << "New client connected from" << clientSocket->peerAddress().toString();  // Выводим сообщение о новом подключении.
    }
}

/**
 * @brief Слот для обработки данных, полученных от клиента.
 *
 * Читает данные из сокета клиента, парсит JSON, и вызывает соответствующие
 * функции для обработки запроса.
 */
void MyTcpServer::slotServerRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());  // Получаем указатель на сокет клиента.
    if (!clientSocket) {
        qDebug() << "Invalid client socket in slotServerRead";  // Выводим сообщение об ошибке, если сокет недействителен.
        return;  // Завершаем обработку.
    }

    QByteArray requestData = clientSocket->readAll();  // Читаем все данные из сокета.
    QString request = QString::fromUtf8(requestData).trimmed();  // Преобразуем данные в строку и убираем пробелы в начале и конце.

    qDebug() << "Received raw request:" << requestData.toHex();  // Выводим в лог полученные данные в шестнадцатеричном формате.
    qDebug() << "Received input (parsed):" << request;  // Выводим в лог полученные данные в текстовом формате.
    QByteArray response;  // Переменная для хранения ответа сервера.

    QJsonDocument doc = QJsonDocument::fromJson(request.toUtf8());  // Пытаемся преобразовать строку в JSON документ.
    if (doc.isObject()) {
        QJsonObject jsonObj = doc.object();  // Получаем JSON объект из документа.
        QString type = jsonObj["type"].toString();  // Получаем тип запроса.
        QString nickname = jsonObj["nickname"].toString();  // Получаем никнейм клиента.

        // Если тип запроса "register" или "login", регистрируем клиента.
        if (type == "register" || type == "login") {
            if (!nickname.isEmpty()) {
                registerClient(nickname, clientSocket);  // Регистрируем клиента.
                response = parse(request, this);  // Обрабатываем запрос.
                qDebug() << "Clients registered:" << mClients.keys();  // Выводим список зарегистрированных клиентов.
            } else {
                response = createJsonResponse("error", "error", "Nickname is empty");  // Создаем JSON ответ об ошибке.
            }
        }
        // Если тип запроса "ready_to_battle", устанавливаем флаг готовности игрока.
        else if (type == "ready_to_battle") {
            if (!nickname.isEmpty() && players.contains(nickname)) {
                QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасного доступа к данным.
                qDebug() << "Processing ready_to_battle for" << nickname << "- currentGameId:" << currentGameId << "- Socket state:" << clientSocket->state();
                readyPlayers.insert(nickname);  // Добавляем игрока в список готовых игроков.

                qDebug() << "Player" << nickname << "is ready. Ready players:" << readyPlayers;
                response = createJsonResponse("ready_to_battle", "success", "Ready status received");  // Создаем JSON ответ об успехе.
                // Если готово два игрока и игра уже создана.
                if (readyPlayers.size() == 2 && currentGameId != -1) {
                    qDebug() << "Both players ready, starting game with gameId:" << currentGameId;
                    DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.
                    QString player1 = players[0];  // Получаем имя первого игрока (предполагаем, что порядок игроков важен).
                    db->updateTurn(currentGameId, player1);  // Устанавливаем ход первого игрока в базе данных.

                    QJsonObject startMsg;  // Создаем JSON объект для сообщения о начале игры.
                    startMsg["type"] = "game_start";  // Устанавливаем тип сообщения.
                    startMsg["status"] = "success";  // Устанавливаем статус сообщения.
                    startMsg["message"] = "Game started";  // Устанавливаем сообщение.
                    startMsg["current_turn"] = player1;  // Устанавливаем текущий ход (первый игрок).
                    QByteArray startResponse = QJsonDocument(startMsg).toJson(QJsonDocument::Compact) + "\r\n";  // Преобразуем JSON объект в QByteArray.
                    qDebug() << "Prepared game_start message:" << startResponse;

                    // Отправляем сообщение о начале игры всем клиентам.
                    for (const QString &player : mClients.keys()) {
                        QTcpSocket *targetSocket = mClients[player];  // Получаем сокет игрока.
                        qDebug() << "Attempting to send to" << player << "- Socket state:" << targetSocket->state();
                        if (targetSocket->state() == QAbstractSocket::ConnectedState) {  // Проверяем состояние сокета.
                            bool success = targetSocket->write(startResponse);  // Отправляем сообщение.
                            targetSocket->flush();  // Отправляем данные немедленно.
                            if (success) {
                                qDebug() << "Successfully sent game_start to" << player;  // Логируем успешную отправку.
                            } else {
                                qDebug() << "Failed to send game_start to" << player << "- Error:" << targetSocket->errorString();  // Логируем ошибку отправки.
                            }
                        } else {
                            qDebug() << "Cannot send to" << player << "- Socket not connected, state:" << targetSocket->state();  // Логируем, если сокет не подключен.
                        }
                    }
                }
            } else {
                response = createJsonResponse("error", "error", "Player not registered");  // Если игрок не зарегистрирован.
            }
        }
        // Обрабатываем команду "make_move".
        else if (type == "make_move") {
            int gameId = jsonObj["game_id"].toInt();  // Получаем ID игры.
            int x = jsonObj["x"].toInt();  // Получаем координату X.
            int y = jsonObj["y"].toInt();  // Получаем координату Y.
            qDebug() << "Processing make_move for" << nickname << "in game" << gameId << "at (" << x << "," << y << ")";

            DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.
            QString currentTurn = db->getCurrentTurn(gameId);  // Получаем текущий ход игры.
            qDebug() << "Current turn for game" << gameId << "is" << currentTurn;  // Выводим в лог текущий ход.

            // Проверяем, является ли текущий игрок тем, кто пытается сделать ход.
            if (currentTurn != nickname) {
                response = createJsonResponse("error", "error", "Not your turn");  // Создаем JSON ответ об ошибке.
                qDebug() << "Move rejected: not" << nickname << "'s turn, current turn is" << currentTurn;  // Выводим в лог информацию об ошибке.
            } else {
                // Если ход игрока правильный, проверяем результат выстрела.
                QString result = db->checkMove(gameId, nickname, x, y);
                qDebug() << "Move result for" << nickname << ":" << result;  // Выводим в лог результат выстрела.

                // Обрабатываем различные результаты выстрела.
                if (result == "error") {
                    response = createJsonResponse("error", "error", "Failed to process move");  // Создаем JSON ответ об ошибке.
                    qDebug() << "Move processing failed for" << nickname;  // Выводим в лог информацию об ошибке.
                } else if (result == "already_shot") {
                    response = createJsonResponse("error", "error", "Cell already shot");  // Создаем JSON ответ об ошибке.
                    qDebug() << "Move rejected: cell (" << x << "," << y << ") already shot by" << nickname;  // Выводим в лог информацию об ошибке.
                } else {
                    // Если ход успешен, формируем ответы для текущего игрока и оппонента.
                    QJsonObject moveResponse;  // Создаем JSON объект для ответа текущему игроку.
                    moveResponse["type"] = "make_move";  // Устанавливаем тип ответа.
                    moveResponse["status"] = result;  // Устанавливаем статус ответа.
                    moveResponse["message"] = "Move processed";  // Устанавливаем сообщение.
                    moveResponse["x"] = x;  // Устанавливаем координату X.
                    moveResponse["y"] = y;  // Устанавливаем координату Y.

                    QJsonObject opponentResponse;  // Создаем JSON объект для ответа оппоненту.
                    opponentResponse["type"] = "move_result";  // Устанавливаем тип ответа.
                    opponentResponse["status"] = result;  // Устанавливаем статус ответа.
                    opponentResponse["x"] = x;  // Устанавливаем координату X.
                    opponentResponse["y"] = y;  // Устанавливаем координату Y.
                    opponentResponse["message"] = "Opponent made a move";  // Устанавливаем сообщение.

                    // Обновляем current_turn только один раз
                    QString opponent = getOpponent(nickname);  // Получаем никнейм оппонента.
                    if (!opponent.isEmpty()) {  // Проверяем, найден ли оппонент.
                        // Если промах или потопил, передаем ход оппоненту.
                        if (result != "hit" && result != "sunk") {
                            db->updateTurn(gameId, opponent);  // Обновляем текущий ход в базе данных.
                            qDebug() << "Turn updated to" << opponent << "for game" << gameId;  // Выводим в лог информацию об обновлении хода.
                        }
                        moveResponse["current_turn"] = (result != "hit" && result != "sunk") ? opponent : currentTurn; // Устанавливаем текущий ход в ответе.
                        opponentResponse["current_turn"] = (result != "hit" && result != "sunk") ? opponent : currentTurn; // Устанавливаем текущий ход в ответе.
                    } else {
                        qDebug() << "Opponent not found for" << nickname;  // Логируем, если оппонент не найден.
                        moveResponse["current_turn"] = currentTurn; // Устанавливаем текущий ход в ответе.
                        opponentResponse["current_turn"] = currentTurn; // Устанавливаем текущий ход в ответе.
                    }

                    // Отправляем ответы
                    response = QJsonDocument(moveResponse).toJson(QJsonDocument::Compact) + "\r\n";  // Преобразуем ответ текущему игроку в QByteArray.
                    qDebug() << "Prepared response for" << nickname << ":" << response;  // Логируем подготовленный ответ.

                    if (!opponent.isEmpty()) {  // Проверяем, найден ли оппонент.
                        QByteArray opponentMessage = QJsonDocument(opponentResponse).toJson(QJsonDocument::Compact) + "\r\n";  // Преобразуем ответ оппоненту в QByteArray.
                        qDebug() << "Sending move_result to" << opponent << ":" << opponentMessage;  // Логируем отправку сообщения оппоненту.
                        sendMessageToUser(opponent, opponentMessage);  // Отправляем сообщение оппоненту.
                    } else {
                        qDebug() << "Opponent not found for" << nickname << "in game" << gameId;  // Логируем, если оппонент не найден.
                    }
                }
            }
        }
        // Обрабатываем другие типы запросов (если они есть).
        else {
            response = parse(request, this);  // Вызываем функцию parse для обработки запроса.
            qDebug() << "Processed other request type:" << type << ", response:" << response;  // Логируем обработку других типов запросов.
        }
    }
    // Если не удалось распарсить JSON.
    else {
        qDebug() << "Failed to parse JSON for request:" << request;  // Логируем ошибку парсинга.
        response = createJsonResponse("error", "error", "Invalid JSON format");  // Создаем JSON ответ об ошибке.
    }

    // Отправляем ответ клиенту, если сокет подключен.
    if (clientSocket->state() == QAbstractSocket::ConnectedState) {  // Проверяем состояние сокета.
        qDebug() << "Sending response to" << getNicknameBySocket(clientSocket) << ". Response:" << response;  // Логируем отправку ответа.
        clientSocket->write(response);  // Отправляем ответ клиенту.
        clientSocket->flush();  // Отправляем данные немедленно.
    } else {
        qDebug() << "Cannot send response to" << getNicknameBySocket(clientSocket) << ", socket state:" << clientSocket->state();  // Логируем, если не удалось отправить ответ.
    }
}

/**
 * @brief Слот для обработки отключения клиента.
 *
 *  Вызывается при отключении клиента от сервера. Удаляет клиента из списков,
 *  очищает ресурсы.
 */
void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());  // Получаем указатель на сокет клиента.
    if (clientSocket) {  // Проверяем, что сокет существует.
        QString nickname = getNicknameBySocket(clientSocket);  // Получаем никнейм клиента.
        if (!nickname.isEmpty()) {  // Проверяем, что никнейм не пустой.
            QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
            readyPlayers.remove(nickname);  // Удаляем игрока из списка готовых.
            unregisterClient(clientSocket);  // Удаляем клиента из списка клиентов.
            qDebug() << "Client" << nickname << "disconnected! Socket state:" << clientSocket->state();  // Выводим сообщение об отключении клиента.
        }
        clientSocket->deleteLater();  // Удаляем сокет.
    }
}

/**
 * @brief Отправляет сообщение конкретному пользователю.
 *
 *  Находит сокет пользователя по никнейму и отправляет ему сообщение.
 *
 * @param nickname Никнейм пользователя.
 * @param message Сообщение для отправки.
 */
void MyTcpServer::sendMessageToUser(const QString &nickname, const QByteArray &message)
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасного доступа к данным.
    if (mClients.contains(nickname)) {  // Проверяем, что пользователь существует.
        QTcpSocket *socket = mClients[nickname];  // Получаем сокет пользователя.
        if (socket->state() == QAbstractSocket::ConnectedState) {  // Проверяем состояние сокета.
            socket->write(message);  // Отправляем сообщение.
            socket->flush();  // Отправляем данные немедленно.
            qDebug() << "Sent message to" << nickname << ":" << message;  // Логируем отправку сообщения.
        } else {
            qDebug() << "Socket for" << nickname << "is not connected. State:" << socket->state();  // Логируем, если сокет не подключен.
        }
    } else {
        qDebug() << "User" << nickname << "not found or not connected. Current clients:" << mClients.keys();  // Логируем, если пользователь не найден.
    }
}

void MyTcpServer::registerClient(const QString &nickname, QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    mClients.insert(nickname, socket);  // Добавляем клиента в map mClients (никнейм -> сокет).
    mSocketToNickname.insert(socket, nickname);  // Добавляем клиента в map mSocketToNickname (сокет -> никнейм).
    qDebug() << "Registered client:" << nickname << "Socket state:" << socket->state();  // Логируем регистрацию клиента.
}

/**
 * @brief Удаляет клиента из списков зарегистрированных пользователей.
 *
 *  Удаляет клиента из списков mClients, mSocketToNickname, а также из списков
 *  игр, если клиент в них участвовал.  Отправляет сообщение о завершении игры
 *  оппоненту, если клиент был в игре.
 *
 * @param socket Указатель на сокет клиента.
 */
void MyTcpServer::unregisterClient(QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    QString nickname = mSocketToNickname.value(socket, "");  // Получаем никнейм клиента.
    if (!nickname.isEmpty()) {  // Проверяем, что никнейм не пустой.
        mClients.remove(nickname);  // Удаляем клиента из списка клиентов.
        mSocketToNickname.remove(socket);  // Удаляем сокет из списка сокетов.
        players.removeAll(nickname);  // Удаляем игрока из списка игроков текущей игры.
        readyPlayers.remove(nickname);  // Удаляем игрока из списка готовых.
        QString opponent = getOpponentInternal(nickname);  // Получаем никнейм оппонента.
        if (!opponent.isEmpty()) {  // Проверяем, что у клиента был оппонент.
            sendMessageToUser(opponent, createJsonResponse("gameover", "opponent_disconnected", "Opponent disconnected"));  // Отправляем сообщение оппоненту об отключении.
            resetGame();  // Сбрасываем состояние игры.
        }
    }
}

/**
 * @brief Получает никнейм пользователя по сокету.
 *
 * @param socket Указатель на сокет.
 * @return QString Никнейм пользователя, связанного с данным сокетом, или "" если не найден.
 */
QString MyTcpServer::getNicknameBySocket(QTcpSocket *socket)
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    return mSocketToNickname.value(socket, "");  // Возвращаем никнейм.
}

/**
 * @brief Добавляет игрока в список игроков текущей игры.
 *
 * @param nickname Никнейм игрока.
 */
void MyTcpServer::addPlayerToGame(const QString &nickname)
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    if (!players.contains(nickname)) {  // Проверяем, что игрок еще не добавлен.
        players.append(nickname);  // Добавляем игрока в список.
        qDebug() << "Added player to game:" << nickname;  // Логируем добавление игрока.
    }
}

/**
 * @brief Получает никнейм оппонента.  (Константная версия)
 *
 * @param nickname Никнейм текущего игрока.
 * @return QString Никнейм оппонента, или "" если оппонент не найден.
 */
QString MyTcpServer::getOpponent(const QString &nickname) const
{
    return const_cast<MyTcpServer*>(this)->getOpponentInternal(nickname);  // Вызываем внутреннюю, не константную версию.
}

/**
 * @brief Получает никнейм оппонента (внутренняя, не константная версия).
 *
 *  Используется для поиска оппонента среди игроков текущей игры.
 *
 * @param nickname Никнейм текущего игрока.
 * @return QString Никнейм оппонента, или "" если оппонент не найден.
 */
QString MyTcpServer::getOpponentInternal(const QString &nickname)
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    for (const QString &player : players) {  // Перебираем список игроков.
        if (player != nickname) {  // Если текущий игрок не является переданным никнеймом.
            return player;  // Возвращаем никнейм оппонента.
        }
    }
    return "";  // Возвращаем пустую строку, если оппонент не найден.
}

/**
 * @brief Получает количество игроков в текущей игре. (Константная версия)
 *
 * @return int Количество игроков.
 */
int MyTcpServer::getPlayerCount() const
{
    return const_cast<MyTcpServer*>(this)->getPlayerCountInternal();  // Вызываем внутреннюю, не константную версию.
}

/**
 * @brief Получает количество игроков в текущей игре (внутренняя, не константная версия).
 *
 * @return int Количество игроков.
 */
int MyTcpServer::getPlayerCountInternal()
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    return players.size();  // Возвращаем количество игроков.
}

/**
 * @brief Сбрасывает состояние игры.
 *
 *  Очищает списки игроков и готовности, а также сбрасывает ID текущей игры.
 */
void MyTcpServer::resetGame()
{
    QMutexLocker locker(&mutex);  // Блокируем мьютекс для потокобезопасности.
    players.clear();  // Очищаем список игроков.
    readyPlayers.clear();  // Очищаем список готовых игроков.
    currentGameId = -1;  // Сбрасываем ID текущей игры.
    qDebug() << "Game reset.";  // Логируем сброс игры.
}

/**
 * @brief Получает ID текущей игры.
 *
 * @return int ID текущей игры.
 */
int MyTcpServer::getGameId() const
{
    return currentGameId;  // Возвращаем ID текущей игры.
}


























