#include "func2serv.h"
#include "DatabaseManager.h"
#include "mytcpserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

/**
 * @brief Создает JSON-ответ.
 *
 * Эта функция создает JSON-ответ в формате:
 * {
 *   "type": "тип ответа",
 *   "status": "статус ответа",
 *   "message": "сообщение"
 * }
 *
 * @param type Тип ответа (например, "register", "login", "error").
 * @param status Статус ответа ("success", "error").
 * @param message Сообщение, содержащее информацию об ответе.
 * @return JSON-документ в формате QByteArray с добавлением символов "\r\n" в конце.
 */
QByteArray createJsonResponse(const QString &type, const QString &status, const QString &message) {
    QJsonObject jsonObj;  // Создаем JSON объект.
    jsonObj["type"] = type;  // Добавляем поле "type".
    jsonObj["status"] = status; // Добавляем поле "status".
    jsonObj["message"] = message;  // Добавляем поле "message".
    QJsonDocument doc(jsonObj);  // Создаем JSON документ из объекта.
    return doc.toJson(QJsonDocument::Compact) + "\r\n";  // Преобразуем в QByteArray и добавляем разделитель.
}

/**
 * @brief Парсит входящую строку, содержащую JSON-команду, и обрабатывает её.
 *
 * Эта функция выполняет следующие действия:
 * 1.  Парсит входящую строку в QJsonDocument.
 * 2.  Проверяет формат JSON и наличие поля "type".
 * 3.  В зависимости от значения поля "type" вызывает соответствующие обработчики команд.
 * 4.  Возвращает JSON-ответ.
 *
 * @param input Входящая строка, содержащая JSON-команду.
 * @param server Указатель на объект MyTcpServer, используемый для взаимодействия с клиентами.
 * @return JSON-ответ в формате QByteArray.
 */
QByteArray parse(const QString &input, MyTcpServer *server) {
    qDebug() << "Received input:" << input;  // Выводим полученные данные в лог для отладки.
    QJsonDocument doc = QJsonDocument::fromJson(input.toUtf8());  // Пытаемся преобразовать строку в JSON-документ.
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format, input:" << input;  // Выводим сообщение об ошибке, если не удалось распарсить JSON.
        return createJsonResponse("error", "error", "Invalid JSON format");  // Возвращаем JSON-ответ об ошибке.
    }

    QJsonObject jsonObj = doc.object();  // Получаем JSON объект из документа.
    if (!jsonObj.contains("type")) {
        qDebug() << "Missing type field in JSON, input:" << input;  // Выводим сообщение об ошибке, если отсутствует поле "type".
        return createJsonResponse("error", "error", "Missing type field");  // Возвращаем JSON-ответ об ошибке.
    }

    QString type = jsonObj["type"].toString();  // Получаем значение поля "type".
    qDebug() << "Parsed type:" << type;  // Выводим тип команды в лог.
    if (type == "register") {
        QByteArray response = handleRegister(input);  // Обрабатываем команду "register".
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj["status"].toString() == "success") {
                QString nickname = jsonObj["nickname"].toString();
                obj["nickname"] = nickname;
                response = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\r\n";
            }
        }
        return response;  // Возвращаем ответ.
    } else if (type == "login") {
        QByteArray response = slotLogin(input);  // Обрабатываем команду "login".
        QJsonDocument doc = QJsonDocument::fromJson(response);
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj["status"].toString() == "success") {
                QString nickname = jsonObj["nickname"].toString();
                obj["nickname"] = nickname;
                response = QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\r\n";
            }
        }
        return response;  // Возвращаем ответ.
    } else if (type == "start_game") {
        return handleStartGame(input, server);  // Обрабатываем команду "start_game".
    } else if (type == "place_ship") {
        return handlePlaceShip(input, server); // Обрабатываем команду "place_ship".
    } else if (type == "make_move") {
        return handleMakeMove(input, server);  // Обрабатываем команду "make_move".
    } else if (type == "ready_to_battle") {
        return createJsonResponse("ready_to_battle", "success", "Ready status received"); // Обрабатываем команду "ready_to_battle".
    }

    qDebug() << "Unknown command type:" << type;  // Выводим сообщение об ошибке, если тип команды неизвестен.
    return createJsonResponse("error", "error", "Unknown command");  // Возвращаем JSON-ответ об ошибке.
}

