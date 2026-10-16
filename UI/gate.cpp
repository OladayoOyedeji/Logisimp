#include "gate.h"
#include "port.h"

double Gate::getX()
{
    return x;
}

double Gate::getY()
{
    return y;
}

const std::vector<Port *> &Gate::getInputPorts() const
{
    return inputPorts;
}

const std::vector<Port *> &Gate::getOutputPorts() const
{
    return outputPorts;
}
