#include <iostream>

// Simulator:
#include "LogicValue.h"
#include "LogicVector.h"
#include "Port.h"
#include "Component.h"
#include "Signal.h"
#include "Simulator.h"
#include "Circuit.h"
#include "Gate.h"
#include "Gates.h"
#include "Pin.h"
//#include "Clock.h"
//#include "Mux.h"
//#include "Adder.h"
//#include "Register.h"
//#include "RAM.h"
//#include "ROM.h"
// TODO: custom user circuits, led, clock, mux, adder, register, ram, rom

// GUI:
// TODO:

void test_logic_value()
{
    LogicValue result = logicAnd(HIGH, LOW);
}

int main()
{
    std::cout << "Hello world" << std::endl;
    return 0;
}
