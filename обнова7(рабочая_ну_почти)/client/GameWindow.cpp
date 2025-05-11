#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "NetworkClient.h"

GameWindow::GameWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    playerField.resize(10, QVector<CellState>(10, Empty));
    enemyField.resize(10, QVector<CellState>(10, Empty));

    setupPlayerField();
    setupEnemyField();

    connect(ui->readyButton, &QPushButton::clicked, this, &GameWindow::readyToFight);
    connect(&NetworkClient::instance(), &NetworkClient::gameReady, this, [](int gameId, const QString &opponent) {
        Q_UNUSED(opponent);
        qDebug() << "Received game_id:" << gameId;
    });
    // Подключаем новые сигналы
    connect(&NetworkClient::instance(), &NetworkClient::gameStarted, this, &GameWindow::onGameStarted);
    connect(&NetworkClient::instance(), &NetworkClient::moveResult, this, &GameWindow::onMoveResult);
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

    if (playerCells.contains(btn)) {
        // Размещение кораблей на своем поле (до readyToFight)
        if (playerField[row][col] == Empty) {
            playerField[row][col] = Ship;
            btn->setStyleSheet("background-color: darkgreen;");
        } else if (playerField[row][col] == Ship) {
            playerField[row][col] = Empty;
            btn->setStyleSheet("background-color: lightblue;");
        }
    } else if (enemyCells.contains(btn) && enemyField[row][col] == Empty) {
        // Ход по полю врага
        int gameId = NetworkClient::instance().getGameId();
        if (gameId == -1) {
            QMessageBox::warning(this, "Ошибка", "Игра не началась!");
            return;
        }
        QJsonObject json;
        json["type"] = "make_move";
        json["nickname"] = NetworkClient::instance().getCurrentNickname();
        json["game_id"] = gameId;
        json["x"] = col;
        json["y"] = row;
        NetworkClient::instance().sendMessage(QJsonDocument(json).toJson());
        qDebug() << "Sent make_move request: x=" << col << ", y=" << row;
    }
}

