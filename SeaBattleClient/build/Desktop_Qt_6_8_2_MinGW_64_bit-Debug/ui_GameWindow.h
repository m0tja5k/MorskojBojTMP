/********************************************************************************
** Form generated from reading UI file 'GameWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GAMEWINDOW_H
#define UI_GAMEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameWindow
{
public:
    QLabel *label;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_2;
    QLabel *statusLabel;
    QPushButton *surrenderButton;

    void setupUi(QWidget *GameWindow)
    {
        if (GameWindow->objectName().isEmpty())
            GameWindow->setObjectName("GameWindow");
        GameWindow->resize(400, 300);
        label = new QLabel(GameWindow);
        label->setObjectName("label");
        label->setGeometry(QRect(140, 10, 131, 41));
        gridLayoutWidget = new QWidget(GameWindow);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(0, 110, 160, 80));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setContentsMargins(0, 0, 0, 0);
        gridLayoutWidget_2 = new QWidget(GameWindow);
        gridLayoutWidget_2->setObjectName("gridLayoutWidget_2");
        gridLayoutWidget_2->setGeometry(QRect(230, 110, 160, 80));
        gridLayout_2 = new QGridLayout(gridLayoutWidget_2);
        gridLayout_2->setObjectName("gridLayout_2");
        gridLayout_2->setContentsMargins(0, 0, 0, 0);
        statusLabel = new QLabel(GameWindow);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setGeometry(QRect(150, 70, 101, 20));
        surrenderButton = new QPushButton(GameWindow);
        surrenderButton->setObjectName("surrenderButton");
        surrenderButton->setGeometry(QRect(150, 210, 93, 29));

        retranslateUi(GameWindow);

        QMetaObject::connectSlotsByName(GameWindow);
    } // setupUi

    void retranslateUi(QWidget *GameWindow)
    {
        GameWindow->setWindowTitle(QCoreApplication::translate("GameWindow", "Form", nullptr));
        label->setText(QCoreApplication::translate("GameWindow", "\320\234\320\276\321\200\321\201\320\272\320\276\320\271 \320\261\320\276\320\271", nullptr));
        statusLabel->setText(QCoreApplication::translate("GameWindow", "\321\201\321\202\320\260\321\202\321\203\321\201 \320\270\320\263\321\200\321\213", nullptr));
        surrenderButton->setText(QCoreApplication::translate("GameWindow", "\320\241\320\264\320\260\321\202\321\214\321\201\321\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GameWindow: public Ui_GameWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMEWINDOW_H
