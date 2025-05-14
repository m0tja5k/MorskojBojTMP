/**
 * @file func2serv.h
 * @brief Заголовочный файл для функций, обрабатывающих запросы от клиентов.
 *
 * Содержит объявления функций, используемых для парсинга JSON-данных,
 * обработки команд, взаимодействия с базой данных и формирования ответов.
 */
#ifndef FUNC2SERV_H
#define FUNC2SERV_H

#include <QByteArray>
#include <QString>
#include <class MyTcpServer> // Исправлено: Добавлено #include для класса MyTcpServer (необходимо для объявления parse)

// Forward declaration для класса MyTcpServer, чтобы избежать зависимости
class MyTcpServer;

/**
 * @brief Функция обработки запросов.
 *
 * Парсит входящую JSON строку, определяет тип команды и вызывает
 * соответствующую функцию обработки.
 *
 * @param input Входящая строка, содержащая JSON запрос.
 * @param server Указатель на экземпляр класса MyTcpServer.
 * @return QByteArray JSON ответ.
 */
QByteArray parse(const QString &input, MyTcpServer *server);

/**
 * @brief Парсит данные регистрации из JSON строки.
 *
 * Извлекает данные регистрации (никнейм, email, пароль) из JSON строки.
 *
 * @param data JSON строка с данными регистрации.
 * @param nickname Ссылка на QString для сохранения никнейма.
 * @param email Ссылка на QString для сохранения email.
 * @param password Ссылка на QString для сохранения пароля.
 * @return bool true - при успешном парсинге, false - в противном случае.
 */
bool parseRegisterData(const QString &data, QString &nickname, QString &email, QString &password);

/**
 * @brief Парсит данные логина из JSON строки.
 *
 * Извлекает данные логина (никнейм, пароль) из JSON строки.
 *
 * @param data JSON строка с данными логина.
 * @param nickname Ссылка на QString для сохранения никнейма.
 * @param password Ссылка на QString для сохранения пароля.
 * @return bool true - при успешном парсинге, false - в противном случае.
 */
bool parseLoginData(const QString &data, QString &nickname, QString &password);

/**
 * @brief Обрабатывает запрос на регистрацию пользователя.
 *
 * @param data JSON строка с данными регистрации.
 * @return QByteArray JSON ответ.
 */
QByteArray handleRegister(const QString &data);

/**
 * @brief Обрабатывает запрос на вход пользователя в систему (логин).
 *
 * @param data JSON строка с данными логина.
 * @return QByteArray JSON ответ.
 */
QByteArray slotLogin(const QString &data);

/**
 * @brief Обрабатывает запрос на начало игры.
 *
 * @param data JSON строка с данными запроса на начало игры.
 * @param server Указатель на экземпляр класса MyTcpServer.
 * @return QByteArray JSON ответ.
 */
QByteArray handleStartGame(const QString &data, MyTcpServer *server);

/**
 * @brief Обрабатывает запрос на размещение корабля на игровом поле.
 *
 * @param data JSON строка с данными о размещении корабля.
 * @param server Указатель на экземпляр класса MyTcpServer.
 * @return QByteArray JSON ответ.
 */
QByteArray handlePlaceShip(const QString &data, MyTcpServer *server);

/**
 * @brief Обрабатывает запрос на совершение хода в игре.
 *
 * @param data JSON строка с данными о ходе.
 * @param server Указатель на экземпляр класса MyTcpServer.
 * @return QByteArray JSON ответ.
 */
QByteArray handleMakeMove(const QString &data, MyTcpServer *server);

/**
 * @brief Создает JSON ответ.
 *
 * @param type Тип ответа (например, "register", "login", "error").
 * @param status Статус ответа ("success", "error").
 * @param message Сообщение, содержащее информацию об ответе.
 * @return QByteArray JSON ответ.
 */
QByteArray createJsonResponse(const QString &type, const QString &status, const QString &message);

#endif // FUNC2SERV_H