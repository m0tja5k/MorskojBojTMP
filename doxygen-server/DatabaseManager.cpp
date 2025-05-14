/**
 * @file DatabaseManager.cpp
 * @brief Реализация класса DatabaseManager.
 *
 *  Этот файл содержит реализацию класса DatabaseManager,
 *  который управляет подключением к базе данных SQLite и выполняет SQL-запросы.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QSqlRecord>

/**
 * @brief Указатель на экземпляр класса DatabaseManager (Singleton).
 */
DatabaseManager* DatabaseManager::instance = nullptr;

/**
 * @brief Мьютекс для обеспечения потокобезопасности при доступе к экземпляру Singleton.
 */
QMutex mutex;

/**
 * @brief Конструктор класса DatabaseManager.
 *
 *  Инициализирует подключение к базе данных SQLite и создает необходимые таблицы,
 *  если они еще не существуют.
 */
DatabaseManager::DatabaseManager()
{
    /**
     * @brief Проверка доступности драйвера SQLite.
     */
    if (!QSqlDatabase::drivers().contains("QSQLITE")) {
        qDebug() << "Error: SQLite driver not available!";
    } else {
        qDebug() << "SQLite driver is available.";
    }

    /**
     * @brief Добавление соединения с базой данных.
     */
    db = QSqlDatabase::addDatabase("QSQLITE");
    /**
     * @brief Установка имени базы данных.
     */
    db.setDatabaseName("server_db.sqlite");
    /**
     * @brief Установка опций соединения.
     *
     * Устанавливаем тайм-аут для операций с базой данных.
     */
    db.setConnectOptions("QSQLITE_BUSY_TIMEOUT=1000"); // Устанавливаем тайм-аут 1 сек
    qDebug() << "Attempting to open database at:" << db.databaseName();

    /**
     * @brief Открытие соединения с базой данных.
     */
    if (!db.open()) {
        qDebug() << "Error opening DB:" << db.lastError().text();
    } else {
        qDebug() << "Database connected successfully!";
        QSqlQuery query(db);

        /**
         * @brief Создание таблицы User.
         */
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

        /**
         * @brief Создание таблицы Game.
         */
        success = query.exec("CREATE TABLE IF NOT EXISTS Game ("
                             "game_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "player1 TEXT NOT NULL, "
                             "player2 TEXT NOT NULL, "
                             "current_turn TEXT NOT NULL, "
                             "FOREIGN KEY(player1) REFERENCES User(nickname), "
                             "FOREIGN KEY(player2) REFERENCES User(nickname))");
        if (!success) {
            qDebug() << "Error creating table Game:" << query.lastError().text();
        } else {
            qDebug() << "Table Game created or already exists.";
        }

        /**
         * @brief Создание таблицы Ship.
         */
        success = query.exec("CREATE TABLE IF NOT EXISTS Ship ("
                             "ship_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "game_id INTEGER NOT NULL, "
                             "player TEXT NOT NULL, "
                             "x INTEGER NOT NULL, "
                             "y INTEGER NOT NULL, "
                             "size INTEGER NOT NULL, "
                             "is_horizontal INTEGER NOT NULL, "
                             "FOREIGN KEY(game_id) REFERENCES Game(game_id), "
                             "FOREIGN KEY(player) REFERENCES User(nickname))");
        if (!success) {
            qDebug() << "Error creating table Ship:" << query.lastError().text();
        } else {
            qDebug() << "Table Ship created or already exists.";
        }
    }
}

/**
 * @file DatabaseManager.cpp
 * @brief Реализация класса DatabaseManager (продолжение).
 *
 *  Этот файл содержит реализацию класса DatabaseManager,
 *  который управляет подключением к базе данных SQLite и выполняет SQL-запросы (продолжение).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QSqlRecord>

// ... (Предыдущий код DatabaseManager) ...

        /**
         * @brief Создание таблицы Move.
         */
        success = query.exec("CREATE TABLE IF NOT EXISTS Move ("
                             "move_id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "game_id INTEGER NOT NULL, "
                             "player TEXT NOT NULL, "
                             "x INTEGER NOT NULL, "
                             "y INTEGER NOT NULL, "
                             "result TEXT NOT NULL, "
                             "FOREIGN KEY(game_id) REFERENCES Game(game_id), "
                             "FOREIGN KEY(player) REFERENCES User(nickname))");
        if (!success) {
            qDebug() << "Error creating table Move:" << query.lastError().text();
        } else {
            qDebug() << "Table Move created or already exists.";
        }
    }
}

