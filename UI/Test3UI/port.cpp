#include "port.h"
#include "gate.h"

Port::Port(Gate* Gate, double x, double y, PortType type)
    : parentGate(Gate), offsetX(x), offsetY(y), type(type)
{
}

double Port::getX() const
{
    return parentGate->getX() + offsetX;
}

double Port::getY() const
{
    return parentGate->getY() + offsetY;
}

PortType Port::getType()
{
    return type;
}

bool Port::contains(double x, double y) const
{
    std::cout << "OUTPUT PORT CLICKED" << '\n';

    double dx = x - getX();
    double dy = y - getY();

    double radius = 8.0;

    return dx * dx + dy * dy <= radius * radius;
}
