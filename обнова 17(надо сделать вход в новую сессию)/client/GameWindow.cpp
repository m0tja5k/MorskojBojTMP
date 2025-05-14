#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "WindowManager.h"
#include "NetworkClient.h"

GameWindow::GameWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    QPoint lastPos = WindowManager::getLastWindowPosition();
    if (!lastPos.isNull()) {
        this->move(lastPos);
    }

    playerField.resize(10, QVector<CellState>(10, Empty));
    enemyField.resize(10, QVector<CellState>(10, Empty));

    setupPlayerField();
    setupEnemyField();

    for (QPushButton* btn : enemyCells) {
        btn->setEnabled(false);
        btn->setStyleSheet("background-color: lightgray;");
    }

    connect(ui->readyButton, &QPushButton::clicked, this, &GameWindow::readyToFight);
    connect(&NetworkClient::instance(), &NetworkClient::gameReady, this, [](int gameId, const QString &opponent) {
        Q_UNUSED(opponent);
        qDebug() << "Received game_id:" << gameId;
    });
    connect(&NetworkClient::instance(), &NetworkClient::gameStarted, this, &GameWindow::onGameStarted);
    connect(&NetworkClient::instance(), &NetworkClient::moveResult, this, &GameWindow::onMoveResult);
    connect(&NetworkClient::instance(), &NetworkClient::shipPlacedSuccessfully, this, &GameWindow::onShipPlacedSuccessfully);
    connect(&NetworkClient::instance(), &NetworkClient::shipPlacementFailed, this, &GameWindow::onShipPlacementFailed);
    connect(&NetworkClient::instance(), &NetworkClient::allShipsPlaced, this, &GameWindow::onAllShipsPlaced);
    connect(&NetworkClient::instance(), &NetworkClient::updateUIEnabled, this, &GameWindow::updateEnemyFieldEnabled);
    connect(&NetworkClient::instance(), &NetworkClient::ownMoveResult, this, &GameWindow::onOwnMoveResult);
    connect(&NetworkClient::instance(), &NetworkClient::gameOver, this, &GameWindow::onGameOver);
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
        if (playerField[row][col] == Empty) {
            playerField[row][col] = Ship;
            btn->setStyleSheet("background-color: darkgreen;");
        } else if (playerField[row][col] == Ship) {
            playerField[row][col] = Empty;
            btn->setStyleSheet("background-color: lightblue;");
        }
    } else if (enemyCells.contains(btn) && enemyField[row][col] == Empty) {
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

    // Проверка корректности кораблей
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

    if (placedShips[3] != 1 || placedShips[2] != 2 ||
        placedShips[1] != 3 || placedShips[0] != 4) {
        QMessageBox::warning(this, "Ошибка", "Неправильное количество кораблей!\nТребуется: 1x4, 2x3, 3x2, 4x1");
        return;
    }

    // Проверка расстояния между кораблями
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

    qDebug() << "Blocking player field for editing";
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(false);
        if (playerField[btn->property("row").toInt()][btn->property("col").toInt()] == Ship) {
            btn->setStyleSheet("background-color: darkgreen;");
        } else {
            btn->setStyleSheet("background-color: gray;");
        }
    }


    visited.fill(QVector<bool>(10, false));
    int gameId = NetworkClient::instance().getGameId();
    if (gameId == -1) {
        QMessageBox::warning(this, "Ошибка", "Game ID не установлен!");
        unlockPlayerField();
        return;
    }

    // Отправка кораблей на сервер
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

                qDebug() << "Queueing ship: gameId=" << gameId << ", x=" << col << ", y=" << row
                         << ", length=" << length << ", horizontal=" << horizontal;
                NetworkClient::instance().queueShip(gameId, col, row, length, horizontal);
            }
        }
    }

    ui->statusLabel->setText("Отправка кораблей на сервер...");
    ui->readyButton->setEnabled(false);
}

void GameWindow::onShipPlacedSuccessfully()
{
    ui->statusLabel->setText("Корабли успещно размещены!");
}

void GameWindow::onShipPlacementFailed(const QString &reason)
{
    QMessageBox::warning(this, "Ошибка", "Не удалось разместить корабль: " + reason);
    unlockPlayerField();
    ui->statusLabel->setText("Ошибка размещения корабля");
    ui->readyButton->setEnabled(true);
}

void GameWindow::onAllShipsPlaced()
{
    int gameId = NetworkClient::instance().getGameId();
    if (gameId == -1) {
        QMessageBox::warning(this, "Ошибка", "Game ID не установлен!");
        unlockPlayerField();
        ui->statusLabel->setText("Ошибка");
        ui->readyButton->setEnabled(true);
        return;
    }
    NetworkClient::instance().readyToBattle(gameId);
    ui->statusLabel->setText("Ожидание противника...");
}

void GameWindow::unlockPlayerField()
{
    qDebug() << "Unlocking player field";
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(true);
        btn->setStyleSheet(playerField[btn->property("row").toInt()][btn->property("col").toInt()] == Ship ?
                               "background-color: darkgreen;" : "background-color: lightblue;");
    }
}

