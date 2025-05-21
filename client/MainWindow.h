// MainWindow.h
#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }  // Исправлено: Uri → Ui
QT_END_NAMESPACE

// NetworkClient.h перенесен в MainWindow.cpp, если не используется здесь напрямую

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setUserInfo(const QString &nickname);

signals:
    void startGameRequested();

private slots:
    void on_startButton_clicked();
    void connectButtonClicked();
    void handleConnectionChanged(bool);
    void handleError(const QString&);

private:
    Ui::MainWindow *ui; // Исправлено: Uri → Ui, добавлен указатель *
    void updateUI();
    QString currentNickname;
};
