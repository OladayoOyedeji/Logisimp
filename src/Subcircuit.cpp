/**
 * @file Subcircuit.cpp
 * @brief Implements custom circuit placement and evaluation.
 * @author Cole Schwandt
 */

#include "Subcircuit.h"

#include "Circuit.h"
#include "CircuitState.h"
#include "Pin.h"
#include "Port.h"
#include "Simulator.h"
#include "SubcircuitState.h"
#include "Net.h"

Subcircuit::Subcircuit(int id, Circuit & circuit)
    : Component(id), circuit_(&circuit)
{}

ComponentState * Subcircuit::create_state(CircuitState & state) const
{
    return new SubcircuitState(*this, state);
}

void Subcircuit::evaluate(Simulator & simulator, CircuitState & state)
{
    SubcircuitState & subcircuit_state = dynamic_cast< SubcircuitState & >(state.component_state(*this));

    CircuitState & child_state = subcircuit_state.child_state();

    // transfer the parent's input values into the child circuit
    // (make faster with array of size 2 index by port->net() == nullptr?)
    // The original optimization note refers to parent_input_port below.
    // Project keeps placement port order aligned with the blueprint's pin order.
    // A missing parent connection supplies UNKNOWN rather than retaining a
    // child's previous value, which would hide an undriven interface input.
    for (int i = 0; i < input_count(); ++i)
    {
        Port * parent_input_port = input_port(i);
        Pin * child_input_pin = circuit_->input_pins().at(i);

        LogicVector input_value = state.value(*parent_input_port);

        child_input_pin->set(input_value, simulator, child_state);
    }

    // publish the child's current output values into the parent circuit
    // Input changes queue child evaluations; they do not settle the child here.
    // Publish its current outputs now. Changed child output pins will enqueue
    // this placement again so later evaluations publish the settled values.
    for (int i = 0; i < output_count(); ++i)
    {
        Port * parent_output_port = output_port(i);

        Pin * child_output_pin = circuit_->output_pins().at(i);
        simulator.drive(state, *parent_output_port, child_output_pin->value(child_state));
    }
}

std::string Subcircuit::to_string() const
{
    std::string result;

    result += "Subcircuit(id=";
    result += std::to_string(id());
    result += ", label=\"";
    result += label();
    result += "\", inputs=";
    result += std::to_string(input_count());
    result += ", outputs=";
    result += std::to_string(output_count());
    result += ')';

    return result;
}
