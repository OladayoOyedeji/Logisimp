#include "mainwindow.h"
#include "circuitcontroller.h"
#include "Project.h"
#include "Gates.h"
#include <iostream>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // This is for testing, Remove later
    CircuitController controller;
    AndGate& logicGate = controller.addAndGate();
    std::cout << "Created logical AND gate: "
              << logicGate.to_string() << '\n';

    Project test;
    auto& circuit = test.create_circuit();

    std::cout << "Logical test created successfully!!!" << '\n';
    
    MainWindow w;
    w.show();
    return a.exec();
}
