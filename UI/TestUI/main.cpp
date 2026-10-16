/**
 * @file main.cpp
 * @brief Starts the Qt drawing prototype.
 * @author Cole Schwandt
 */

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char * argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