#include "func2serv.h"
#include "DatabaseManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

/**
 * @brief Парсит данные регистрации из JSON строки.
 *
 * Извлекает данные регистрации (никнейм, email, пароль) из JSON строки.
 *
 * @param data JSON строка, содержащая данные регистрации.
 *             Ожидается JSON объект с полями "nickname", "email" и "password".
 * @param nickname Ссылка на строку для сохранения никнейма.
 * @param email Ссылка на строку для сохранения email.
 * @param password Ссылка на строку для сохранения пароля.
 * @return true, если данные успешно извлечены и проверены, иначе false.
 */
bool parseRegisterData(const QString &data, QString &nickname, QString &email, QString &password) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());  // Преобразуем JSON строку в QJsonDocument.
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format in parseRegisterData";
        return false;  // Возвращаем false, если формат неверный.
    }

    QJsonObject jsonObj = doc.object();  // Получаем JSON объект.
    if (!jsonObj.contains("nickname") || !jsonObj.contains("email") || !jsonObj.contains("password")) {
        qDebug() << "Missing fields in parseRegisterData";
        return false;  // Возвращаем false, если отсутствуют необходимые поля.
    }

    nickname = jsonObj["nickname"].toString();  // Получаем никнейм.
    email = jsonObj["email"].toString();  // Получаем email.
    password = jsonObj["password"].toString();  // Получаем пароль.

    if (nickname.isEmpty() || email.isEmpty() || password.isEmpty()) {
        qDebug() << "Empty fields in parseRegisterData";
        return false;  // Возвращаем false, если какое-либо поле пустое.
    }

    qDebug() << "Parsed register data - Nickname:" << nickname << "Email:" << email << "Password:" << password;
    return true;  // Возвращаем true, если данные успешно извлечены.
}

/**
 * @brief Парсит данные логина из JSON строки.
 *
 * Извлекает данные логина (никнейм, пароль) из JSON строки.
 *
 * @param data JSON строка, содержащая данные логина.
 *             Ожидается JSON объект с полями "nickname" и "password".
 * @param nickname Ссылка на строку для сохранения никнейма.
 * @param password Ссылка на строку для сохранения пароля.
 * @return true, если данные успешно извлечены и проверены, иначе false.
 */
bool parseLoginData(const QString &data, QString &nickname, QString &password) {
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());  // Преобразуем JSON строку в QJsonDocument.
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format in parseLoginData";
        return false;  // Возвращаем false, если формат неверный.
    }

    QJsonObject jsonObj = doc.object();  // Получаем JSON объект.
    if (!jsonObj.contains("nickname") || !jsonObj.contains("password")) {
        qDebug() << "Missing fields in parseLoginData";
        return false;  // Возвращаем false, если отсутствуют необходимые поля.
    }

    nickname = jsonObj["nickname"].toString();  // Получаем никнейм.
    password = jsonObj["password"].toString();  // Получаем пароль.

    if (nickname.isEmpty() || password.isEmpty()) {
        qDebug() << "Empty fields in parseLoginData";
        return false;  // Возвращаем false, если какое-либо поле пустое.
    }

    qDebug() << "Parsed login data - Nickname:" << nickname << "Password:" << password;
    return true;  // Возвращаем true, если данные успешно извлечены.
}

/**
 * @brief Обрабатывает запрос на регистрацию пользователя.
 *
 * Парсит данные регистрации, проверяет их, проверяет наличие пользователя
 * с таким никнеймом или email в базе данных и, если все в порядке,
 * добавляет нового пользователя в базу данных.
 *
 * @param data JSON строка, содержащая данные регистрации.
 * @return JSON-ответ, содержащий результат обработки запроса.
 */
