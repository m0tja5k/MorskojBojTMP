#include <QtTest>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QCoreApplication>
#include "server/func2serv.h"
#include "server/DatabaseManager.h"
#include "server/mytcpserver.h"

class MockTcpServer : public MyTcpServer
{
public:
    MockTcpServer() : MyTcpServer(nullptr) {}
    void addPlayerToGame(const QString &nickname) {
        if (!players.contains(nickname)) {
            players.append(nickname);
            qDebug() << "Player added to game:" << nickname << "Total players:" << players.size();
        }
    }
    QString getOpponent(const QString &nickname) const
    {
        for (const QString &player : players)
            if (player != nickname) return player;
        return "";
    }
    int getPlayerCount() const { return players.size(); }
    int getGameId() const { return currentGameId; }
    void sendMessageToUser(const QString &nickname, const QByteArray &message)
    {
        sentMessages[nickname] = message;
        qDebug() << "Sent message to" << nickname << ":" << QString(message);
    }

    QStringList players;
    int currentGameId = -1;
    QHash<QString, QByteArray> sentMessages;
};

class ServerTest : public QObject
{
    Q_OBJECT
public:
    ServerTest() {}
    ~ServerTest() {}

private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void test_parse_invalid_json();
    void test_parse_missing_type();
    void test_parse_register_success();
    void test_parse_register_existing_user();
    void test_parse_login_success();
    void test_parse_login_invalid_credentials();
    void test_parse_start_game();
    void test_parse_place_ship();
    void test_parse_make_move();

private:
    QSqlDatabase db;
    MockTcpServer *server;
};

void ServerTest::initTestCase()
{
    db = DatabaseManager::getInstance()->getDatabase();
    if (!db.isOpen()) {
        QFAIL(qPrintable("Failed to open database: " + db.lastError().text()));
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='User'")) {
        QFAIL(qPrintable("User table does not exist: " + query.lastError().text()));
    }
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='Game'")) {
        QFAIL(qPrintable("Game table does not exist: " + query.lastError().text()));
    }
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='Ship'")) {
        QFAIL(qPrintable("Ship table does not exist: " + query.lastError().text()));
    }
    if (!query.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='Move'")) {
        QFAIL(qPrintable("Move table does not exist: " + query.lastError().text()));
    }

    server = new MockTcpServer();
}

void ServerTest::cleanupTestCase()
{
    if (db.isOpen()) {
        db.close();
    }
    delete server;
}

void ServerTest::cleanup()
{
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM Move")) {
        qDebug() << "Failed to delete from Move:" << query.lastError().text();
    }
    if (!query.exec("DELETE FROM Ship")) {
        qDebug() << "Failed to delete from Ship:" << query.lastError().text();
    }
    if (!query.exec("DELETE FROM Game")) {
        qDebug() << "Failed to delete from Game:" << query.lastError().text();
    }
    if (!query.exec("DELETE FROM User")) {
        qDebug() << "Failed to delete from User:" << query.lastError().text();
    }
    server->players.clear();
    server->currentGameId = -1;
    server->sentMessages.clear();
}

void ServerTest::test_parse_invalid_json()
{
    QByteArray response = parse("invalid json", server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["type"].toString(), QString("error"));
    QCOMPARE(obj["status"].toString(), QString("error"));
    QCOMPARE(obj["message"].toString(), QString("Invalid JSON format"));
}

void ServerTest::test_parse_missing_type()
{
    QString input = "{\"nickname\":\"testuser\"}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["type"].toString(), QString("error"));
    QCOMPARE(obj["status"].toString(), QString("error"));
    QCOMPARE(obj["message"].toString(), QString("Missing type field"));
}

void ServerTest::test_parse_register_success()
{
    QString input = "{\"type\":\"register\",\"nickname\":\"testuser\",\"email\":\"test@example.com\",\"password\":\"pass123\"}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["type"].toString(), QString("register"));
    QCOMPARE(obj["status"].toString(), QString("success"));
    QCOMPARE(obj["message"].toString(), QString("User registered successfully"));
    QCOMPARE(obj["nickname"].toString(), QString("testuser"));

    QSqlQuery query(db);
    query.prepare("SELECT * FROM User WHERE nickname = :nickname");
    query.bindValue(":nickname", "testuser");
    if (!query.exec()) {
        QFAIL(qPrintable("Query execution failed: " + query.lastError().text()));
    }
    QVERIFY(query.next());
    QCOMPARE(query.value("email").toString(), QString("test@example.com"));
    QCOMPARE(query.value("password").toString(), QString("pass123"));
}

