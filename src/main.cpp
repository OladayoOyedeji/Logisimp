#include <iostream>

// Simulator:
#include "LogicValue.h"
#include "LogicVector.h"
#include "Port.h"
#include "Component.h"
#include "Wire.h"
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

void test_half_adder()
{
    Circuit circuit;
    Simulator simulator;

    Pin & a = circuit.create_pin(INPUT_PIN);
    Pin & b = circuit.create_pin(INPUT_PIN);

    Pin & sum = circuit.create_pin(OUTPUT_PIN);
    Pin & carry = circuit.create_pin(OUTPUT_PIN);

    XorGate & xor_gate = circuit.create_xor_gate();
    AndGate & and_gate = circuit.create_and_gate();

    Wire & a_wire = circuit.create_wire();
    Wire & b_wire = circuit.create_wire();
    Wire & sum_wire = circuit.create_wire();
    Wire & carry_wire = circuit.create_wire();

    circuit.connect(*a.outputs()[0], a_wire);
    circuit.connect(*xor_gate.inputs()[0], a_wire);
    circuit.connect(*and_gate.inputs()[0], a_wire);

    circuit.connect(*b.outputs()[0], b_wire);
    circuit.connect(*xor_gate.inputs()[1], b_wire);
    circuit.connect(*and_gate.inputs()[1], b_wire);

    circuit.connect(*xor_gate.output(), sum_wire);
    circuit.connect(*sum.inputs()[0], sum_wire);

    circuit.connect(*and_gate.output(), carry_wire);
    circuit.connect(*carry.inputs()[0], carry_wire);

    circuit.initialize(simulator);

    std::cout << "A B | SUM CARRY" << std::endl;
    std::cout << "----|----------" << std::endl;

    for (int a_value = 0; a_value <= 1; a_value++)
    {
        for (int b_value = 0; b_value <= 1; b_value++)
        {
            a.set(LogicVector(1, a_value == 0 ? LOW : HIGH), simulator);
            b.set(LogicVector(1, b_value == 0 ? LOW : HIGH), simulator);

            simulator.run();

            std::cout << a_value << ' '
                      << b_value << " | "
                      << sum.value()[0] << ' '
                      << carry.value()[0] << '\n';
        }
    }
}

int main()
{
    test_half_adder();

    return 0;
}
