#include "wire.h"
#include "port.h"

Wire::Wire(Port *startPort, Port *endPort)
    : start(startPort), end(endPort)
{}

Port *Wire::getStart() const
{
    return start;
}

Port *Wire::getEnd() const
{
    return end;
}
