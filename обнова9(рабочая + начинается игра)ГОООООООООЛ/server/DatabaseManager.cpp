#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>

DatabaseManager* DatabaseManager::instance = nullptr;
QMutex mutex;

DatabaseManager::DatabaseManager()
{
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "Error: SQLite driver not available!";
    } else {
        qDebug() << "SQLite driver is available.";
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("server_db.sqlite");
    qDebug() << "Attempting to open database at:" << db.databaseName();

    if (!db.open()) {
        qDebug() << "Error opening DB:" << db.lastError().text();
    } else {
        qDebug() << "Database connected successfully!";
        QSqlQuery query(db);

        // Создание таблицы User
        bool success = query.exec("CREATE TABLE IF NOT EXISTS User ("
                                  "nickname TEXT PRIMARY KEY, "
                                  "email TEXT NOT NULL UNIQUE, "
                                  "password TEXT NOT NULL, "
                                  "connection_info TEXT)");
        if (!success) {
            qDebug() << "Error creating table User:" << query.lastError().text();
        } else {
            qDebug() << "Table User created or already exists.";
        }

        // Создание таблицы Game
        success = query.exec("CREATE TABLE IF NOT EXISTS Game ("
                             "game_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "player1 TEXT NOT NULL, "
                             "player2 TEXT NOT NULL, "
                             "current_turn TEXT NOT NULL, " // Чей ход
                             "FOREIGN KEY(player1) REFERENCES User(nickname), "
                             "FOREIGN KEY(player2) REFERENCES User(nickname))");
        if (!success) {
            qDebug() << "Error creating table Game:" << query.lastError().text();
        } else {
            qDebug() << "Table Game created or already exists.";
        }

        // Создание таблицы Ship
        success = query.exec("CREATE TABLE IF NOT EXISTS Ship ("
                             "ship_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "game_id INTEGER NOT NULL, "
                             "player TEXT NOT NULL, "
                             "x INTEGER NOT NULL, "
                             "y INTEGER NOT NULL, "
                             "size INTEGER NOT NULL, "
                             "is_horizontal INTEGER NOT NULL, " // 1 - горизонтальный, 0 - вертикальный
                             "FOREIGN KEY(game_id) REFERENCES Game(game_id), "
                             "FOREIGN KEY(player) REFERENCES User(nickname))");
        if (!success) {
            qDebug() << "Error creating table Ship:" << query.lastError().text();
        } else {
            qDebug() << "Table Ship created or already exists.";
        }

        // Создание таблицы Move
        success = query.exec("CREATE TABLE IF NOT EXISTS Move ("
                             "move_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "game_id INTEGER NOT NULL, "
                             "player TEXT NOT NULL, "
                             "x INTEGER NOT NULL, "
                             "y INTEGER NOT NULL, "
                             "result TEXT NOT NULL, " // "hit", "miss", "sunk"
                             "FOREIGN KEY(game_id) REFERENCES Game(game_id), "
                             "FOREIGN KEY(player) REFERENCES User(nickname))");
        if (!success) {
            qDebug() << "Error creating table Move:" << query.lastError().text();
        } else {
            qDebug() << "Table Move created or already exists.";
        }
    }
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
    instance = nullptr;
}

DatabaseManager* DatabaseManager::getInstance()
{
    if (!instance) {
        instance = new DatabaseManager();
    }
    return instance;
}

QSqlDatabase DatabaseManager::getDatabase()
{
    return db;
}

bool DatabaseManager::addUser(const QString &nickname, const QString &email, const QString &password)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", nickname);
    query.bindValue(":email", email);
    query.bindValue(":password", password);
    query.bindValue(":connection_info", "");

    qDebug() << "Adding user - Nickname:" << nickname << "Email:" << email;

    if (!query.exec()) {
        qDebug() << "Error adding user:" << query.lastError().text();
        return false;
    }
    qDebug() << "User added successfully.";
    return true;
}

void DatabaseManager::printUsers()
{
    QSqlQuery query(db);
    if (!query.exec("SELECT * FROM User")) {
        qDebug() << "Error fetching users:" << query.lastError().text();
        return;
    }
    while (query.next()) {
        qDebug() << "Nickname:" << query.value("nickname").toString()
        << "Email:" << query.value("email").toString()
        << "Password:" << query.value("password").toString()
        << "Connection:" << query.value("connection_info").toString();
    }
}

int DatabaseManager::createGame(const QString &player1, const QString &player2)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return -1;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Game (player1, player2, current_turn) VALUES (:player1, :player2, :current_turn)");
    query.bindValue(":player1", player1);
    query.bindValue(":player2", player2);
    query.bindValue(":current_turn", player1); // Инициализируем первым игроком

    if (!query.exec()) {
        qDebug() << "Error creating game:" << query.lastError().text();
        return -1;
    }

    // Получаем ID новой игры
    query.exec("SELECT last_insert_rowid()");
    if (query.next()) {
        int gameId = query.value(0).toInt();
        qDebug() << "Game created with ID:" << gameId << "between" << player1 << "and" << player2;
        return gameId;
    }
    return -1;
}

