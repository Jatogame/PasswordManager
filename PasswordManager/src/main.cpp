#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Open Main Window
    MainWindow w;
    //Open the Main Window locked
    w.open_locked();

    w.show();
    return a.exec();
}
