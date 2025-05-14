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
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GameWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QLabel *statusLabel;
    QHBoxLayout *fieldLayout;
    QWidget *playerFieldWidget;
    QWidget *enemyFieldWidget;
    QHBoxLayout *buttonLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *readyButton;
    QSpacerItem *horizontalSpacer_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *GameWindow)
    {
        if (GameWindow->objectName().isEmpty())
            GameWindow->setObjectName("GameWindow");
        GameWindow->resize(700, 600);
        centralwidget = new QWidget(GameWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName("verticalLayout");
        statusLabel = new QLabel(centralwidget);
        statusLabel->setObjectName("statusLabel");
        statusLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout->addWidget(statusLabel);

        fieldLayout = new QHBoxLayout();
        fieldLayout->setObjectName("fieldLayout");
        playerFieldWidget = new QWidget(centralwidget);
        playerFieldWidget->setObjectName("playerFieldWidget");
        playerFieldWidget->setMinimumSize(QSize(300, 300));

        fieldLayout->addWidget(playerFieldWidget);

        enemyFieldWidget = new QWidget(centralwidget);
        enemyFieldWidget->setObjectName("enemyFieldWidget");
        enemyFieldWidget->setMinimumSize(QSize(300, 300));

        fieldLayout->addWidget(enemyFieldWidget);


        verticalLayout->addLayout(fieldLayout);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName("buttonLayout");
        horizontalSpacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonLayout->addItem(horizontalSpacer);

        readyButton = new QPushButton(centralwidget);
        readyButton->setObjectName("readyButton");

        buttonLayout->addWidget(readyButton);

        horizontalSpacer_2 = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        buttonLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(buttonLayout);

        GameWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(GameWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 700, 26));
        GameWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(GameWindow);
        statusbar->setObjectName("statusbar");
        GameWindow->setStatusBar(statusbar);

        retranslateUi(GameWindow);

        QMetaObject::connectSlotsByName(GameWindow);
    } // setupUi

    void retranslateUi(QMainWindow *GameWindow)
    {
        GameWindow->setWindowTitle(QCoreApplication::translate("GameWindow", "\320\234\320\276\321\200\321\201\320\272\320\276\320\271 \320\261\320\276\320\271", nullptr));
        statusLabel->setText(QCoreApplication::translate("GameWindow", "\320\240\320\260\321\201\321\201\321\202\320\260\320\262\321\214\321\202\320\265 \320\272\320\276\321\200\320\260\320\261\320\273\320\270", nullptr));
        readyButton->setText(QCoreApplication::translate("GameWindow", "\320\223\320\276\321\202\320\276\320\262 \320\272 \320\261\320\276\321\216", nullptr));
    } // retranslateUi

};

namespace Ui {
    class GameWindow: public Ui_GameWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GAMEWINDOW_H
