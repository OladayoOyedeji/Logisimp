#ifndef GATE_H
#define GATE_H

#include "circuit.h"
#include <vector>
#include <iostream>

class Port;

class Gate : public Circuit
{
public:
    virtual ~Gate() = default;

    double getX();
    double getY();

    const std::vector<Port*>& getInputPorts() const;
    const std::vector<Port*>& getOutputPorts() const;

protected:
    int numInputs = 0;
    int numOutputs = 1;

    double width = 50.0;
    double height = 50.0;

    std::vector<Port*> inputPorts;
    std::vector<Port*> outputPorts;
};

#endif // GATE_H