QByteArray handleRegister(const QString &data) {
    QString nickname, email, password;  // Переменные для хранения данных.
    if (!parseRegisterData(data, nickname, email, password)) {
        return createJsonResponse("register", "error", "Invalid registration data");  // Возвращаем ошибку, если данные неверны.
    }

    DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.
    QSqlDatabase database = db->getDatabase();  // Получаем объект базы данных.
    if (!database.isOpen()) {
        qDebug() << "Database is not open in handleRegister";
        return createJsonResponse("register", "error", "Database is not open");  // Возвращаем ошибку, если база данных не открыта.
    }

    QSqlQuery query(database);  // Создаем объект запроса.
    query.prepare("SELECT COUNT(*) FROM User WHERE nickname = :nickname OR email = :email");  // Подготавливаем запрос для проверки существования пользователя.
    query.bindValue(":nickname", nickname);  // Привязываем значение никнейма.
    query.bindValue(":email", email);  // Привязываем значение email.

    qDebug() << "Executing query in handleRegister: SELECT COUNT(*) FROM User WHERE nickname =" << nickname << "OR email =" << email;

    if (!query.exec()) {
        qDebug() << "Database query failed (SELECT) in handleRegister:" << query.lastError().text();  // Выводим сообщение об ошибке, если запрос не выполнен.
        return createJsonResponse("register", "error", "Database query failed");  // Возвращаем ошибку, если запрос не выполнен.
    }

    query.next();  // Переходим к результату запроса.
    if (query.value(0).toInt() > 0) {
        return createJsonResponse("register", "error", "User already exists");  // Возвращаем ошибку, если пользователь уже существует.
    }

    bool success = db->addUser(nickname, email, password);  // Добавляем нового пользователя в базу данных.
    if (success) {
        return createJsonResponse("register", "success", "User registered successfully");  // Возвращаем успех, если добавление прошло успешно.
    } else {
        return createJsonResponse("register", "error", "Registration failed");  // Возвращаем ошибку, если добавление не удалось.
    }
}

#include "func2serv.h"
#include "DatabaseManager.h"
#include "mytcpserver.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

/**
 * @brief Обрабатывает запрос на вход пользователя в систему (логин).
 *
 * Парсит данные логина (никнейм, пароль), проверяет их, проверяет наличие пользователя
 * с такими данными в базе данных и, если все в порядке, возвращает сообщение об успехе.
 *
 * @param data JSON строка, содержащая данные логина.  Ожидается JSON объект с полями "nickname" и "password".
 * @return JSON-ответ, содержащий результат обработки запроса.
 */
QByteArray slotLogin(const QString &data) {
    QString nickname, password;  // Переменные для хранения никнейма и пароля.

    // Парсим данные логина из JSON строки.
    if (!parseLoginData(data, nickname, password)) {
        qDebug() << "Invalid login data in slotLogin";
        return createJsonResponse("login", "error", "Invalid login data");  // Возвращаем ошибку, если данные неверны.
    }

    DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.
    QSqlDatabase database = db->getDatabase();  // Получаем объект базы данных.

    // Проверяем, открыта ли база данных.
    if (!database.isOpen()) {
        qDebug() << "Database is not open in slotLogin";
        return createJsonResponse("login", "error", "Database is not open");  // Возвращаем ошибку, если база данных не открыта.
    }

    QSqlQuery query(database);  // Создаем объект запроса.
    // Подготавливаем SQL запрос для проверки наличия пользователя с указанным никнеймом и паролем.
    query.prepare("SELECT * FROM User WHERE nickname = :nickname AND password = :password");
    query.bindValue(":nickname", nickname);  // Привязываем значение никнейма.
    query.bindValue(":password", password);  // Привязываем значение пароля.

    qDebug() << "Executing query in slotLogin: SELECT * FROM User WHERE nickname =" << nickname << "AND password =" << password;

    // Выполняем SQL запрос.
    if (!query.exec()) {
        qDebug() << "Database query failed (SELECT) in slotLogin:" << query.lastError().text();
        return createJsonResponse("login", "error", "Database query failed");  // Возвращаем ошибку, если запрос не выполнен.
    }

    // Проверяем, найден ли пользователь с указанными данными.
    if (query.next()) {
        qDebug() << "Login successful";
        return createJsonResponse("login", "success", "Login successful");  // Возвращаем успех, если пользователь найден.
    } else {
        qDebug() << "Login error";
        return createJsonResponse("login", "error", "Invalid nickname or password");  // Возвращаем ошибку, если пользователь не найден.
    }
}

