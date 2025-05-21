#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H
#include "ui_GameWindow.h"
#include <QMainWindow>
#include <QPushButton>
#include <QGridLayout>

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow();
    void onGameStarted(const QString &currentTurn);
    void onMoveResult(const QString &status, int x, int y, const QString &message);

signals:
    void backToMainMenu();

private slots:
    void markSunkenShip(int row, int col, bool isPlayerField);
    void updateCellColor(int row, int col, QList<QPushButton*>& cells, bool isSunk);
    void handleCellClick();
    void readyToFight();
    void onShipPlacedSuccessfully();
    void onShipPlacementFailed(const QString &reason);
    void onAllShipsPlaced();
    void updateEnemyFieldEnabled(bool enabled);
    void updateStatusLabel(const QString &currentTurn);
    void onOwnMoveResult(const QString &status, int x, int y, const QString &message);
    void onGameOver(const QString &message);

private:
    void setupPlayerField();
    void setupEnemyField();
    void clearFields();
    void unlockPlayerField();

    enum CellState {
        Empty,
        Ship,
        Miss,
        Hit,
        Sunk,
        Forbidden
    };

    QVector<QVector<CellState>> playerField;
    QVector<QVector<CellState>> enemyField;
    QList<QPushButton*> playerCells;
    QList<QPushButton*> enemyCells;

    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
