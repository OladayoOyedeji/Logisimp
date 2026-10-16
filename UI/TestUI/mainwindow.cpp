/**
 * @file mainwindow.cpp
 * @brief Creates the Qt prototype window and drawing canvas.
 * @author Cole Schwandt
 */

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "canvas.h"

MainWindow::MainWindow(QWidget * parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Logisimp");
    resize(1200, 800);

    // Qt's parent and central-widget ownership release the canvas with the window.
    Canvas * canvas = new Canvas(this);

    setCentralWidget(canvas);
}

MainWindow::~MainWindow()
{
    delete ui;
}
