#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "canvas.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Logisimp");
    resize(1200, 800);

    gui::Canvas *canvas = new gui::Canvas(this);

    setCentralWidget(canvas);
}

MainWindow::~MainWindow()
{
    delete ui;
}