void GameWindow::onGameStarted(const QString &currentTurn)
{
    qDebug() << "Game started received, current turn:" << currentTurn;
    updateStatusLabel(currentTurn);

    // Блокируем поле игрока
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(false);
        if (playerField[btn->property("row").toInt()][btn->property("col").toInt()] == Ship) {
            btn->setStyleSheet("background-color: darkgreen;");
        } else {
            btn->setStyleSheet("background-color: gray;");
        }
    }

    // Активируем поле противника только для игрока, чей ход
    QString nickname = NetworkClient::instance().getCurrentNickname();
    updateEnemyFieldEnabled(currentTurn == nickname);
    ui->statusLabel->setText("Ход игрока: " + nickname);
}

void GameWindow::onGameOver(const QString &message)
{
    qDebug() << "Game over received with message:" << message;
    QString nickname = NetworkClient::instance().getCurrentNickname();

    // Проверяем, победил ли игрок
    bool isWinner = message.contains(nickname, Qt::CaseInsensitive) && message.contains("победил", Qt::CaseInsensitive);

    // Обновляем строку состояния
    ui->statusLabel->setText(isWinner ? "Вы победили!" : "Вы проиграли!");

    // Блокируем оба поля
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(false);
    }
    for (QPushButton *btn : enemyCells) {
        btn->setEnabled(false);
    }

    // Показываем диалоговое окно с результатом и предложением начать новую игру
    QMessageBox::StandardButton reply = QMessageBox::information(
        this,
        "Игра окончена",
        isWinner ? "Поздравляем! Вы победили!\nХотите начать новую игру?" : "Вы проиграли!\nХотите начать новую игру?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        clearFields();
        NetworkClient::instance().requestStartGame();
        ui->statusLabel->setText("Ожидание начала новой игры...");
        ui->readyButton->setEnabled(true);
    }
    if (reply == QMessageBox::No) {
        this->hide();
        emit backToMainMenu();
    }
}

void GameWindow::onMoveResult(const QString &status, int x, int y, const QString &message)
{
    int row = y;
    int col = x;
    QPushButton *btn = nullptr;
    for (QPushButton *cell : playerCells) { // Изменено: playerCells вместо enemyCells
        if (cell->property("row").toInt() == row && cell->property("col").toInt() == col) {
            btn = cell;
            break;
        }
    }

    if (btn) {
        if (status == "hit" || status == "sunk") {
            playerField[row][col] = Hit;
            btn->setStyleSheet("background-color: red;");
            btn->setText("X");
            ui->statusLabel->setText(status == "sunk" ? "Ваш корабль потоплен!" : "Ваш корабль подбит!");
        } else if (status == "miss") {
            playerField[row][col] = Miss;
            btn->setStyleSheet("background-color: white;");
            btn->setText("•");
            ui->statusLabel->setText("Противник промахнулся!");
        }
    }
}

void GameWindow::onOwnMoveResult(const QString &status, int x, int y, const QString &message)
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
            ui->statusLabel->setText(status == "sunk" ? "Корабль противника потоплен!" : "Попадание!");
        } else if (status == "miss") {
            enemyField[row][col] = Miss;
            btn->setStyleSheet("background-color: white;");
            btn->setText("•");
            ui->statusLabel->setText("Промах!");
        }
    }
}

void GameWindow::updateEnemyFieldEnabled(bool enabled)
{
    qDebug() << "Updating enemy field enabled:" << enabled;
    for (QPushButton *btn : enemyCells) {
        btn->setEnabled(enabled);
    }
    QString nickname = NetworkClient::instance().getCurrentNickname();
    QString opponent = nickname == "1" ? "2" : "1"; // Предполагаем никнеймы для корректного отображения
    updateStatusLabel(enabled ? nickname : opponent);
}

void GameWindow::updateStatusLabel(const QString &currentTurn)
{
    QString nickname = NetworkClient::instance().getCurrentNickname();
    if (currentTurn == nickname) {
        ui->statusLabel->setStyleSheet("background-color: lightgreen; color: black; font-weight: bold;");
    } else {
        ui->statusLabel->setStyleSheet("background-color: lightcoral; color: black; font-weight: bold;");
    }
}

void GameWindow::clearFields()
{
    qDebug() << "Clearing fields and unlocking player field";
    if (playerCells.size() != 100 || enemyCells.size() != 100) {
        qDebug() << "Error: Invalid cell lists size. playerCells:" << playerCells.size()
        << ", enemyCells:" << enemyCells.size();
        return;
    }

    for (int i = 0; i < playerCells.size(); ++i) {
        playerCells[i]->setStyleSheet("background-color: lightblue;");
        playerCells[i]->setEnabled(true);
        playerCells[i]->setText("");
        enemyCells[i]->setStyleSheet("background-color: lightgray;");
        enemyCells[i]->setEnabled(false);
        enemyCells[i]->setText("");
    }
    playerField.fill(QVector<CellState>(10, Empty));
    enemyField.fill(QVector<CellState>(10, Empty));
}

GameWindow::~GameWindow()
{
    delete ui;
}
