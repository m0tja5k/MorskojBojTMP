#include "WindowManager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    WindowManager windowManager;
    windowManager.showAuthWindow();

    return a.exec();
}
