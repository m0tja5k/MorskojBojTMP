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

private slots:
    void handleCellClick();
    void readyToFight();
    void onShipPlacedSuccessfully(); // Новый слот
    void onShipPlacementFailed(const QString &reason); // Новый слот
    void onAllShipsPlaced(); // Новый слот

private:
    void setupPlayerField();
    void setupEnemyField();
    void clearFields();
    void unlockPlayerField(); // Новый метод

    enum CellState {
        Empty,
        Ship,
        Miss,
        Hit,
        Forbidden
    };

    QVector<QVector<CellState>> playerField;
    QVector<QVector<CellState>> enemyField;
    QList<QPushButton*> playerCells;
    QList<QPushButton*> enemyCells;

    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
