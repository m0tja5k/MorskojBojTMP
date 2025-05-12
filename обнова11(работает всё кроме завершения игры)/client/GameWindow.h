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
    void onShipPlacedSuccessfully();
    void onShipPlacementFailed(const QString &reason);
    void onAllShipsPlaced();
    void updateEnemyFieldEnabled(bool enabled);
    void updateStatusLabel(const QString &currentTurn);
    void onOwnMoveResult(const QString &status, int x, int y, const QString &message); // Новый слот

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
        Forbidden
    };

    QVector<QVector<CellState>> playerField;
    QVector<QVector<CellState>> enemyField;
    QList<QPushButton*> playerCells;
    QList<QPushButton*> enemyCells;

    Ui::GameWindow *ui;
};

#endif // GAMEWINDOW_H
