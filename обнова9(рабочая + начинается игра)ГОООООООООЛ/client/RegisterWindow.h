#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterWindow; }
QT_END_NAMESPACE

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void registrationRequested(const QString &nickname, const QString &email,
                               const QString &password, const QString &confirmPassword);
    void backToAuthRequested();

private slots:
    void on_registerButton_clicked();
    void on_backButton_clicked();

private:
    Ui::RegisterWindow *ui;
};

#endif // REGISTERWINDOW_H
