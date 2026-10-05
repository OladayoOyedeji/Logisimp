// CircuitState.cpp

#include "CircuitState.h"

#include "Circuit.h"
#include "Component.h"
#include "ComponentState.h"
#include "Wire.h"

LogicVector & CircuitState::value(const Wire & wire)
{
    return wire_values_.at(&wire);
}

LogicVector CircuitState::value(const Wire & wire) const
{
    return wire_values_.at(&wire);
}

ComponentState & CircuitState::component_state(const Component & component)
{
    return *component_states_.at(&component);
}

const ComponentState & CircuitState::component_state(const Component & component) const
{
    return *component_states_.at(&component);
}

void CircuitState::clear_component_states()
{
    for (auto & entry : component_states_)
    {
        delete entry.second;
    }

    component_states_.clear();
}

void CircuitState::reset()
{
    clear_component_states();
    wire_values_.clear();

    try
    {
        // init every wire to UNKNOWN
        for (const Wire * wire : circuit_->wires())
        {
            // wire_values_[wire] = LogicVector(wire->width(), UNKNOWN)
            wire_values_.emplace(wire, LogicVector(wire->width(), UNKNOWN));
            // emplace is [] but doesn't overwrite existing values.
            // it's more explicit C++ to use emplace because it's clear
            // new values are being inserted
        }

        // create runtime states for components that need a state
        for (Component * component : circuit_->components())
        {
            ComponentState * state = component->create_state(*this);

            if (state != nullptr)
            {
                try
                {
                    component_states_.emplace(component, state);
                }
                catch (...) // std::bad_alloc prob the only possible exception that could happen here
                {
                    delete state;
                    throw;
                }
            }
        }
    }
    catch (...)
    {
        clear_component_states();
        wire_values.clear();
        throw;
    }
}

std::string CircuitState::to_string() const
{
    std::string ret = "CircuitState(";
    const std::vector< Wire * > & wires = circuit_->wires();
    for (int i = 0; i < wires.size(); ++i)
    {
        if (i > 0)
        {
            ret += ", ";
        }

        const Wire * wire = wires[i];
        ret += "wire ";
        ret += std::to_string(wire->id());
        ret += "=";
        ret += value(*wire).to_string();
    }
    ret += ")";

    return ret;
}