/**
 * @brief Деструктор класса DatabaseManager.
 *
 *  Закрывает соединение с базой данных при уничтожении объекта.
 */
DatabaseManager::~DatabaseManager()
{
    /**
     * @brief Закрытие соединения с базой данных, если оно открыто.
     */
    if (db.isOpen()) {
        db.close();
    }
    /**
     * @brief Установка указателя экземпляра в nullptr.
     */
    instance = nullptr;
}

/**
 * @brief Получение экземпляра DatabaseManager (Singleton).
 *
 *  Реализация паттерна Singleton. Возвращает существующий экземпляр или создает новый.
 *
 * @return Указатель на экземпляр DatabaseManager.
 */
DatabaseManager* DatabaseManager::getInstance()
{
    /**
     * @brief Проверка наличия экземпляра.
     *
     *  Если экземпляр не существует, создаем его.
     */
    if (!instance) {
        instance = new DatabaseManager();
    }
    return instance;
}

/**
 * @brief Получение объекта QSqlDatabase.
 *
 *  Предоставляет доступ к объекту QSqlDatabase для выполнения запросов.
 *
 * @return Объект QSqlDatabase.
 */
QSqlDatabase DatabaseManager::getDatabase()
{
    return db;
}

/**
 * @brief Добавление нового пользователя в базу данных.
 *
 * @param nickname Никнейм пользователя.
 * @param email Адрес электронной почты пользователя.
 * @param password Пароль пользователя.
 * @return True, если пользователь успешно добавлен, иначе - false.
 */
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

/**
 * @brief Вывод информации о всех пользователях из базы данных (для отладки).
 */
