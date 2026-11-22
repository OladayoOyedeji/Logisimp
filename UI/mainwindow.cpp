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

    ui->SettingsButton->setParent(canvas);
    ui->GateToolBar->setParent(canvas);

    setCentralWidget(canvas);

    ui->SettingsButton->raise();
    ui->SettingsButton->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_SettingsButton_clicked()
{
    // Open settings menu
}


void MainWindow::on_AndButton_clicked()
{
    // Make and gate
    std::cout << "And button clicked" << '\n';
}

