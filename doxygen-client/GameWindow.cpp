/**
 * @file GameWindow.cpp
 * @brief Реализация класса GameWindow.
 *
 *  Этот файл содержит реализацию методов класса GameWindow,
 *  отвечающего за отображение и управление игровым процессом.
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */

#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "NetworkClient.h"

/**
 * @brief Конструктор класса GameWindow.
 *
 *  Инициализирует игровое окно, настраивает пользовательский интерфейс,
 *  создает игровое поле для игрока и противника, подключает сигналы и слоты.
 *
 * @param parent Указатель на родительский виджет.
 */
GameWindow::GameWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    /**
     * @brief Инициализация игровых полей.
     *
     * Создаются двумерные векторы, представляющие игровое поле игрока и поле противника.
     * Все ячейки изначально находятся в состоянии Empty.
     */
    playerField.resize(10, QVector<CellState>(10, Empty));
    enemyField.resize(10, QVector<CellState>(10, Empty));

    /**
     * @brief Настройка игрового поля игрока.
     *
     *  Создание кнопок для каждой ячейки игрового поля игрока, настройка их внешнего вида
     *  и подключение сигнала clicked к слоту handleCellClick.
     */
    setupPlayerField();

    /**
     * @brief Настройка игрового поля противника.
     *
     *  Создание кнопок для каждой ячейки игрового поля противника, настройка их внешнего вида
     *  и подключение сигнала clicked к слоту handleCellClick.
     */
    setupEnemyField();

    /**
     * @brief Подключение сигнала readyButton::clicked к слоту readyToFight.
     *
     * При нажатии на кнопку "Готов" вызывается слот readyToFight.
     */
    connect(ui->readyButton, &QPushButton::clicked, this, &GameWindow::readyToFight);

    /**
     * @brief Подключение сигнала NetworkClient::gameReady к анонимному лямбда-слоту.
     *
     *  При получении сигнала gameReady от NetworkClient выводится отладочное сообщение
     *  с идентификатором игры.
     */
    connect(&NetworkClient::instance(), &NetworkClient::gameReady, this, [](int gameId, const QString &opponent) {
        Q_UNUSED(opponent);
        qDebug() << "Received game_id:" << gameId;
    });

    /**
     * @brief Подключение сигнала NetworkClient::gameStarted к слоту onGameStarted.
     *
     *  При получении сигнала gameStarted от NetworkClient вызывается слот onGameStarted.
     */
    connect(&NetworkClient::instance(), &NetworkClient::gameStarted, this, &GameWindow::onGameStarted);

    /**
     * @brief Подключение сигнала NetworkClient::moveResult к слоту onMoveResult.
     *
     *  При получении сигнала moveResult от NetworkClient вызывается слот onMoveResult.
     */
    connect(&NetworkClient::instance(), &NetworkClient::moveResult, this, &GameWindow::onMoveResult);

    /**
     * @brief Подключение сигнала NetworkClient::shipPlacedSuccessfully к слоту onShipPlacedSuccessfully.
     *
     * При успешной установке корабля (сигнал `shipPlacedSuccessfully`) вызывается слот `onShipPlacedSuccessfully`.
     */
    connect(&NetworkClient::instance(), &NetworkClient::shipPlacedSuccessfully, this, &GameWindow::onShipPlacedSuccessfully);

    /**
     * @brief Подключение сигнала NetworkClient::shipPlacementFailed к слоту onShipPlacementFailed.
     *
     * При неудачной установке корабля (сигнал `shipPlacementFailed`) вызывается слот `onShipPlacementFailed`.
     */
    connect(&NetworkClient::instance(), &NetworkClient::shipPlacementFailed, this, &GameWindow::onShipPlacementFailed);

    /**
     * @brief Подключение сигнала NetworkClient::allShipsPlaced к слоту onAllShipsPlaced.
     *
     * Когда все корабли размещены (сигнал `allShipsPlaced`), вызывается слот `onAllShipsPlaced`.
     */
    connect(&NetworkClient::instance(), &NetworkClient::allShipsPlaced, this, &GameWindow::onAllShipsPlaced);

    /**
     * @brief Подключение сигнала NetworkClient::updateUIEnabled к слоту updateEnemyFieldEnabled.
     *
     * Сигнал NetworkClient::updateUIEnabled подключается к слоту GameWindow::updateEnemyFieldEnabled,
     * чтобы обновлять доступность (enabled) пользовательского интерфейса для поля противника.
     */
    connect(&NetworkClient::instance(), &NetworkClient::updateUIEnabled, this, &GameWindow::updateEnemyFieldEnabled);

    /**
     * @brief Подключение сигнала NetworkClient::ownMoveResult к слоту onOwnMoveResult.
     *
     * При получении результата собственного хода (сигнал `ownMoveResult`) вызывается слот `onOwnMoveResult`.
     */
    connect(&NetworkClient::instance(), &NetworkClient::ownMoveResult, this, &GameWindow::onOwnMoveResult); // Новый сигнал
}

