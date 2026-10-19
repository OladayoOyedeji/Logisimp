/**
 * @file CircuitState.cpp
 * @brief Implements the runtime state of a circuit instance.
 * @author Cole Schwandt
 */

#include "CircuitState.h"

#include "Circuit.h"
#include "Component.h"
#include "ComponentState.h"
#include "Simulator.h"
#include "Subcircuit.h"
#include "Net.h"
#include "Pin.h"
#include "Port.h"

#include <algorithm>
#include <stdexcept>

CircuitState::CircuitState(const Circuit & circuit, CircuitState * parent, const Subcircuit * parent_subcircuit)
    : circuit_(&circuit), parent_(parent), parent_subcircuit_(parent_subcircuit)
{
    if ((parent_ == nullptr) != (parent_subcircuit_ == nullptr))
    {
        // Output notifications need both links to find the containing placement;
        // root instances have neither link.
        throw std::invalid_argument("Nested state requires both a parent and a placement");
    }

    if (parent_ != nullptr)
    {
        const std::vector< Component * > & components = parent_->circuit().components();
        if (parent_subcircuit_->circuit() != circuit_ || std::find(components.begin(), components.end(), parent_subcircuit_) == components.end())
        {
            throw std::invalid_argument("Placement does not belong to the supplied parent or blueprint");
        }
    }

    // Derived nets are borrowed by the value map. Register before constructing
    // child states so connectivity edits cannot invalidate any live hierarchy.
    ++circuit_->active_states_;
    try
    {
        reset();
    }
    catch (...)
    {
        --circuit_->active_states_;
        throw;
    }
}

CircuitState::~CircuitState()
{
    clear_component_states();
    --circuit_->active_states_;
}

LogicVector & CircuitState::value(const Net & net)
{
    return net_values_.at(&net);
}

LogicVector CircuitState::value(const Net & net) const
{
    return net_values_.at(&net);
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
    // Virtual destruction also releases child CircuitStates held by placements.
    // Pending simulator events borrow these objects and must already be cleared.
    for (const std::pair< const Component * const, ComponentState * > & entry : component_states_)
    {
        delete entry.second;
    }

    component_states_.clear();
}

void CircuitState::reset()
{
    clear_component_states();
    net_values_.clear();
    output_values_.clear();
    pin_shapes_.clear();

    try
    {
        // init every net to UNKNOWN
        // No driver has been evaluated yet; UNKNOWN keeps an undriven net from
        // looking like a known LOW signal merely because storage was recreated.
        for (const Net * net : circuit_->nets())
        {
            // net_values_[net] = LogicVector(net->width(), UNKNOWN)
            net_values_.emplace(net, LogicVector(net->width(), UNKNOWN));
            // emplace is [] but doesn't overwrite existing values.
            // it's more explicit C++ to use emplace because it's clear
            // new values are being inserted
        }

        // create runtime states for components that need a state
        // Stateless gates share only their blueprint; pins and subcircuits need
        // fresh storage so repeated placements have independent runtime values.
        for (Component * component : circuit_->components())
        {
            // Output values belong to this instance even while no wire is
            // attached. Live connectivity edits can seed replacement nets from
            // the last driven value without replacing the component's state.
            for (Port * output_port : component->outputs())
            {
                output_values_.emplace(output_port, LogicVector(output_port->width(), UNKNOWN));
            }

            const Pin * pin = dynamic_cast< const Pin * >(component);
            if (pin != nullptr)
            {
                pin_shapes_.emplace(component, std::make_pair(int(pin->type()), pin->width()));
            }

            ComponentState * component_state = component->create_state(*this);

            if (component_state != nullptr)
            {
                try
                {
                    component_states_.emplace(component, component_state);
                }
                catch (...) // std::bad_alloc prob the only possible exception that could happen here
                {
                    // The factory returned an owned allocation, but the map has
                    // not adopted it if inserting the entry fails.
                    delete component_state;
                    throw;
                }
            }
        }
    }
    catch (...)
    {
        // Reset discarded the previous storage; release any partially rebuilt
        // hierarchy rather than expose a mixture of initialized and absent state.
        clear_component_states();
        net_values_.clear();
        output_values_.clear();
        pin_shapes_.clear();
        throw;
    }
}

bool CircuitState::reset(Simulator & simulator)
{
    // Queued events can point at children that reset() is about to destroy.
    simulator.clear();
    reset();
    return initialize(simulator);
}

bool CircuitState::initialize(Simulator & simulator)
{
    // Initialization replaces all pending work in the shared simulator, even
    // for other roots; the current signal values themselves are preserved.
    simulator.clear();
    try
    {
        simulator.schedule(*this);
        return simulator.run();
    }
    catch (...)
    {
        simulator.clear();
        throw;
    }
}

std::string CircuitState::to_string() const
{
    std::string ret = "CircuitState(";
    const std::vector< Net * > & nets = circuit_->nets();
    for (int i = 0; i < int(nets.size()); ++i)
    {
        if (i > 0)
        {
            ret += ", ";
        }

        const Net * net = nets[i];
        ret += "net ";
        ret += std::to_string(net->id());
        ret += "=";
        ret += value(*net).to_string();
    }
    ret += ")";

    return ret;
}
