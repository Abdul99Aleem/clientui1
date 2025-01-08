//main.cpp
#include "mainwindow.h"
#include "conferancecallwindow.h"
#include "clientwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