/**
 * @brief Настраивает игровое поле игрока.
 *
 *  Создает сетку кнопок для игрового поля игрока, устанавливает их внешний вид
 *  и подключает сигнал clicked к слоту handleCellClick.
 */
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

/**
 * @brief Настраивает игровое поле противника.
 *
 *  Создает сетку кнопок для игрового поля противника, устанавливает их внешний вид
 *  и подключает сигнал clicked к слоту handleCellClick.
 */
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

/**
 * @brief Обрабатывает нажатие на ячейку игрового поля.
 *
 *  Определяет, какая ячейка была нажата, и выполняет соответствующие действия.
 *  В зависимости от состояния игры (расстановка кораблей, ход игрока, ход противника)
 *  может отправлять запрос на установку корабля, запрос на ход или игнорировать нажатие.
 */
void GameWindow::handleCellClick()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int row = btn->property("row").toInt();
    int col = btn->property("col").toInt();

/**
 * @file GameWindow.cpp
 * @brief Реализация класса GameWindow (продолжение).
 *
 *  Этот файл содержит реализацию методов класса GameWindow,
 *  отвечающего за отображение и управление игровым процессом (продолжение).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "NetworkClient.h"

/**
 * @brief Обрабатывает нажатие на ячейку игрового поля.
 *
 *  Определяет, какая ячейка была нажата, и выполняет соответствующие действия.
 *  В зависимости от состояния игры (расстановка кораблей, ход игрока, ход противника)
 *  может отправлять запрос на установку корабля, запрос на ход или игнорировать нажатие.
 */
void GameWindow::handleCellClick()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    int row = btn->property("row").toInt();
    int col = btn->property("col").toInt();

    /**
     * @brief Обработка нажатия на ячейку поля игрока.
     *
     *  Если нажата ячейка на поле игрока, изменяет состояние ячейки (Ship/Empty)
     *  и перекрашивает кнопку.
     */
    if (playerCells.contains(btn)) {
        if (playerField[row][col] == Empty) {
            playerField[row][col] = Ship;
            btn->setStyleSheet("background-color: darkgreen;");
        } else if (playerField[row][col] == Ship) {
            playerField[row][col] = Empty;
            btn->setStyleSheet("background-color: lightblue;");
        }
    }
    /**
     * @brief Обработка нажатия на ячейку поля противника.
     *
     *  Если нажата ячейка на поле противника, проверяет, началась ли игра.
     *  Если игра началась и ячейка пуста (Empty), отправляет запрос на ход
     *  на сервер.
     */
    else if (enemyCells.contains(btn) && enemyField[row][col] == Empty) {
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

/**
 * @brief Обработчик нажатия на кнопку "Готов".
 *
 *  Проверяет расстановку кораблей игрока на корректность и отправляет запрос
 *  на начало игры на сервер, если расстановка правильная.
 */
void GameWindow::readyToFight()
{
    QVector<int> placedShips(4, 0);
    QVector<QVector<bool>> visited(10, QVector<bool>(10, false));

    /**
     * @brief Проверка корректности расстановки кораблей.
     *
     *  Проверяет, соответствует ли количество кораблей каждого размера правилам игры.
     *  Использует алгоритм для определения длин кораблей и подсчета их количества.
     */
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

    /**
     * @brief Проверка количества кораблей каждого размера.
     *
     *  Убеждается, что количество кораблей соответствует правилам игры.
     *  Если количество не соответствует, выдается сообщение об ошибке.
     */
    if (placedShips[3] != 1 || placedShips[2] != 2 ||
        placedShips[1] != 3 || placedShips[0] != 4) {
        QMessageBox::warning(this, "Ошибка", "Неправильное количество кораблей!\nТребуется: 1x4, 2x3, 3x2, 4x1");
        return;
    }

    // Проверка расстояния между кораблями  (добавим комментарии здесь позже, когда будет реализация)
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            // ...
        }
    }
}

