/**
 * @file Pin.cpp
 * @brief Implements circuit pin evaluation and interaction with runtime values.
 * @author Cole Schwandt
 */

#include "Pin.h"

#include "CircuitState.h"
#include "PinState.h"
#include "Port.h"
#include "Simulator.h"
#include "Net.h"

#include <stdexcept>

Pin::Pin(int id, PinType type, int width)
    : Component(id), type_(type), width_(width)
{
    if (type_ != INPUT_PIN && type_ != OUTPUT_PIN)
    {
        throw std::invalid_argument("Invalid pin type");
    }

    if (type_ == INPUT_PIN)
    {
        add_output(width_, "Y");
    }
    else
    {
        add_input(width_, "A");
    }
}

ComponentState * Pin::create_state(CircuitState &) const
{
    return new PinState(width_, type_ == INPUT_PIN ? LOW : UNKNOWN);
}

LogicVector & Pin::value(CircuitState & state)
{
    PinState & pin_state = dynamic_cast< PinState & >(state.component_state(*this));

    return pin_state.value();
}

LogicVector Pin::value(const CircuitState & state) const
{
    const PinState & pin_state = dynamic_cast< const PinState & >(state.component_state(*this));

    return pin_state.value();
}

void Pin::set(const LogicVector & bits, Simulator & simulator, CircuitState & state)
{
    if (type_ != INPUT_PIN)
    {
        throw std::logic_error("Cannot set an output pin");
    }

    if (bits.width() != width_)
    {
        throw std::invalid_argument("Pin value width does not match pin width");
    }

    value(state) = bits;

    evaluate(simulator, state);
}

void Pin::toggle(Simulator & simulator, CircuitState & state)
{
    // UNKNOWN carries no known binary value, so inversion must preserve it.
    static const LogicValue toggle_table[3] =
    {
        HIGH,
        LOW,
        UNKNOWN
    };

    if (type_ != INPUT_PIN)
    {
        throw std::logic_error("Cannot toggle an output pin");
    }

    LogicVector & bits = value(state);

    for (int i = 0; i < bits.width(); ++i)
    {
        bits[i] = toggle_table[bits[i]];
    }

    evaluate(simulator, state);
}

void Pin::evaluate(Simulator & simulator, CircuitState & state)
{
    if (type_ == INPUT_PIN)
    {
        // Interface inputs drive the circuit internally, hence their OUTPUT port.
        Port * pin_output_port = output_port(0);

        simulator.drive(state, *pin_output_port, value(state));

        return;
    }

    Port * pin_input_port = input_port(0);

    // An undriven interface output must not retain an earlier sampled value.
    LogicVector bits(width_, UNKNOWN);

    if (pin_input_port->net() != nullptr)
    {
        bits = state.value(*pin_input_port->net());
    }

    if (value(state) != bits)
    {
        value(state) = bits;

        // Only changed child outputs can require a parent placement to republish.
        // For a root circuit, notify_parent has no containing placement to queue.
        simulator.notify_parent(state);
    }
}

std::string Pin::to_string() const
{
    std::string ret;

    ret += "Pin(id=";
    ret += std::to_string(id());
    ret += ", type=";

    if (type_ == INPUT_PIN)
    {
        ret += "INPUT_PIN";
    }
    else
    {
        ret += "OUTPUT_PIN";
    }

    ret += ", width=";
    ret += std::to_string(width_);
    ret += ')';

    return ret;
}