bool DatabaseManager::saveShip(int gameId, const QString &player, int x, int y, int size, bool isHorizontal)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Ship (game_id, player, x, y, size, is_horizontal) VALUES (:game_id, :player, :x, :y, :size, :is_horizontal)");
    query.bindValue(":game_id", gameId);
    query.bindValue(":player", player);
    query.bindValue(":x", x);
    query.bindValue(":y", y);
    query.bindValue(":size", size);
    query.bindValue(":is_horizontal", isHorizontal ? 1 : 0);

    if (!query.exec()) {
        qDebug() << "Error saving ship:" << query.lastError().text();
        return false;
    }
    qDebug() << "Ship saved for player" << player << "in game" << gameId;
    return true;
}

bool DatabaseManager::saveMove(int gameId, const QString &player, int x, int y, const QString &result)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Move (game_id, player, x, y, result) VALUES (:game_id, :player, :x, :y, :result)");
    query.bindValue(":game_id", gameId);
    query.bindValue(":player", player);
    query.bindValue(":x", x);
    query.bindValue(":y", y);
    query.bindValue(":result", result);

    if (!query.exec()) {
        qDebug() << "Error saving move:" << query.lastError().text();
        return false;
    }
    qDebug() << "Move saved: Player" << player << "in game" << gameId << "at (" << x << "," << y << ") - Result:" << result;
    return true;
}

QString DatabaseManager::checkMove(int gameId, const QString &player, int x, int y)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return "error";
    }

    // Получаем оппонента
    QSqlQuery gameQuery(db);
    gameQuery.prepare("SELECT player1, player2 FROM Game WHERE game_id = :game_id");
    gameQuery.bindValue(":game_id", gameId);
    if (!gameQuery.exec() || !gameQuery.next()) {
        qDebug() << "Error fetching game:" << gameQuery.lastError().text();
        return "error";
    }

    QString player1 = gameQuery.value("player1").toString();
    QString player2 = gameQuery.value("player2").toString();
    QString opponent = (player == player1) ? player2 : player1;

    // Проверяем, не стреляли ли уже в эту клетку
    QSqlQuery moveQuery(db);
    moveQuery.prepare("SELECT result FROM Move WHERE game_id = :game_id AND player = :player AND x = :x AND y = :y");
    moveQuery.bindValue(":game_id", gameId);
    moveQuery.bindValue(":player", player);
    moveQuery.bindValue(":x", x);
    moveQuery.bindValue(":y", y);
    if (moveQuery.exec() && moveQuery.next()) {
        return "already_shot";
    }

    // Проверяем, есть ли корабль оппонента в этой клетке
    QSqlQuery shipQuery(db);
    shipQuery.prepare("SELECT size, is_horizontal FROM Ship WHERE game_id = :game_id AND player = :player");
    shipQuery.bindValue(":game_id", gameId);
    shipQuery.bindValue(":player", opponent);
    if (!shipQuery.exec()) {
        qDebug() << "Error fetching ships:" << shipQuery.lastError().text();
        return "error";
    }

    bool hit = false;
    int shipSize = 0;
    while (shipQuery.next()) {
        int shipX = shipQuery.value("x").toInt();
        int shipY = shipQuery.value("y").toInt();
        shipSize = shipQuery.value("size").toInt();
        bool isHorizontal = shipQuery.value("is_horizontal").toBool();

        if (isHorizontal) {
            if (y == shipY && x >= shipX && x < shipX + shipSize) {
                hit = true;
                break;
            }
        } else {
            if (x == shipX && y >= shipY && y < shipY + shipSize) {
                hit = true;
                break;
            }
        }
    }

    if (hit) {
        // Проверяем, потоплен ли корабль
        int hitCount = 0;
        QSqlQuery hitQuery(db);
        hitQuery.prepare("SELECT COUNT(*) FROM Move WHERE game_id = :game_id AND player = :player AND result = 'hit'");
        hitQuery.bindValue(":game_id", gameId);
        hitQuery.bindValue(":player", player);
        if (hitQuery.exec() && hitQuery.next()) {
            hitCount = hitQuery.value(0).toInt();
        }

        // Увеличиваем счётчик попаданий
        hitCount++;
        if (hitCount == shipSize) {
            return "sunk";
        }
        return "hit";
    }
    return "miss";
}

QString DatabaseManager::getCurrentTurn(int gameId)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return "";
    }

    QSqlQuery query(db);
    query.prepare("SELECT current_turn FROM Game WHERE game_id = :game_id");
    query.bindValue(":game_id", gameId);
    if (!query.exec() || !query.next()) {
        qDebug() << "Error fetching current turn:" << query.lastError().text();
        return "";
    }
    return query.value("current_turn").toString();
}

bool DatabaseManager::updateTurn(int gameId, const QString &nextPlayer)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        qDebug() << "Database is not open!";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE Game SET current_turn = :current_turn WHERE game_id = :game_id");
    query.bindValue(":current_turn", nextPlayer);
    query.bindValue(":game_id", gameId);
    if (!query.exec()) {
        qDebug() << "Error updating turn:" << query.lastError().text();
        return false;
    }
    qDebug() << "Turn updated to" << nextPlayer << "for game" << gameId;
    return true;
}
