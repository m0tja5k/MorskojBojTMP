#include "func2serv.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include "DatabaseManager.h"

QByteArray parse(const QString &input, MyTcpServer *server)
{
    QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());
    QJsonObject response;

    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "Invalid JSON format, input:" << input;
        return createJsonResponse("error", "error", "Invalid JSON format");
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("type")) {
        qDebug() << "Missing type field in JSON, input:" << input;
        return createJsonResponse("error", "error", "Missing type field");
    }

    QString type = obj["type"].toString();
    qDebug() << "Parsed type:" << type;

    if (type == "register") {
        return handleRegister(input);
    }
    else if (type == "login") {
        return slotLogin(input);
    }
    else if (type == "start_game") {
        return handleStartGame(input, server);
    }
    else if (type == "place_ship") {
        return handlePlaceShip(input, server);
    }
    else if (type == "make_move") {
        return handleMakeMove(input, server);
    }

    return createJsonResponse("error", "error", "Unknown request type");
}

bool parseRegisterData(const QString &data, QString &nickname, QString &email, QString &password)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("nickname") || !obj.contains("email") || !obj.contains("password")) {
        return false;
    }

    nickname = obj["nickname"].toString();
    email = obj["email"].toString();
    password = obj["password"].toString();
    return true;
}

bool parseLoginData(const QString &data, QString &nickname, QString &password)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("nickname") || !obj.contains("password")) {
        return false;
    }

    nickname = obj["nickname"].toString();
    password = obj["password"].toString();
    return true;
}

QByteArray handleRegister(const QString &data)
{
    QString nickname, email, password;
    if (!parseRegisterData(data, nickname, email, password)) {
        qDebug() << "Failed to parse register data:" << data;
        return createJsonResponse("register", "error", "Invalid JSON format");
    }

    qDebug() << "Parsed register data - Nickname:" << nickname << "Email:" << email << "Password:" << password;

    DatabaseManager *dbManager = DatabaseManager::getInstance();
    if (dbManager->addUser(nickname, email, password)) {
        QJsonObject response;
        response["type"] = "register";
        response["status"] = "success";
        response["message"] = "User registered successfully";
        response["nickname"] = nickname;
        return QJsonDocument(response).toJson();
    } else {
        return createJsonResponse("register", "error", "User already exists");
    }
}

QByteArray slotLogin(const QString &data)
{
    QString nickname, password;
    if (!parseLoginData(data, nickname, password)) {
        qDebug() << "Failed to parse login data:" << data;
        return createJsonResponse("login", "error", "Invalid JSON format");
    }

    qDebug() << "Parsed login data - Nickname:" << nickname << "Password:" << password;

    QSqlDatabase db = DatabaseManager::getInstance()->getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM User WHERE nickname = :nickname AND password = :password");
    query.bindValue(":nickname", nickname);
    query.bindValue(":password", password);
    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return createJsonResponse("login", "error", "Database error");
    }

    if (query.next()) {
        QJsonObject response;
        response["type"] = "login";
        response["status"] = "success";
        response["message"] = "Login successful";
        response["nickname"] = nickname;
        return QJsonDocument(response).toJson();
    } else {
        qDebug() << "Login error";
        return createJsonResponse("login", "error", "Invalid nickname or password");
    }
}
QByteArray handleStartGame(const QString &data, MyTcpServer *server)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return createJsonResponse("start_game", "error", "Invalid JSON format");
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("nickname")) {
        return createJsonResponse("start_game", "error", "Missing nickname field");
    }

    QString nickname = obj["nickname"].toString();
    qDebug() << "Parsed start_game - Nickname:" << nickname;

    server->addPlayerToGame(nickname);
    qDebug() << "Added player to game:" << nickname;

    if (server->getPlayerCount() < 2) {
        qDebug() << "Waiting for second player, current count:" << server->getPlayerCount();
        return createJsonResponse("start_game", "waiting", "Waiting for opponent");
    } else {
        QString opponent = server->getOpponent(nickname);
        DatabaseManager *dbManager = DatabaseManager::getInstance();
        int gameId = dbManager->createGame(nickname, opponent);
        if (gameId <= 0) {
            return createJsonResponse("start_game", "error", "Failed to create game");
        }

        server->currentGameId = gameId;
        qDebug() << "Game created with ID:" << gameId << "between" << nickname << "and" << opponent;

        // Отправляем сообщение текущему игроку (nickname)
        QJsonObject response;
        response["type"] = "game_ready";
        response["status"] = "success";
        response["message"] = "Please place your ships and confirm readiness";
        response["game_id"] = gameId;
        response["opponent"] = opponent;
        qDebug() << "Sending game_ready to" << nickname;
        server->sendMessageToUser(nickname, QJsonDocument(response).toJson());

        // Отправляем сообщение оппоненту
        QJsonObject opponentResponse;
        opponentResponse["type"] = "game_ready";
        opponentResponse["status"] = "success";
        opponentResponse["message"] = "Please place your ships and confirm readiness";
        opponentResponse["game_id"] = gameId;
        opponentResponse["opponent"] = nickname;
        qDebug() << "Sending game_ready to" << opponent;
        server->sendMessageToUser(opponent, QJsonDocument(opponentResponse).toJson());

        qDebug() << "Returning game_ready response for" << nickname;
        return QJsonDocument(response).toJson();
    }
}
QByteArray handlePlaceShip(const QString &data, MyTcpServer *server)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return createJsonResponse("place_ship", "error", "Invalid JSON format");
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("nickname") || !obj.contains("game_id") || !obj.contains("x") || !obj.contains("y") || !obj.contains("size") || !obj.contains("is_horizontal")) {
        return createJsonResponse("place_ship", "error", "Missing required fields");
    }

    QString nickname = obj["nickname"].toString();
    int gameId = obj["game_id"].toInt();
    int x = obj["x"].toInt();
    int y = obj["y"].toInt();
    int size = obj["size"].toInt();
    bool isHorizontal = obj["is_horizontal"].toBool();
    qDebug() << "Parsed place_ship - Nickname:" << nickname << "Game ID:" << gameId << "x:" << x << "y:" << y << "size:" << size << "is_horizontal:" << isHorizontal;

    DatabaseManager *dbManager = DatabaseManager::getInstance();
    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM Game WHERE game_id = :game_id AND (player1 = :nickname OR player2 = :nickname)");
    query.bindValue(":game_id", gameId);
    query.bindValue(":nickname", nickname);
    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return createJsonResponse("place_ship", "error", "Database error");
    }
    if (!query.next()) {
        qDebug() << "Invalid game or player for game_id:" << gameId << "nickname:" << nickname;
        return createJsonResponse("place_ship", "error", "Invalid game or player");
    }

    // Валидация размещения корабля (сетка 10x10)
    if (x < 0 || y < 0 || size <= 0 || (isHorizontal && x + size > 10) || (!isHorizontal && y + size > 10)) {
        qDebug() << "Invalid ship placement: x=" << x << "y=" << y << "size=" << size << "is_horizontal=" << isHorizontal;
        return createJsonResponse("place_ship", "error", "Invalid ship placement");
    }

    if (dbManager->saveShip(gameId, nickname, x, y, size, isHorizontal)) {
        qDebug() << "Ship saved for player" << nickname << "in game" << gameId;
        return createJsonResponse("place_ship", "success", "Ship placed successfully");
    } else {
        qDebug() << "Failed to save ship for game_id:" << gameId << "nickname:" << nickname;
        return createJsonResponse("place_ship", "error", "Failed to place ship");
    }
}

