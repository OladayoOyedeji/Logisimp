#ifndef GATE_H
#define GATE_H

#include "circuit.h"

class Gate : public Circuit
{
public:
    virtual ~Gate() = default;

protected:
    int numInputs = 0;
    int numOutputs = 1;

    double width = 50.0;
    double height = 50.0;
};

#endif // GATE_H
