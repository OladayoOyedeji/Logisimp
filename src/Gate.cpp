// Gate.cpp

#include "Gate.h"
#include "LogicVector.h"
#include "Port.h"
#include "Simulator.h"
#include "Wire.h"

#include <stdexcept>

Gate::Gate(int id, int input_count)
    : Component(id)
{
    if (input_count <= 0)
    {
        throw std::invalid_argument("Gate input count must be positive");
    }

    for (int i = 0; i < input_count; i++)
    {
        add_input(1);
    }

    add_output(1, "Y");
}

Port * Gate::output()
{
    return output_port(0);
}

Port * Gate::output() const
{
    return output_port(0);
}

std::string Gate::to_string() const
{
    std::string ret;

    ret += "Gate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}

LogicValue Gate::input_value(int index) const
{
    Port * port = input_port(index);

    if (port->wire() == nullptr)
    {
        return UNKNOWN;
    }

    return port->wire()->value()[0];
}

void Gate::drive_output(LogicValue value, Simulator & simulator)
{
    Port * port = output_port(0);

    if (port->wire() == nullptr)
    {
        return;
    }

    LogicVector output_value(1);
    output_value[0] = value;

    simulator.drive(*port->wire(), output_value);
}
