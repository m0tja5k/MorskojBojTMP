#include "basewindow.h"
#include <QCloseEvent>

BaseWindow::BaseWindow(QWidget *parent) : QMainWindow(parent)
{
    restoreWindowState();
}

BaseWindow::~BaseWindow() {}

void BaseWindow::closeEvent(QCloseEvent *event)
{
    saveWindowState();
    QMainWindow::closeEvent(event);
}

void BaseWindow::saveWindowState()
{
    QSettings settings("YourCompany", "YourApp");
    settings.beginGroup(windowName());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
}

void BaseWindow::restoreWindowState()
{
    QSettings settings("YourCompany", "YourApp");
    settings.beginGroup(windowName());
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    settings.endGroup();
}