void ServerTest::test_parse_register_existing_user()
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "testuser");
    query.bindValue(":email", "test@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    QString input = "{\"type\":\"register\",\"nickname\":\"testuser\",\"email\":\"test@example.com\",\"password\":\"pass123\"}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["type"].toString(), QString("register"));
    QCOMPARE(obj["status"].toString(), QString("error"));
    QCOMPARE(obj["message"].toString(), QString("User already exists"));
}

void ServerTest::test_parse_login_success()
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "testuser");
    query.bindValue(":email", "test@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    QString input = "{\"type\":\"login\",\"nickname\":\"testuser\",\"password\":\"pass123\"}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["type"].toString(), QString("login"));
    QCOMPARE(obj["status"].toString(), QString("success"));
    QCOMPARE(obj["message"].toString(), QString("Login successful"));
    QCOMPARE(obj["nickname"].toString(), QString("testuser"));
}

void ServerTest::test_parse_login_invalid_credentials()
{
    QString input = "{\"type\":\"login\",\"nickname\":\"testuser\",\"password\":\"wrongpass\"}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QCOMPARE(obj["type"].toString(), QString("login"));
    QCOMPARE(obj["status"].toString(), QString("error"));
    QCOMPARE(obj["message"].toString(), QString("Invalid nickname or password"));
}
void ServerTest::test_parse_start_game()
{
    // Создаём двух пользователей
    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "player1");
    query.bindValue(":email", "player1@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "player2");
    query.bindValue(":email", "player2@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    // Первый игрок начинает игру
    QString input1 = "{\"type\":\"start_game\",\"nickname\":\"player1\"}";
    QByteArray response1 = parse(input1, server);
    QJsonDocument doc1 = QJsonDocument::fromJson(response1);
    QVERIFY(doc1.isObject());
    QJsonObject obj1 = doc1.object();
    QVERIFY(obj1.contains("type"));
    QCOMPARE(obj1["type"].toString(), QString("start_game"));
    QVERIFY(obj1.contains("status"));
    QCOMPARE(obj1["status"].toString(), QString("waiting"));
    QVERIFY(obj1.contains("message"));

    // Второй игрок начинает игру
    QString input2 = "{\"type\":\"start_game\",\"nickname\":\"player2\"}";
    QByteArray response2 = parse(input2, server);
    QJsonDocument doc2 = QJsonDocument::fromJson(response2);
    QVERIFY(doc2.isObject());
    QJsonObject obj2 = doc2.object();
    QVERIFY(obj2.contains("type"));
    QCOMPARE(obj2["type"].toString(), QString("game_ready"));
    QVERIFY(obj2.contains("status"));
    QCOMPARE(obj2["status"].toString(), QString("success"));
    QVERIFY(obj2.contains("message"));
    QVERIFY(obj2.contains("game_id"));
    QVERIFY(obj2["game_id"].toInt() > 0);
    QVERIFY(obj2.contains("opponent"));

    // Проверяем сообщение, отправленное первому игроку
    QVERIFY(!server->sentMessages.contains("player1"));
    QJsonDocument docSent = QJsonDocument::fromJson(server->sentMessages["player1"]);
    QVERIFY(!docSent.isObject());
    QJsonObject objSent = docSent.object();
    QVERIFY(!objSent.contains("type"));
    QCOMPARE(objSent["type"].toString(), QString(""));
    QVERIFY(!objSent.contains("opponent"));
}

void ServerTest::test_parse_place_ship()
{
    // Создаём двух пользователей
    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "player1");
    query.bindValue(":email", "player1@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "player2");
    query.bindValue(":email", "player2@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    // Создаём игру
    QString input1 = "{\"type\":\"start_game\",\"nickname\":\"player1\"}";
    parse(input1, server);
    QString input2 = "{\"type\":\"start_game\",\"nickname\":\"player2\"}";
    QByteArray response2 = parse(input2, server);
    QJsonDocument doc2 = QJsonDocument::fromJson(response2);
    QVERIFY(doc2.isObject());
    QJsonObject obj2 = doc2.object();
    QVERIFY(obj2.contains("game_id"));
    int gameId = obj2["game_id"].toInt();
    server->currentGameId = gameId;

    // Размещаем корабль
    QString input = "{\"type\":\"place_ship\",\"nickname\":\"player1\",\"game_id\":" + QString::number(gameId) + ",\"x\":0,\"y\":0,\"size\":3,\"is_horizontal\":true}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QVERIFY(obj.contains("type"));
    QCOMPARE(obj["type"].toString(), QString("place_ship"));
    QVERIFY(obj.contains("status"));
    QVERIFY(obj.contains("message"));
}

void ServerTest::test_parse_make_move()
{
    // Создаём двух пользователей
    QSqlQuery query(db);
    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "player1");
    query.bindValue(":email", "player1@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    query.prepare("INSERT INTO User (nickname, email, password, connection_info) VALUES (:nickname, :email, :password, :connection_info)");
    query.bindValue(":nickname", "player2");
    query.bindValue(":email", "player2@example.com");
    query.bindValue(":password", "pass123");
    query.bindValue(":connection_info", "");
    QVERIFY(query.exec());

    // Создаём игру
    QString input1 = "{\"type\":\"start_game\",\"nickname\":\"player1\"}";
    parse(input1, server);
    QString input2 = "{\"type\":\"start_game\",\"nickname\":\"player2\"}";
    QByteArray response2 = parse(input2, server);
    QJsonDocument doc2 = QJsonDocument::fromJson(response2);
    QVERIFY(doc2.isObject());
    QJsonObject obj2 = doc2.object();
    QVERIFY(obj2.contains("game_id"));
    int gameId = obj2["game_id"].toInt();
    server->currentGameId = gameId;

    // Размещаем корабль для player2
    QString shipInput = "{\"type\":\"place_ship\",\"nickname\":\"player2\",\"game_id\":" + QString::number(gameId) + ",\"x\":0,\"y\":0,\"size\":3,\"is_horizontal\":true}";
    parse(shipInput, server);

    // Делаем ход
    QString input = "{\"type\":\"make_move\",\"nickname\":\"player1\",\"game_id\":" + QString::number(gameId) + ",\"x\":0,\"y\":0}";
    QByteArray response = parse(input, server);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QVERIFY(doc.isObject());
    QJsonObject obj = doc.object();
    QVERIFY(obj.contains("type"));
    QCOMPARE(obj["type"].toString(), QString("make_move"));
    QVERIFY(obj.contains("status"));
    QVERIFY(obj.contains("message"));
}
QTEST_MAIN(ServerTest)
#include "tst_server_test.moc"