QByteArray handleMakeMove(const QString &data, MyTcpServer *server)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        return createJsonResponse("make_move", "error", "Invalid JSON format");
    }

    QJsonObject obj = doc.object();
    if (!obj.contains("nickname") || !obj.contains("game_id") || !obj.contains("x") || !obj.contains("y")) {
        return createJsonResponse("make_move", "error", "Missing required fields");
    }

    QString nickname = obj["nickname"].toString();
    int gameId = obj["game_id"].toInt();
    int x = obj["x"].toInt();
    int y = obj["y"].toInt();
    qDebug() << "Parsed make_move - Nickname:" << nickname << "Game ID:" << gameId << "x:" << x << "y:" << y;

    DatabaseManager *dbManager = DatabaseManager::getInstance();
    QSqlDatabase db = dbManager->getDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT * FROM Game WHERE game_id = :game_id AND (player1 = :nickname OR player2 = :nickname)");
    query.bindValue(":game_id", gameId);
    query.bindValue(":nickname", nickname);
    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return createJsonResponse("make_move", "error", "Database error");
    }
    if (!query.next()) {
        qDebug() << "Invalid game or player for game_id:" << gameId << "nickname:" << nickname;
        return createJsonResponse("make_move", "error", "Invalid game or player");
    }

    QString opponent = (query.value("player1").toString() == nickname) ? query.value("player2").toString() : query.value("player1").toString();
    QString currentTurn = dbManager->getCurrentTurn(gameId);
    if (currentTurn != nickname) {
        qDebug() << "Not player's turn: current_turn=" << currentTurn << "nickname=" << nickname;
        return createJsonResponse("make_move", "error", "Not your turn");
    }

    // Валидация координат (сетка 10x10)
    if (x < 0 || x >= 10 || y < 0 || y >= 10) {
        qDebug() << "Invalid move coordinates: x=" << x << "y=" << y;
        return createJsonResponse("make_move", "error", "Invalid move coordinates");
    }

    QString result = dbManager->checkMove(gameId, opponent, x, y);
    if (result == "error") {
        qDebug() << "Failed to check move for game_id:" << gameId << "opponent:" << opponent;
        return createJsonResponse("make_move", "error", "Failed to check move");
    }

    if (!dbManager->saveMove(gameId, nickname, x, y, result)) {
        qDebug() << "Failed to save move for game_id:" << gameId << "nickname:" << nickname;
        return createJsonResponse("make_move", "error", "Failed to save move");
    }

    // Обновляем ход (передаём ход оппоненту)
    if (!dbManager->updateTurn(gameId, opponent)) {
        qDebug() << "Failed to update turn for game_id:" << gameId << "nextPlayer:" << opponent;
        return createJsonResponse("make_move", "error", "Failed to update turn");
    }

    QJsonObject response;
    response["type"] = "make_move";
    response["status"] = result;
    response["message"] = result == "hit" ? "Hit!" : "Miss!";
    response["x"] = x;
    response["y"] = y;
    response["current_turn"] = opponent;

    QJsonObject opponentResponse;
    opponentResponse["type"] = "move_result";
    opponentResponse["status"] = result;
    opponentResponse["message"] = result == "hit" ? "Your ship was hit!" : "Opponent missed!";
    opponentResponse["x"] = x;
    opponentResponse["y"] = y;
    server->sendMessageToUser(opponent, QJsonDocument(opponentResponse).toJson());

    return QJsonDocument(response).toJson();
}

QByteArray createJsonResponse(const QString &type, const QString &status, const QString &message)
{
    QJsonObject response;
    response["type"] = type;
    response["status"] = status;
    response["message"] = message;
    return QJsonDocument(response).toJson();
}