/**
 * @brief Обрабатывает запрос на начало игры.
 *
 * Добавляет игрока в игру, создает игру в базе данных, если набралось два игрока,
 * и отправляет сообщение обоим игрокам о готовности к расстановке кораблей.
 *
 * @param data JSON строка, содержащая данные запроса на начало игры. Ожидается JSON объект с полем "nickname".
 * @param server Указатель на объект MyTcpServer, управляющий подключением клиентов.
 * @return JSON-ответ, содержащий результат обработки запроса.
 */
QByteArray handleStartGame(const QString &data, MyTcpServer *server) {
    // Преобразуем JSON строку в QJsonDocument.
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

    // Проверяем, является ли документ JSON объектом.
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format in handleStartGame";
        return createJsonResponse("start_game", "error", "Invalid JSON format");  // Возвращаем ошибку, если формат неверный.
    }

    QJsonObject jsonObj = doc.object();  // Получаем JSON объект из документа.
    QString nickname = jsonObj["nickname"].toString();  // Получаем никнейм игрока.

    // Проверяем, указан ли никнейм.
    if (nickname.isEmpty()) {
        qDebug() << "Missing nickname in handleStartGame";
        return createJsonResponse("start_game", "error", "Missing nickname");  // Возвращаем ошибку, если никнейм не указан.
    }

    // Проверяем, передан ли указатель на сервер.
    if (!server) {
        qDebug() << "Server error in handleStartGame";
        return createJsonResponse("start_game", "error", "Server error");  // Возвращаем ошибку, если указатель на сервер не передан.
    }

    server->addPlayerToGame(nickname);  // Добавляем игрока в игру.

    // Если количество игроков в игре равно 2, начинаем игру.
    if (server->getPlayerCount() == 2) {
        QString opponent = server->getOpponent(nickname);  // Получаем никнейм оппонента.
        DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.
        int gameId = db->createGame(nickname, opponent);  // Создаем игру в базе данных.

        // Если игра успешно создана.
        if (gameId != -1) {
            server->currentGameId = gameId;  // Сохраняем ID игры на сервере.

            QJsonObject responseObj;  // Создаем JSON объект для ответа.
            responseObj["type"] = "game_ready";  // Указываем тип ответа.
            responseObj["status"] = "success";  // Указываем статус ответа.
            responseObj["message"] = "Please place your ships and confirm readiness";  // Указываем сообщение.
            responseObj["game_id"] = gameId;  // Добавляем ID игры в ответ.
            responseObj["opponent"] = opponent;  // Добавляем никнейм оппонента в ответ.
            QByteArray response = QJsonDocument(responseObj).toJson(QJsonDocument::Compact) + "\r\n";  // Преобразуем JSON объект в QByteArray.
            server->sendMessageToUser(nickname, response);  // Отправляем сообщение игроку.

            responseObj["opponent"] = nickname;
            response = QJsonDocument(responseObj).toJson(QJsonDocument::Compact) + "\r\n";
            server->sendMessageToUser(opponent, response);  // Отправляем сообщение оппоненту.
        } else {
            qDebug() << "Failed to create game in handleStartGame";
            return createJsonResponse("start_game", "error", "Failed to create game");  // Возвращаем ошибку, если не удалось создать игру.
        }
    }

    return createJsonResponse("start_game", "waiting", "Waiting for opponent");  // Возвращаем сообщение о том, что ожидаем оппонента.
}

/**
 * @brief Обрабатывает запрос на размещение корабля на игровом поле.
 *
 * Извлекает данные о корабле из JSON строки, проверяет их корректность,
 * и сохраняет информацию о размещении корабля в базе данных.
 *
 * @param data JSON строка, содержащая данные о размещении корабля.
 *             Ожидается JSON объект с полями "nickname", "game_id", "x", "y", "size", "is_horizontal".
 * @param server Указатель на объект MyTcpServer, управляющий подключением клиентов.
 * @return JSON-ответ, содержащий результат обработки запроса.
 */
