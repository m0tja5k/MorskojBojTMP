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

private slots:
    void handleCellClick();
    void readyToFight();  // Убран слот placeShip()

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
