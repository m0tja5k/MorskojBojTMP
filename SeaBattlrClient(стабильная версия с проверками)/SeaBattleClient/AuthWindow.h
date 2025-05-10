#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class AuthWindow; }
QT_END_NAMESPACE

class AuthWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

signals:
    void loginRequested(const QString &email, const QString &password);
    void registerRequested();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();

private:
    Ui::AuthWindow *ui;
};

#endif // AUTHWINDOW_H