void GameWindow::readyToFight()
{
    QVector<int> placedShips(4, 0);
    QVector<QVector<bool>> visited(10, QVector<bool>(10, false));

    // Подсчет кораблей
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            if (playerField[row][col] == Ship && !visited[row][col]) {
                int length = 1;
                bool horizontal = false;

                if (col < 9 && playerField[row][col+1] == Ship) {
                    horizontal = true;
                    for (int c = col+1; c < 10 && playerField[row][c] == Ship; ++c) {
                        length++;
                    }
                } else if (row < 9 && playerField[row+1][col] == Ship) {
                    for (int r = row+1; r < 10 && playerField[r][col] == Ship; ++r) {
                        length++;
                    }
                }

                if (length > 4) {
                    QMessageBox::warning(this, "Ошибка", "Слишком длинный корабль!");
                    return;
                }

                if (horizontal) {
                    for (int c = col; c < col+length; ++c) visited[row][c] = true;
                } else {
                    for (int r = row; r < row+length; ++r) visited[r][col] = true;
                }

                if (length == 1) placedShips[0]++;
                else if (length == 2) placedShips[1]++;
                else if (length == 3) placedShips[2]++;
                else if (length == 4) placedShips[3]++;
            }
        }
    }

    // Проверка количества кораблей
    if (placedShips[3] != 1 || placedShips[2] != 2 ||
        placedShips[1] != 3 || placedShips[0] != 4) {
        QMessageBox::warning(this, "Ошибка", "Неправильное количество кораблей!\nТребуется: 1x4, 2x3, 3x2, 4x1");
        return;
    }

    // Проверка расстояния между кораблями
    QVector<QVector<bool>> forbidden(10, QVector<bool>(10, false));

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            if (playerField[row][col] == Ship) {
                for (int r = std::max(0, row-1); r <= std::min(9, row+1); ++r) {
                    for (int c = std::max(0, col-1); c <= std::min(9, col+1); ++c) {
                        forbidden[r][c] = true;
                    }
                }
            }
        }
    }

    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            if (playerField[row][col] == Ship) {
                for (int r = std::max(0, row-1); r <= std::min(9, row+1); ++r) {
                    for (int c = std::max(0, col-1); c <= std::min(9, col+1); ++c) {
                        if (playerField[r][c] == Ship && (r != row || c != col)) {
                            bool sameShip = false;
                            if (r == row) {
                                int minC = std::min(c, col);
                                int maxC = std::max(c, col);
                                sameShip = true;
                                for (int i = minC; i <= maxC; ++i) {
                                    if (playerField[row][i] != Ship) sameShip = false;
                                }
                            }
                            if (!sameShip && c == col) {
                                int minR = std::min(r, row);
                                int maxR = std::max(r, row);
                                sameShip = true;
                                for (int i = minR; i <= maxR; ++i) {
                                    if (playerField[i][col] != Ship) sameShip = false;
                                }
                            }

                            if (!sameShip) {
                                QMessageBox::warning(this, "Ошибка",
                                                     "Корабли должны находиться на расстоянии минимум 1 клетки!");
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    // Отправляем корабли на сервер
    visited.fill(QVector<bool>(10, false));
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            if (playerField[row][col] == Ship && !visited[row][col]) {
                int length = 1;
                bool horizontal = false;

                if (col < 9 && playerField[row][col+1] == Ship) {
                    horizontal = true;
                    for (int c = col+1; c < 10 && playerField[row][c] == Ship; ++c) {
                        length++;
                        visited[row][c] = true;
                    }
                } else if (row < 9 && playerField[row+1][col] == Ship) {
                    for (int r = row+1; r < 10 && playerField[r][col] == Ship; ++r) {
                        length++;
                        visited[r][col] = true;
                    }
                }
                visited[row][col] = true;

                int gameId = NetworkClient::instance().getGameId();
                if (gameId == -1) {
                    QMessageBox::warning(this, "Ошибка", "Game ID не установлен!");
                    return;
                }
                NetworkClient::instance().placeShip(gameId, col, row, length, horizontal);
            }
        }
    }

    // Отправляем ready_to_battle
    int gameId = NetworkClient::instance().getGameId();
    if (gameId == -1) {
        QMessageBox::warning(this, "Ошибка", "Game ID не установлен!");
        return;
    }
    NetworkClient::instance().readyToBattle(gameId);
    ui->statusLabel->setText("Ожидание противника...");
    ui->readyButton->setEnabled(false);

    // Блокируем поле игрока
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(false);
    }
}

void GameWindow::onGameStarted(const QString &currentTurn)
{
    ui->statusLabel->setText("Игра началась! Ходит: " + currentTurn);
}

void GameWindow::onMoveResult(const QString &status, int x, int y, const QString &message)
{
    int row = y;
    int col = x;
    QPushButton *btn = nullptr;
    for (QPushButton *cell : enemyCells) {
        if (cell->property("row").toInt() == row && cell->property("col").toInt() == col) {
            btn = cell;
            break;
        }
    }

    if (btn) {
        if (status == "hit" || status == "sunk") {
            enemyField[row][col] = Hit;
            btn->setStyleSheet("background-color: red;");
            btn->setText("X");
        } else if (status == "miss") {
            enemyField[row][col] = Miss;
            btn->setStyleSheet("background-color: white;");
            btn->setText("•");
        }
        ui->statusLabel->setText(message);
    }
}

void GameWindow::clearFields()
{
    for (int i = 0; i < 100; ++i) {
        playerCells[i]->setStyleSheet("background-color: lightblue;");
        enemyCells[i]->setStyleSheet("background-color: lightgray;");
        playerCells[i]->setEnabled(true); // Разблокируем поле при очистке
    }
    playerField.fill(QVector<CellState>(10, Empty));
    enemyField.fill(QVector<CellState>(10, Empty));
}

GameWindow::~GameWindow()
{
    delete ui;
}
