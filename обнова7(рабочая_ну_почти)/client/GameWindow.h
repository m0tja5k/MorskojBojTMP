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
    void onGameStarted(const QString &currentTurn); // Новый слот для game_start
    void onMoveResult(const QString &status, int x, int y, const QString &message); // Новый слот для move_result
private slots:
    void handleCellClick();
    void readyToFight();


private:
    void setupPlayerField();
    void setupEnemyField();
    void clearFields();

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