/**
 * @file GameWindow.cpp
 * @brief Реализация класса GameWindow (продолжение).
 *
 *  Этот файл содержит реализацию методов класса GameWindow,
 *  отвечающего за отображение и управление игровым процессом (продолжение).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "NetworkClient.h"

/**
 * @brief Обработчик нажатия на кнопку "Готов".
 *
 *  Проверяет расстановку кораблей игрока на корректность и отправляет запрос
 *  на начало игры на сервер, если расстановка правильная.
 */
void GameWindow::readyToFight()
{
    QVector<int> placedShips(4, 0);
    QVector<QVector<bool>> visited(10, QVector<bool>(10, false));

    /**
     * @brief Проверка корректности расстановки кораблей.
     *
     *  Проверяет, соответствует ли количество кораблей каждого размера правилам игры.
     *  Использует алгоритм для определения длин кораблей и подсчета их количества.
     */
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

    /**
     * @brief Проверка количества кораблей каждого размера.
     *
     *  Убеждается, что количество кораблей соответствует правилам игры.
     *  Если количество не соответствует, выдается сообщение об ошибке.
     */
    if (placedShips[3] != 1 || placedShips[2] != 2 ||
        placedShips[1] != 3 || placedShips[0] != 4) {
        QMessageBox::warning(this, "Ошибка", "Неправильное количество кораблей!\nТребуется: 1x4, 2x3, 3x2, 4x1");
        return;
    }

    /**
     * @brief Проверка расстояния между кораблями.
     *
     *  Убеждается, что между кораблями есть расстояние минимум в одну клетку.
     *  Если корабли расположены слишком близко друг к другу, выдается сообщение об ошибке.
     */
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

    /**
     * @brief Блокировка поля игрока для редактирования.
     *
     *  После проверки корректности расстановки кораблей поле игрока блокируется
     *  для дальнейшего редактирования. Кнопки на поле игрока становятся неактивными.
     */
    qDebug() << "Blocking player field for editing";
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(false);
        if (playerField[btn->property("row").toInt()][btn->property("col").toInt()] == Ship) {
            btn->setStyleSheet("background-color: darkgreen;");
        } else {
            btn->setStyleSheet("background-color: gray;");
        }
    }

    /**
     * @brief Получение идентификатора игры.
     *
     *  Получает идентификатор игры из NetworkClient. Если идентификатор не установлен,
     *  выводит сообщение об ошибке и разблокирует поле игрока.
     */
    visited.fill(QVector<bool>(10, false));
    int gameId = NetworkClient::instance().getGameId();
    if (gameId == -1) {
        QMessageBox::warning(this, "Ошибка", "Game ID не установлен!");
        unlockPlayerField();
        return;
    }

    /**
     * @brief Отправка информации о расстановке кораблей на сервер.
     */
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

/**
 * @file GameWindow.cpp
 * @brief Реализация класса GameWindow (продолжение).
 *
 *  Этот файл содержит реализацию методов класса GameWindow,
 *  отвечающего за отображение и управление игровым процессом (продолжение).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "NetworkClient.h"

/**
 * @brief Обработчик нажатия на кнопку "Готов".
 *
 *  Проверяет расстановку кораблей игрока на корректность и отправляет запрос
 *  на начало игры на сервер, если расстановка правильная.
 */
