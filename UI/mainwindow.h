#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace gui {
class Canvas;
}

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_SettingsButton_clicked();

    void on_AndButton_clicked();

private:
    Ui::MainWindow *ui;
    gui::Canvas *canvas = nullptr;
};
#endif // MAINWINDOW_H