QByteArray handlePlaceShip(const QString &data, MyTcpServer *server) {
    // Преобразуем JSON строку в QJsonDocument.
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

    // Проверяем, является ли документ JSON объектом.
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Invalid JSON format");  // Возвращаем ошибку, если формат неверный.
    }

    QJsonObject jsonObj = doc.object();  // Получаем JSON объект из документа.

    // Проверяем наличие обязательных полей.
    if (!jsonObj.contains("nickname") || !jsonObj.contains("game_id") || !jsonObj.contains("x") ||
        !jsonObj.contains("y") || !jsonObj.contains("size") || !jsonObj.contains("is_horizontal")) {
        qDebug() << "Missing required fields in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Missing required fields");  // Возвращаем ошибку, если отсутствуют обязательные поля.
    }

    QString nickname = jsonObj["nickname"].toString();  // Получаем никнейм игрока.
    int gameId = jsonObj["game_id"].toInt();  // Получаем ID игры.
    int x = jsonObj["x"].toInt();  // Получаем координату X.
    int y = jsonObj["y"].toInt();  // Получаем координату Y.
    int size = jsonObj["size"].toInt();  // Получаем размер корабля.
    bool isHorizontal = jsonObj["is_horizontal"].toBool();  // Получаем ориентацию корабля.

    // Проверяем, что никнейм не пустой.
    if (nickname.isEmpty()) {
        qDebug() << "Invalid nickname in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Invalid nickname");  // Возвращаем ошибку, если никнейм недействителен.
    }

    // Проверяем, что ID игры соответствует текущей игре на сервере.
    if (gameId != server->getGameId()) {
        qDebug() << "Invalid game ID in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Invalid game ID");  // Возвращаем ошибку, если ID игры недействителен.
    }

    // Проверка корректности координат и размера
    if (x < 0 || y < 0 || size < 1 || size > 4 || x >= 10 || y >= 10) {
        qDebug() << "Invalid ship coordinates or size in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Invalid ship coordinates or size");  // Возвращаем ошибку, если координаты или размер корабля недействительны.
    }
    if (isHorizontal && x + size > 10) {
        qDebug() << "Ship exceeds horizontal board limits in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Ship exceeds horizontal board limits");  // Возвращаем ошибку, если корабль выходит за горизонтальные границы поля.
    }
    if (!isHorizontal && y + size > 10) {
        qDebug() << "Ship exceeds vertical board limits in handlePlaceShip";
        return createJsonResponse("place_ship", "error", "Ship exceeds vertical board limits");  // Возвращаем ошибку, если корабль выходит за вертикальные границы поля.
    }

    DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.

    // Сохраняем информацию о размещении корабля в базе данных.
    if (db->saveShip(gameId, nickname, x, y, size, isHorizontal)) {
        qDebug() << "Ship placed successfully for" << nickname << ": game_id=" << gameId
                 << ", x=" << x << ", y=" << y << ", size=" << size << ", is_horizontal=" << isHorizontal;
        return createJsonResponse("place_ship", "success", "Ship placed successfully");  // Возвращаем успех, если корабль успешно размещен.
    } else {
        qDebug() << "Failed to place ship for" << nickname << ": game_id=" << gameId;
        return createJsonResponse("place_ship", "error", "Failed to place ship");  // Возвращаем ошибку, если не удалось разместить корабль.
    }
}

/**
 * @brief Обрабатывает запрос на совершение хода в игре.
 *
 * Извлекает данные о ходе из JSON строки, проверяет их корректность,
 * проверяет результат хода, и сохраняет информацию о ходе в базе данных.
 *
 * @param data JSON строка, содержащая данные о ходе.
 *             Ожидается JSON объект с полями "nickname", "game_id", "x", "y".
 * @param server Указатель на объект MyTcpServer, управляющий подключением клиентов.
 * @return JSON-ответ, содержащий результат обработки запроса.
 */