void GameWindow::readyToFight()
{
    QVector<int> placedShips(4, 0);
    QVector<QVector<bool>> visited(10, QVector<bool>(10, false));

    /**
     * @brief Проверка корректности расстановки кораблей.
     *
     *  Проверяет, соответствует ли количество кораблей каждого размера правилам игры.
     *  Использует алгоритм для определения длин кораблей и подсчета их количества.
     */
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

                /**
                 * @brief Отправка данных о корабле на сервер.
                 *
                 * Отправляет на сервер информацию о расположении корабля (gameId, x, y, length, horizontal).
                 * Данные отправляются через метод queueShip класса NetworkClient.
                 */
                qDebug() << "Queueing ship: gameId=" << gameId << ", x=" << col << ", y=" << row
                         << ", length=" << length << ", horizontal=" << horizontal;
                NetworkClient::instance().queueShip(gameId, col, row, length, horizontal);
            }
        }
    }

    /**
     * @brief Обновление статуса и блокировка кнопки "Готов".
     *
     *  После отправки всех кораблей на сервер обновляется текст на statusLabel
     *  и блокируется кнопка readyButton, чтобы предотвратить повторную отправку.
     */
    ui->statusLabel->setText("Отправка кораблей на сервер...");
    ui->readyButton->setEnabled(false);
}

/**
 * @brief Обработчик сигнала успешного размещения корабля на сервере.
 *
 *  Обновляет текст на statusLabel, сообщая об успешном размещении корабля.
 */
void GameWindow::onShipPlacedSuccessfully()
{
    ui->statusLabel->setText("Корабль успешно размещён. Отправка следующего...");
}

/**
 * @brief Обработчик сигнала неудачного размещения корабля на сервере.
 *
 *  Выводит сообщение об ошибке, разблокирует поле игрока и обновляет текст на statusLabel.
 *
 * @param reason Причина, по которой не удалось разместить корабль.
 */
void GameWindow::onShipPlacementFailed(const QString &reason)
{
    QMessageBox::warning(this, "Ошибка", "Не удалось разместить корабль: " + reason);
    unlockPlayerField();
    ui->statusLabel->setText("Ошибка размещения корабля");
    ui->readyButton->setEnabled(true);
}

/**
 * @brief Обработчик сигнала успешной установки всех кораблей.
 *
 *  Получает идентификатор игры, вызывает метод readyToBattle класса NetworkClient
 *  и обновляет текст на statusLabel.
 */
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

/**
 * @brief Разблокирует поле игрока для редактирования.
 *
 *  Позволяет игроку снова редактировать расстановку кораблей.
 *  Обновляет стиль кнопок на поле игрока в зависимости от состояния ячейки.
 */
void GameWindow::unlockPlayerField()
{
    qDebug() << "Unlocking player field";
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(true);
        btn->setStyleSheet(playerField[btn->property("row").toInt()][btn->property("col").toInt()] == Ship ?
                               "background-color: darkgreen;" : "background-color: lightblue;");
    }
}

/**
 * @brief Обработчик сигнала начала игры.
 *
 *  Обновляет текст на statusLabel, блокирует поле игрока для редактирования
 *  и активирует поле противника, если ход текущего игрока.
 *
 * @param currentTurn Никнейм игрока, чей сейчас ход.
 */
void GameWindow::onGameStarted(const QString &currentTurn)
{
    qDebug() << "Game started received, current turn:" << currentTurn;
    updateStatusLabel(currentTurn);

    /**
     * @brief Блокировка поля игрока для редактирования.
     *
     *  После начала игры поле игрока блокируется для дальнейшего редактирования.
     *  Кнопки на поле игрока становятся неактивными.
     */
    for (QPushButton *btn : playerCells) {
        btn->setEnabled(false);
        if (playerField[btn->property("row").toInt()][btn->property("col").toInt()] == Ship) {
            btn->setStyleSheet("background-color: darkgreen;");
        } else {
            btn->setStyleSheet("background-color: gray;");
        }
    }

    /**
     * @brief Активация поля противника только для игрока, чей ход.
     *
     * Поле противника становится доступным для нажатий только в том случае,
     * если никнейм текущего игрока (`currentTurn`) совпадает с никнеймом
     * текущего пользователя.
     */
    QString nickname = NetworkClient::instance().getCurrentNickname();
    updateEnemyFieldEnabled(currentTurn == nickname);
}

