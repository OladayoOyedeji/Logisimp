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

    Canvas *canvas = new Canvas(this);

    setCentralWidget(canvas);
}

MainWindow::~MainWindow()
{
    delete ui;
}
