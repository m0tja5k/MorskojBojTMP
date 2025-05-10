#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <QRandomGenerator>

GameWindow::GameWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    // Инициализация полей 10x10
    playerField.resize(10, QVector<CellState>(10, Empty));
    enemyField.resize(10, QVector<CellState>(10, Empty));

    setupPlayerField();
    setupEnemyField();

    // Кнопки управления
    connect(ui->placeShipsButton, &QPushButton::clicked, this, &GameWindow::placeShip);
    connect(ui->readyButton, &QPushButton::clicked, this, &GameWindow::readyToFight);
}

void GameWindow::setupPlayerField()
{
    QGridLayout *grid = new QGridLayout(ui->playerFieldWidget);
    grid->setSpacing(1);

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            QPushButton *btn = new QPushButton();
            btn->setFixedSize(30, 30);
            btn->setProperty("row", row);
            btn->setProperty("col", col);
            btn->setStyleSheet("background-color: lightblue;");

            connect(btn, &QPushButton::clicked, this, &GameWindow::handleCellClick);

            grid->addWidget(btn, row, col);
            playerCells.append(btn);
        }
    }
}

void GameWindow::setupEnemyField()
{
    QGridLayout *grid = new QGridLayout(ui->enemyFieldWidget);
    grid->setSpacing(1);

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            QPushButton *btn = new QPushButton();
            btn->setFixedSize(30, 30);
            btn->setProperty("row", row);
            btn->setProperty("col", col);
            btn->setStyleSheet("background-color: lightgray;");

            connect(btn, &QPushButton::clicked, this, &GameWindow::handleCellClick);

            grid->addWidget(btn, row, col);
            enemyCells.append(btn);
        }
    }
}

void GameWindow::handleCellClick()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int row = btn->property("row").toInt();
    int col = btn->property("col").toInt();

    // Определяем, какое поле было нажато
    bool isPlayerField = playerCells.contains(btn);

    if (isPlayerField) {
        // Логика для игрового поля игрока
        if (playerField[row][col] == Empty) {
            playerField[row][col] = Ship;
            btn->setStyleSheet("background-color: darkgreen;");
        } else if (playerField[row][col] == Ship) {
            playerField[row][col] = Empty;
            btn->setStyleSheet("background-color: lightblue;");
        }
    } else {
        // Логика для вражеского поля (ход игрока)
        if (enemyField[row][col] == Empty) {
            enemyField[row][col] = Miss;
            btn->setStyleSheet("background-color: white;");
            btn->setText("•");
        } else if (enemyField[row][col] == Ship) {
            enemyField[row][col] = Hit;
            btn->setStyleSheet("background-color: red;");
            btn->setText("X");
        }
    }
}

void GameWindow::placeShipRandomly(int size)
{
    bool placed = false;
    int attempts = 0;
    const int maxAttempts = 100; // Чтобы избежать бесконечного цикла

    while (!placed && attempts < maxAttempts) {
        attempts++;
        int row = QRandomGenerator::global()->bounded(10);
        int col = QRandomGenerator::global()->bounded(10);
        bool horizontal = QRandomGenerator::global()->bounded(2);

        // Проверка выхода за границы поля
        if (horizontal) {
            if (col + size > 10) continue; // Корабль не помещается по горизонтали
        } else {
            if (row + size > 10) continue; // Корабль не помещается по вертикали
        }

        // Проверка пересечения с другими кораблями и соседними клетками
        bool canPlace = true;
        for (int i = -1; i <= size; ++i) {
            for (int j = -1; j <= 1; ++j) {
                int checkRow, checkCol;

                if (horizontal) {
                    checkRow = row + j;
                    checkCol = col + i;
                } else {
                    checkRow = row + i;
                    checkCol = col + j;
                }

                // Проверка выхода за границы при проверке соседей
                if (checkRow < 0 || checkRow >= 10 || checkCol < 0 || checkCol >= 10) {
                    continue;
                }

                // Если это часть корабля (i от 0 до size-1) и клетка занята
                if (i >= 0 && i < size && j == 0) {
                    if (playerField[checkRow][checkCol] != Empty) {
                        canPlace = false;
                        break;
                    }
                }
                // Проверка соседних клеток
                else if (playerField[checkRow][checkCol] == Ship) {
                    canPlace = false;
                    break;
                }
            }
            if (!canPlace) break;
        }

        if (!canPlace) continue;

        // Размещение корабля
        for (int i = 0; i < size; ++i) {
            if (horizontal) {
                playerField[row][col + i] = Ship;
                playerCells[row * 10 + (col + i)]->setStyleSheet("background-color: darkgreen;");
            } else {
                playerField[row + i][col] = Ship;
                playerCells[(row + i) * 10 + col]->setStyleSheet("background-color: darkgreen;");
            }
        }
        placed = true;
    }

    if (!placed) {
        qDebug() << "Не удалось разместить корабль размером" << size;
    }
}