QByteArray handleMakeMove(const QString &data, MyTcpServer *server) {
    // Преобразуем JSON строку в QJsonDocument.
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());

    // Проверяем, является ли документ JSON объектом.
    if (!doc.isObject()) {
        qDebug() << "Invalid JSON format in handleMakeMove";
        return createJsonResponse("make_move", "error", "Invalid JSON format");  // Возвращаем ошибку, если формат неверный.
    }

    QJsonObject jsonObj = doc.object();  // Получаем JSON объект из документа.

    // Проверяем наличие обязательных полей.
    if (!jsonObj.contains("nickname") || !jsonObj.contains("game_id") || !jsonObj.contains("x") || !jsonObj.contains("y")) {
        qDebug() << "Missing required fields in handleMakeMove";
        return createJsonResponse("make_move", "error", "Missing required fields");  // Возвращаем ошибку, если отсутствуют обязательные поля.
    }

    QString nickname = jsonObj["nickname"].toString();  // Получаем никнейм игрока.
    int gameId = jsonObj["game_id"].toInt();  // Получаем ID игры.
    int x = jsonObj["x"].toInt();  // Получаем координату X хода.
    int y = jsonObj["y"].toInt();  // Получаем координату Y хода.

    // Проверяем, что ID игры соответствует текущей игре на сервере.
    if (gameId != server->getGameId()) {
        qDebug() << "Invalid game ID in handleMakeMove";
        return createJsonResponse("make_move", "error", "Invalid game ID");  // Возвращаем ошибку, если ID игры недействителен.
    }

    DatabaseManager *db = DatabaseManager::getInstance();  // Получаем экземпляр DatabaseManager.

    // Получаем текущий ход из базы данных.
    QString currentTurn = db->getCurrentTurn(gameId);

    // Проверяем, что ход делает игрок, чей сейчас ход.
    if (currentTurn != nickname) {
        qDebug() << "Not your turn in handleMakeMove, current turn:" << currentTurn << ", player:" << nickname;
        return createJsonResponse("make_move", "error", "Not your turn");  // Возвращаем ошибку, если сейчас не ход этого игрока.
    }

    // Проверяем результат хода.
    QString result = db->checkMove(gameId, nickname, x, y);
    if (result == "error") {
        qDebug() << "Failed to process move in handleMakeMove";
        return createJsonResponse("make_move", "error", "Failed to process move");  // Возвращаем ошибку, если не удалось обработать ход.
    }

    // Проверяем, что в эту клетку уже стреляли.
    if (result == "already_shot") {
        qDebug() << "Cell already shot in handleMakeMove";
        return createJsonResponse("make_move", "error", "Cell already shot");  // Возвращаем ошибку, если в эту клетку уже стреляли.
    }

    // Сохраняем ход в базе данных.
    db->saveMove(gameId, nickname, x, y, result);

    // Получаем никнейм оппонента.
    QString opponent = server->getOpponent(nickname);
    // Определяем, чей будет следующий ход. Если попал или потопил - ходит тот же игрок, иначе - оппонент.
    QString nextTurn = (result != "hit" && result != "sunk") ? opponent : nickname;

    // Обновляем текущий ход в базе данных, если следующий ход оппонента.
    if (result != "hit" && result != "sunk") {
        db->updateTurn(gameId, opponent);
    }

    QJsonObject response;  // Создаем JSON объект для ответа текущему игроку.
    response["type"] = "make_move";  // Устанавливаем тип ответа.
    response["status"] = result;  // Устанавливаем статус ответа.
    response["message"] = result == "sunk" ? "Ship sunk!" : result == "hit" ? "Hit!" : "Miss!";  // Устанавливаем сообщение.
    response["x"] = x;  // Устанавливаем координату X.
    response["y"] = y;  // Устанавливаем координату Y.
    response["current_turn"] = nextTurn;  // Устанавливаем, чей следующий ход.

    QJsonObject opponentResponse;  // Создаем JSON объект для ответа оппоненту.
    opponentResponse["type"] = "move_result";  // Устанавливаем тип ответа.
    opponentResponse["status"] = result;  // Устанавливаем статус ответа.
    opponentResponse["x"] = x;  // Устанавливаем координату X.
    opponentResponse["y"] = y;  // Устанавливаем координату Y.
    opponentResponse["message"] = result == "sunk" ? "Your ship was sunk!" : result == "hit" ? "Your ship was hit!" : "Opponent missed!";  // Устанавливаем сообщение.
    opponentResponse["current_turn"] = nextTurn;  // Устанавливаем, чей следующий ход.

    // Отправляем сообщение оппоненту.
    server->sendMessageToUser(opponent, QJsonDocument(opponentResponse).toJson(QJsonDocument::Compact) + "\r\n");

    // Возвращаем ответ текущему игроку.
    return QJsonDocument(response).toJson(QJsonDocument::Compact) + "\r\n";
}























































   