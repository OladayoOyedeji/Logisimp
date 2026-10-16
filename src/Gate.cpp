/**
 * @file Gate.cpp
 * @brief Implements the base class for primitive logic gates.
 * @author Cole Schwandt
 */

#include "Gate.h"

#include "CircuitState.h"
#include "LogicVector.h"
#include "Port.h"
#include "Simulator.h"
#include "Net.h"

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

Port * Gate::output_port(int index)
{
    return Component::output_port(index);
}

Port * Gate::output_port(int index) const
{
    return Component::output_port(index);
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

LogicValue Gate::input_value(int index, CircuitState & state) const
{
    Port * gate_input_port = input_port(index);

    if (gate_input_port->net() == nullptr)
    {
        // A missing connection supplies no evidence of either known logic level.
        return UNKNOWN;
    }

    return state.value(*gate_input_port->net())[0];
}

void Gate::drive_output(LogicValue value, Simulator & simulator, CircuitState & state)
{
    Port * gate_output_port = output_port();

    if (gate_output_port->net() == nullptr)
    {
        // A disconnected output has no net value or listeners to update.
        return;
    }

    LogicVector gate_output_value(1);
    gate_output_value[0] = value;

    // drive() suppresses unchanged values and queues listeners in this instance;
    // gate evaluation itself carries no delay or timestamp.
    simulator.drive(state, *gate_output_port->net(), gate_output_value);
}
