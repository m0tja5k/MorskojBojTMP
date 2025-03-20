// MainWindow.h
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

private slots:
    void connectButtonClicked();
    void sendButtonClicked();
    void handleMessage(const QString&);
    void handleConnectionChanged(bool);
    void handleError(const QString&);

private:
    Ui::MainWindow *ui; // Исправлено: Uri → Ui, добавлен указатель *
    void updateUI();
};
