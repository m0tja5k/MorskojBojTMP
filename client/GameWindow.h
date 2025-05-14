/**
 * @file GameWindow.h
 * @brief Объявление класса GameWindow.
 *
 *  Этот файл содержит объявление класса GameWindow, который представляет
 *  игровое окно приложения "Морской бой". Он включает в себя объявление сигналов,
 *  слотов, а также методы для управления игровым процессом и UI.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H
#include "ui_GameWindow.h"
#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>

/**
 * @class GameWindow
 * @brief Класс, представляющий игровое окно приложения "Морской бой".
 *
 *  Этот класс отвечает за отображение игрового поля, обработку действий пользователя
 *  (расстановка кораблей, совершение ходов), а также взаимодействие с сервером.
 */
class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор класса GameWindow.
     *
     *  Инициализирует окно, настраивает UI, создает поля игрока и противника,
     *  а также подключает необходимые сигналы и слоты.
     *
     * @param parent Указатель на родительский виджет (по умолчанию nullptr).
     */
    explicit GameWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор класса GameWindow.
     *
     * Освобождает ресурсы, выделенные для пользовательского интерфейса.
     */
    ~GameWindow();

    /**
     * @brief Обработчик сигнала о начале игры.
     *
     *  Вызывается при получении сигнала о начале игры с сервера. Обновляет UI
     *  в соответствии с начальным состоянием игры.
     *
     * @param currentTurn Никнейм игрока, чей сейчас ход.
     */
    void onGameStarted(const QString &currentTurn);

    /**
     * @brief Обработчик сигнала о результате хода.
     *
     *  Вызывается при получении результата хода (чужого) от сервера. Обновляет UI
     *  в соответствии с результатом хода.
     *
     * @param status "hit" или "miss", указывающий результат хода противника.
     * @param x X-координата ячейки, по которой был произведен выстрел.
     * @param y Y-координата ячейки, по которой был произведен выстрел.
     * @param message Сообщение о результате хода.
     */
    void onMoveResult(const QString &status, int x, int y, const QString &message);

private slots:
    /**
     * @brief Слот для обработки нажатия на ячейку игрового поля.
     *
     *  Вызывается при нажатии на ячейку на игровом поле (поле игрока или противника).
     *  Обрабатывает действие в зависимости от состояния игры и типа поля.
     */
    void handleCellClick();

    /**
     * @brief Слот для обработки нажатия на кнопку "Готов".
     *
     *  Вызывается при нажатии кнопки "Готов". Проверяет расстановку кораблей
     *  и отправляет информацию на сервер.
     */
    void readyToFight();

    /**
     * @brief Слот для обработки сигнала об успешном размещении корабля.
     */
    void onShipPlacedSuccessfully();

    /**
     * @brief Слот для обработки сигнала о неудачном размещении корабля.
     *
     * @param reason Причина, по которой не удалось разместить корабль.
     */
    void onShipPlacementFailed(const QString &reason);

    /**
     * @brief Слот для обработки сигнала об успешной установке всех кораблей.
     */
    void onAllShipsPlaced();

    /**
     * @brief Обновляет доступность (enabled) поля противника в UI.
     *
     * @param enabled True, если поле противника должно быть активно, False - если нет.
     */
    void updateEnemyFieldEnabled(bool enabled);

    /**
     * @brief Слот для обновления текста на statusLabel.
     *
     * @param currentTurn Никнейм игрока, чей сейчас ход.
     */
    void updateStatusLabel(const QString &currentTurn);

    /**
     * @brief Обработчик сигнала о результате собственного хода.
     *
     * @param status "hit" или "miss", указывающий результат собственного хода.
     * @param x X-координата ячейки, по которой был произведен выстрел.
     * @param y Y-координата ячейки, по которой был произведен выстрел.
     * @param message Сообщение о результате хода.
     */
    void onOwnMoveResult(const QString &status, int x, int y, const QString &message); // Новый слот

private:
    /**
     * @brief Создает кнопки для игрового поля игрока.
     */
    void setupPlayerField();

    /**
     * @brief Создает кнопки для игрового поля противника.
     */
    void setupEnemyField();

    /**
     * @brief Очищает поля игрока и противника.
     */
    void clearFields();

    /**
     * @brief Разблокирует поле игрока для редактирования.
     */
    void unlockPlayerField();

    /**
     * @brief Перечисление, определяющее состояние ячейки игрового поля.
     */
    enum CellState {
        Empty,     ///< Ячейка пуста.
        Ship,      ///< В ячейке находится корабль.
        Miss,      ///< В ячейку был произведен промах.
        Hit,       ///< В ячейку попали.
        Forbidden  ///< Ячейка находится рядом с кораблем (запрещена для установки корабля).
    };

    /**
     * @brief Двумерный вектор, представляющий игровое поле игрока.
     */
    QVector<QVector<CellState>> playerField;

    /**
     * @brief Двумерный вектор, представляющий игровое поле противника.
     */
    QVector<QVector<CellState>> enemyField;

    /**
     * @brief Список кнопок, представляющих ячейки на поле игрока.
     */
    QList<QPushButton*> playerCells;

    /**
     * @brief Список кнопок, представляющих ячейки на поле противника.
     */
    QList<QPushButton*> enemyCells;

    /**
     * @brief Указатель на объект пользовательского интерфейса (UI).
     */
    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
