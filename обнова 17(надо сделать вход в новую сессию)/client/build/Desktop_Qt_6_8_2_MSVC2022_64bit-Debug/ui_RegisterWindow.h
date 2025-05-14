/********************************************************************************
** Form generated from reading UI file 'RegisterWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTERWINDOW_H
#define UI_REGISTERWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RegisterWindow
{
public:
    QLabel *label;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QLineEdit *nicknameEdit;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QLineEdit *confirmPasswordEdit;
    QPushButton *registerButton;
    QPushButton *backButton;

    void setupUi(QWidget *RegisterWindow)
    {
        if (RegisterWindow->objectName().isEmpty())
            RegisterWindow->setObjectName("RegisterWindow");
        RegisterWindow->resize(437, 498);
        label = new QLabel(RegisterWindow);
        label->setObjectName("label");
        label->setGeometry(QRect(170, 20, 91, 20));
        verticalLayoutWidget = new QWidget(RegisterWindow);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(0, 70, 441, 431));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        nicknameEdit = new QLineEdit(verticalLayoutWidget);
        nicknameEdit->setObjectName("nicknameEdit");

        verticalLayout->addWidget(nicknameEdit);

        emailEdit = new QLineEdit(verticalLayoutWidget);
        emailEdit->setObjectName("emailEdit");

        verticalLayout->addWidget(emailEdit);

        passwordEdit = new QLineEdit(verticalLayoutWidget);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        verticalLayout->addWidget(passwordEdit);

        confirmPasswordEdit = new QLineEdit(verticalLayoutWidget);
        confirmPasswordEdit->setObjectName("confirmPasswordEdit");
        confirmPasswordEdit->setEchoMode(QLineEdit::EchoMode::Password);

        verticalLayout->addWidget(confirmPasswordEdit);

        registerButton = new QPushButton(verticalLayoutWidget);
        registerButton->setObjectName("registerButton");

        verticalLayout->addWidget(registerButton);

        backButton = new QPushButton(verticalLayoutWidget);
        backButton->setObjectName("backButton");

        verticalLayout->addWidget(backButton);


        retranslateUi(RegisterWindow);

        QMetaObject::connectSlotsByName(RegisterWindow);
    } // setupUi

    void retranslateUi(QWidget *RegisterWindow)
    {
        RegisterWindow->setWindowTitle(QCoreApplication::translate("RegisterWindow", "Form", nullptr));
        label->setText(QCoreApplication::translate("RegisterWindow", "\320\240\320\265\320\263\320\270\321\201\321\202\321\200\320\260\321\206\320\270\321\217", nullptr));
        nicknameEdit->setPlaceholderText(QCoreApplication::translate("RegisterWindow", "nickname", nullptr));
        emailEdit->setPlaceholderText(QCoreApplication::translate("RegisterWindow", "email", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("RegisterWindow", "password", nullptr));
        confirmPasswordEdit->setPlaceholderText(QCoreApplication::translate("RegisterWindow", "confirm password", nullptr));
        registerButton->setText(QCoreApplication::translate("RegisterWindow", "\320\227\320\260\321\200\320\265\320\263\320\270\321\201\321\202\321\200\320\270\321\200\320\276\320\262\320\260\321\202\321\214\321\201\321\217", nullptr));
        backButton->setText(QCoreApplication::translate("RegisterWindow", "\320\235\320\260\320\267\320\260\320\264", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RegisterWindow: public Ui_RegisterWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTERWINDOW_H
