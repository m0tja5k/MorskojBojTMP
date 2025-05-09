#ifndef FUNC2SERV_H
#define FUNC2SERV_H

#include <QByteArray>
#include <QString>

// Функция обработки запросов
QByteArray parse(const QString &input);

// Функции работы с БД
bool parseRegisterData(const QString &data, QString &nickname, QString &email, QString &password);
bool parseLoginData(const QString &data, QString &nickname, QString &password);
QByteArray handleRegister(const QString &data);
QByteArray slotLogin(const QString &data);

#endif // FUNC2SERV_H