/**
 * @brief Обработчик сигнала о результате хода (чужого хода).
 *
 *  Обновляет визуальное состояние ячейки на поле игрока в соответствии с результатом хода противника.
 *
 * @param status  "hit" или "miss", указывающий результат хода противника.
 * @param x X-координата ячейки, по которой был произведен выстрел.
 * @param y Y-координата ячейки, по которой был произведен выстрел.
 * @param message Сообщение о результате хода.
 */
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
/**
 * @file GameWindow.cpp
 * @brief Реализация класса GameWindow (окончание).
 *
 *  Этот файл содержит реализацию методов класса GameWindow,
 *  отвечающего за отображение и управление игровым процессом (окончание).
 *
 * @author [Ваше имя]
 * @date [Дата создания или последнего изменения]
 */
#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QMessageBox>
#include <algorithm>
#include "NetworkClient.h"

/**
 * @brief Обработчик сигнала о результате хода (чужого хода).
 *
 *  Обновляет визуальное состояние ячейки на поле игрока в соответствии с результатом хода противника.
 *
 * @param status "hit" или "miss", указывающий результат хода противника.
 * @param x X-координата ячейки, по которой был произведен выстрел.
 * @param y Y-координата ячейки, по которой был произведен выстрел.
 * @param message Сообщение о результате хода.
 */
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

    /**
     * @brief Обновление состояния ячейки на поле игрока в зависимости от результата хода.
     */
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

/**
 * @brief Обработчик сигнала о результате собственного хода.
 *
 *  Обновляет визуальное состояние ячейки на поле противника в соответствии с результатом собственного хода.
 *
 * @param status "hit" или "miss", указывающий результат собственного хода.
 * @param x X-координата ячейки, по которой был произведен выстрел.
 * @param y Y-координата ячейки, по которой был произведен выстрел.
 * @param message Сообщение о результате хода.
 */
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

    /**
     * @brief Обновление состояния ячейки на поле противника в зависимости от результата хода.
     */
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

/**
 * @brief Обновляет доступность поля противника для совершения хода.
 *
 *  Активирует или деактивирует кнопки на поле противника, в зависимости от того,
 *  чей сейчас ход. Также вызывает обновление текста на statusLabel.
 *
 * @param enabled True, если поле противника должно быть активно, False - если нет.
 */
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

/**
 * @brief Обновляет текст на statusLabel в зависимости от текущего хода.
 *
 *  Устанавливает текст на statusLabel в зависимости от того, чей сейчас ход,
 *  отображая "Ваш ход!" или "Ход противника: [никнейм]".
 *
 * @param currentTurn Никнейм игрока, чей сейчас ход.
 */
void GameWindow::updateStatusLabel(const QString &currentTurn)
{
    QString nickname = NetworkClient::instance().getCurrentNickname();
    if (currentTurn == nickname) {
        ui->statusLabel->setText("Ваш ход!");
    } else {
        ui->statusLabel->setText("Ход противника: " + currentTurn);
    }
}

/**
 * @brief Очищает поля игрока и противника.
 *
 *  Сбрасывает состояние всех ячеек на полях игрока и противника в исходное состояние.
 *  Также разблокирует поле игрока для редактирования.
 */
void GameWindow::clearFields()
{
    qDebug() << "Clearing fields and unlocking player field";
    if (playerCells.size() != 100 || enemyCells.size() != 100) {
        qDebug() << "Error: Invalid cell lists size. playerCells:" << playerCells.size()
                 << ", enemyCells:" << enemyCells.size();
        return;
    }

    /**
     * @brief Сброс визуального состояния ячеек.
     */
    for (int i = 0; i < playerCells.size(); ++i) {
        playerCells[i]->setStyleSheet("background-color: lightblue;");
        playerCells[i]->setEnabled(true);
        playerCells[i]->setText("");
        enemyCells[i]->setStyleSheet("background-color: lightgray;");
        enemyCells[i]->setEnabled(true);
        enemyCells[i]->setText("");
    }

    /**
     * @brief Сброс логического состояния полей.
     */
    playerField.fill(QVector<CellState>(10, Empty));
    enemyField.fill(QVector<CellState>(10, Empty));
}  
GameWindow::~GameWindow()
{
    delete ui;
}