void GameWindow::placeShip()
{
    clearFields();

    // Расстановка кораблей в порядке убывания размера
    placeShipRandomly(4); // 1 четырехпалубный
    for (int i = 0; i < 2; ++i) placeShipRandomly(3); // 2 трехпалубных
    for (int i = 0; i < 3; ++i) placeShipRandomly(2); // 3 двухпалубных
    for (int i = 0; i < 4; ++i) placeShipRandomly(1); // 4 однопалубных

    // Проверка количества кораблей (для отладки)
    checkShipsCount();
}

void GameWindow::checkShipsCount()
{
    int count[5] = {0}; // 0-4 - размеры кораблей

    // Проход по полю и подсчет...
    // (может быть полезно для проверки корректности расстановки)
}

void GameWindow::readyToFight()
{
    // Проверка количества кораблей
    QVector<int> requiredShips = {4, 3, 3, 2, 2, 2, 1, 1, 1, 1}; // 1x4, 2x3, 3x2, 4x1
    QVector<int> placedShips(4, 0); // Индексы: 0-1клетка, 1-2клетки, 2-3клетки, 3-4клетки

    // Подсчет расставленных кораблей
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            if (playerField[row][col] == Ship) {
                // Проверяем горизонтальный корабль
                if (col < 9 && playerField[row][col+1] == Ship) {
                    int length = 1;
                    while (col+length < 10 && playerField[row][col+length] == Ship) {
                        length++;
                    }
                    if (length > 4) {
                        QMessageBox::warning(this, "Ошибка", "Слишком длинный корабль!");
                        return;
                    }
                    placedShips[length-1]++;
                    col += length; // Пропускаем проверенные клетки
                    continue;
                }

                // Проверяем вертикальный корабль
                if (row < 9 && playerField[row+1][col] == Ship) {
                    int length = 1;
                    while (row+length < 10 && playerField[row+length][col] == Ship) {
                        length++;
                    }
                    if (length > 4) {
                        QMessageBox::warning(this, "Ошибка", "Слишком длинный корабль!");
                        return;
                    }
                    placedShips[length-1]++;
                    row += length; // Пропускаем проверенные клетки
                    continue;
                }

                // Одиночный корабль (1 клетка)
                placedShips[0]++;
            }
        }
    }

    // Проверка правильного количества кораблей
    if (placedShips[3] != 1 ||  // 1 корабль на 4 клетки
        placedShips[2] != 2 ||  // 2 корабля на 3 клетки
        placedShips[1] != 3 ||  // 3 корабля на 2 клетки
        placedShips[0] != 4) {  // 4 корабля на 1 клетку
        QMessageBox::warning(this, "Ошибка",
                             "Неправильное количество кораблей!\n"
                             "Требуется: 1x4, 2x3, 3x2, 4x1");
        return;
    }

    // Проверка расстояния между кораблями
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            if (playerField[row][col] == Ship) {
                // Проверяем соседние клетки
                for (int r = qMax(0, row-1); r <= qMin(9, row+1); ++r) {
                    for (int c = qMax(0, col-1); c <= qMin(9, col+1); ++c) {
                        if ((r != row || c != col) && playerField[r][c] == Ship) {
                            // Корабли стоят вплотную или по диагонали
                            QMessageBox::warning(this, "Ошибка",
                                                 "Корабли должны находиться на расстоянии минимум 1 клетки!");
                            return;
                        }
                    }
                }
            }
        }
    }

    // Все проверки пройдены
    ui->statusLabel->setText("Игра началась! Ваш ход.");
    ui->placeShipsButton->setEnabled(false);
    ui->readyButton->setEnabled(false);

    // Можно добавить сигнал о готовности к серверу
    // NetworkClient::instance().sendReadySignal();
}

void GameWindow::clearFields()
{
    for (int i = 0; i < 100; ++i) {
        playerCells[i]->setStyleSheet("background-color: lightblue;");
        enemyCells[i]->setStyleSheet("background-color: lightgray;");
    }
    playerField.fill(QVector<CellState>(10, Empty));
    enemyField.fill(QVector<CellState>(10, Empty));
}

GameWindow::~GameWindow()
{
    delete ui;
}