void DatabaseManager::printUsers()
{
    QMutexLocker locker(&mutex);
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

/**
 * @brief Создание новой игры в базе данных.
 *
 * @param player1 Никнейм первого игрока.
 * @param player2 Никнейм второго игрока.
 * @return ID созданной игры, или -1 в случае ошибки.
 */
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
    query.bindValue(":current_turn", player1);

 if (!query.exec()) {
        qDebug() << "Error creating game:" << query.lastError().text();
        return -1;
    }

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
    // Мьютекс уже заблокирован в вызывающей функции (checkMove), поэтому здесь не блокируем
    if (!db.isOpen()) {
        qDebug() << "Database is not open in saveMove!";
        return false;
    }

    qDebug() << "Starting saveMove for player" << player << "in game" << gameId << "at (" << x << "," << y << ") with result:" << result;

    QSqlQuery query(db);
    query.prepare("INSERT INTO Move (game_id, player, x, y, result) VALUES (:game_id, :player, :x, :y, :result)");
    query.bindValue(":game_id", gameId);
    query.bindValue(":player", player);
    query.bindValue(":x", x);
    query.bindValue(":y", y);
    query.bindValue(":result", result);

    qDebug() << "SQL query:" << query.lastQuery();
    qDebug() << "Bound values:" << query.boundValues();

    try {
        if (!query.exec()) {
            qDebug() << "Error saving move:" << query.lastError().text();
            return false;
        }
    } catch (const std::exception &e) {
        qDebug() << "Exception in saveMove:" << e.what();
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

/**
 * @file DatabaseManager.cpp
 * @brief Реализация методов работы с базой данных (окончание).
 *
 *  Этот файл содержит реализацию методов класса DatabaseManager,
 *  связанных с проверкой и сохранением ходов в игре "Морской бой".
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

    qDebug() << "Starting checkMove for player" << player << "in game" << gameId << "at (" << x << "," << y << ")";

    /**
     * @brief Начало транзакции для обеспечения атомарности операций
     */
    if (!db.transaction()) {
        qDebug() << "Failed to start transaction in checkMove:" << db.lastError().text();
        return "error";
    }

    /**
     * @brief Получение данных игры для определения противника
     */
    QSqlQuery gameQuery(db);
    gameQuery.prepare("SELECT player1, player2 FROM Game WHERE game_id = :game_id");
    gameQuery.bindValue(":game_id", gameId);
    if (!gameQuery.exec() || !gameQuery.next()) {
        qDebug() << "Error fetching game:" << gameQuery.lastError().text();
        db.rollback();
        return "error";
    }

    QString player1 = gameQuery.value("player1").toString();
    QString player2 = gameQuery.value("player2").toString();
    QString opponent = (player == player1) ? player2 : player1;
    qDebug() << "Opponent for" << player << "is" << opponent;

    /**
     * @brief Проверка повторного выстрела в ту же клетку
     */
    QSqlQuery moveQuery(db);
    moveQuery.prepare("SELECT result FROM Move WHERE game_id = :game_id AND player = :player AND x = :x AND y = :y");
    moveQuery.bindValue(":game_id", gameId);
    moveQuery.bindValue(":player", player);
    moveQuery.bindValue(":x", x);
    moveQuery.bindValue(":y", y);
    if (moveQuery.exec() && moveQuery.next()) {
        qDebug() << "Cell (" << x << "," << y << ") already shot by" << player;
        db.commit();
        return "already_shot";
    }

    /**
     * @brief Поиск кораблей противника и проверка попадания
     */
    QSqlQuery shipQuery(db);
    shipQuery.prepare("SELECT ship_id, x, y, size, is_horizontal FROM Ship WHERE game_id = :game_id AND player = :player");
    shipQuery.bindValue(":game_id", gameId);
    shipQuery.bindValue(":player", opponent);
    if (!shipQuery.exec()) {
        qDebug() << "Error fetching ships:" << shipQuery.lastError().text();
        db.rollback();
        return "error";
    }

    bool hit = false;
    int shipId = -1;
    int shipSize = 0;
    int shipX = 0, shipY = 0;
    bool isHorizontal = false;
    while (shipQuery.next()) {
        shipX = shipQuery.value(1).toInt();
        shipY = shipQuery.value(2).toInt();
        shipSize = shipQuery.value(3).toInt();
        isHorizontal = shipQuery.value(4).toBool();
        shipId = shipQuery.value(0).toInt();
        qDebug() << "Checking ship: id=" << shipId << ", x=" << shipX << ", y=" << shipY << ", size=" << shipSize << ", is_horizontal=" << isHorizontal;

        /**
         * @brief Проверка попадания выстрела в область корабля
         * 
         * Для горизонтального корабля: проверяем совпадение Y и диапазон X
         * Для вертикального корабля: проверяем совпадение X и диапазон Y
         */
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

/**
 * @brief Определение результата выстрела (попадание/промах/потопление)
 */
QString result;
if (hit) {
    /**
     * @brief Подсчет существующих попаданий по этому кораблю
     * 
     * Сложный запрос проверяет все попадания в области данного корабля:
     * - Для горизонтальных кораблей проверяем диапазон X при фиксированном Y
     * - Для вертикальных - диапазон Y при фиксированном X
     */
    QSqlQuery hitQuery(db);
    hitQuery.prepare(
        "SELECT COUNT(*) FROM Move WHERE game_id = :game_id AND player = :player AND result IN ('hit', 'sunk') AND "
        "(x >= :ship_x AND x < :ship_x + :size AND y = :ship_y AND :is_horizontal = 1 OR "
        "y >= :ship_y AND y < :ship_y + :size AND x = :ship_x AND :is_horizontal = 0)");
    ...
        
    /**
     * @определение статуса корабля (потоплен/ранен)
     * 
     * Если количество попаданий равняется размеру корабля - корабль потоплен
     * (+1 учитывает текущий выстрел)
     */
    int hitCount = hitQuery.value(0).toInt() + 1;
    qDebug() << "Ship id=" << shipId << ", hits=" << hitCount << ", size=" << shipSize;
    if (hitCount >= shipSize) {
        result = "sunk";
        qDebug() << "Ship id=" << shipId << "sunk!";
    } else {
        result = "hit";
    }
} else {
    result = "miss";
}

/**
 * @brief Сохранение результата хода в рамках текущей транзакции
 */
QSqlQuery moveInsertQuery(db);
moveInsertQuery.prepare(
    "INSERT INTO Move (...) VALUES (...)");
    
try {
    ...
} catch (const std::exception &e) {
    ...
}

/**
 * @brief Фиксация транзакции
 */
if (!db.commit()) {
    ...
}

/**
 * @brief Получение текущего игрока для совершения хода
 * 
 * @param gameId Идентификатор игры
 * @return Никнейм игрока, чей сейчас ход, или пустую строку при ошибке
 */
QString DatabaseManager::getCurrentTurn(int gameId)
{
    QMutexLocker locker(&mutex);
    if (!db.isOpen()) {
        ...
    }
    ...
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







































   
   

















    