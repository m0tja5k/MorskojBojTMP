/**
 * @file DatabaseManager.h
 * @brief Заголовочный файл для класса DatabaseManager, отвечающего за управление базой данных.
 *
 * Класс DatabaseManager реализует шаблон Singleton для обеспечения единственного экземпляра
 * для работы с базой данных.  Он предоставляет методы для подключения к базе данных,
 * добавления пользователей, вывода информации о пользователях и работы с данными игры,
 * включая создание игр, сохранение кораблей и ходов, а также проверку результатов выстрелов.
 */
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

/**
 * @brief Класс, управляющий подключением к базе данных и взаимодействием с ней.
 *
 * Класс DatabaseManager реализует шаблон Singleton для обеспечения единственного экземпляра.
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Получить экземпляр класса DatabaseManager (Singleton).
     * @return Указатель на экземпляр DatabaseManager.
     */
    static DatabaseManager* getInstance();

    /**
     * @brief Получить объект QSqlDatabase для взаимодействия с базой данных.
     * @return Объект QSqlDatabase.
     */
    QSqlDatabase getDatabase();

    /**
     * @brief Добавить нового пользователя в базу данных.
     * @param nickname Ник пользователя.
     * @param email Электронная почта пользователя.
     * @param password Пароль пользователя.
     * @return true, если пользователь успешно добавлен, иначе false.
     */
    bool addUser(const QString &nickname, const QString &email, const QString &password);

    /**
     * @brief Вывести информацию о всех пользователях в консоль (для отладки).
     */
    void printUsers();

    // Методы для работы с игрой
    /**
     * @brief Создать новую игру.
     * @param player1 Ник первого игрока.
     * @param player2 Ник второго игрока.
     * @return ID созданной игры или -1 в случае ошибки.
     */
    int createGame(const QString &player1, const QString &player2); // Создание новой игры с инициализацией первого хода

    /**
     * @brief Сохранить информацию о корабле в базе данных.
     * @param gameId ID игры.
     * @param player Ник игрока.
     * @param x Координата X корабля.
     * @param y Координата Y корабля.
     * @param size Размер корабля.
     * @param isHorizontal Ориентация корабля (true - горизонтально, false - вертикально).
     * @return true, если корабль успешно сохранен, иначе false.
     */
    bool saveShip(int gameId, const QString &player, int x, int y, int size, bool isHorizontal); // Сохранение корабля

    /**
     * @brief Сохранить информацию о ходе игры.
     * @param gameId ID игры.
     * @param player Ник игрока, сделавшего ход.
     * @param x Координата X выстрела.
     * @param y Координата Y выстрела.
     * @param result Результат выстрела ("hit", "miss", "sunk").
     * @return true, если ход успешно сохранен, иначе false.
     */
    bool saveMove(int gameId, const QString &player, int x, int y, const QString &result); // Сохранение хода

    /**
     * @brief Проверить результат выстрела.
     * @param gameId ID игры.
     * @param player Ник игрока, сделавшего ход.
     * @param x Координата X выстрела.
     * @param y Координата Y выстрела.
     * @return Результат выстрела ("hit", "miss", "sunk", "already_hit").
     */
    QString checkMove(int gameId, const QString &player, int x, int y); // Проверка результата выстрела

    /**
     * @brief Получить текущий ход игры.
     * @param gameId ID игры.
     * @return Ник игрока, чей ход сейчас.
     */
    QString getCurrentTurn(int gameId); // Получение текущего хода

    /**
     * @brief Обновить текущий ход игры.
     * @param gameId ID игры.
     * @param nextPlayer Ник игрока, чей ход следующий.
     * @return true, если ход успешно обновлен, иначе false.
     */
    bool updateTurn(int gameId, const QString &nextPlayer); // Обновление текущего хода

private:
    /**
     * @brief Приватный конструктор.  Используется только внутри класса для создания экземпляра (Singleton).
     */
    DatabaseManager();

    /**
     * @brief Приватный деструктор.
     */
    virtual ~DatabaseManager();

    /**
     * @brief Приватный конструктор копирования (удален).
     * @param other Объект для копирования (не используется).
     */
    DatabaseManager(const DatabaseManager&) = delete;

    /**
     * @brief Приватный оператор присваивания (удален).
     * @param other Объект для присваивания (не используется).
     * @return Не возвращает значение (оператор удален).
     */
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    /**
     * @brief Статический указатель на единственный экземпляр класса.
     */
    static DatabaseManager* instance;

    /**
     * @brief Объект QSqlDatabase для подключения к базе данных.
     */
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
