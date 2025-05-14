#include <QCoreApplication>
#include "mytcpserver.h" // Включаем заголовочный файл MyTcpServer

/**
 * @brief Главная функция приложения.
 *
 * Создает и запускает сервер MyTcpServer.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 * @return Код завершения приложения.
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);  // Создаем экземпляр QCoreApplication.  Он необходим для обработки событий.

    MyTcpServer myserv;  // Создаем экземпляр MyTcpServer.  Сервер будет обрабатывать сетевые подключения.

    return a.exec();  // Запускаем цикл обработки событий.  Приложение завершится, когда будет вызван a.quit() или произойдет другое завершение.
}