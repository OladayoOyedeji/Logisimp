/**
 * @file StateUpdate.cpp
 * @brief Prepares persistent runtime instances for structural edits.
 */

#include "StateUpdate.h"

#include "Circuit.h"
#include "Component.h"
#include "ComponentState.h"
#include "Net.h"
#include "Pin.h"
#include "Port.h"
#include "Simulator.h"
#include "Subcircuit.h"
#include "SubcircuitState.h"

StateUpdate::StateUpdate(CircuitState & state)
    : state_(state), committed_(false)
{
    try
    {
        prepare();
    }
    catch (...)
    {
        clear_prepared();
        throw;
    }
}

StateUpdate::~StateUpdate()
{
    clear_prepared();
}

void StateUpdate::clear_prepared() noexcept
{
    for (StateUpdate * child : children_)
    {
        delete child;
    }
    children_.clear();

    if (!committed_)
    {
        for (ComponentState * component_state : created_states_)
        {
            delete component_state;
        }
    }
    created_states_.clear();
}

void StateUpdate::prepare()
{
    for (Component * component : state_.circuit().components())
    {
        bool reuse = true;
        const Pin * pin = dynamic_cast< const Pin * >(component);
        if (pin != nullptr)
        {
            std::pair< int, int > shape(int(pin->type()), pin->width());
            pin_shapes_.emplace(component, shape);
            auto old_shape = state_.pin_shapes_.find(component);
            reuse = old_shape != state_.pin_shapes_.end() && old_shape->second == shape;
        }

        auto old_state = state_.component_states_.find(component);
        ComponentState * component_state = nullptr;
        bool existing = reuse && old_state != state_.component_states_.end();
        if (existing)
        {
            component_state = old_state->second;
        }
        else
        {
            component_state = component->create_state(state_);
            if (component_state != nullptr)
            {
                try
                {
                    created_states_.push_back(component_state);
                }
                catch (...)
                {
                    delete component_state;
                    throw;
                }
            }
        }

        if (component_state != nullptr)
        {
            component_states_.emplace(component, component_state);
        }

        // A new placement already constructed its complete candidate subtree.
        // Existing placements instead prepare their children in place.
        if (existing && dynamic_cast< Subcircuit * >(component) != nullptr)
        {
            SubcircuitState & subcircuit_state = dynamic_cast< SubcircuitState & >(*component_state);
            StateUpdate * child = new StateUpdate(subcircuit_state.child_state());
            try
            {
                children_.push_back(child);
            }
            catch (...)
            {
                delete child;
                throw;
            }
        }

        for (Port * output_port : component->outputs())
        {
            auto old_value = state_.output_values_.find(output_port);
            if (reuse && old_value != state_.output_values_.end() && old_value->second.width() == output_port->width())
            {
                output_values_.emplace(output_port, old_value->second);
            }
            else
            {
                output_values_.emplace(output_port, LogicVector(output_port->width(), UNKNOWN));
            }
        }
    }

    for (const Net * net : state_.circuit().nets())
    {
        LogicVector value(net->width(), UNKNOWN);
        if (net->driver() != nullptr)
        {
            auto old_net_value = state_.net_values_.find(net);
            if (old_net_value != state_.net_values_.end() && old_net_value->second.width() == net->width())
            {
                value = old_net_value->second;
            }
            else
            {
                // Net identity changes on a rebuild; driver identity and its
                // last output value are independent of the wire graph.
                auto driver_value = output_values_.find(net->driver());
                if (driver_value != output_values_.end() && driver_value->second.width() == net->width())
                {
                    value = driver_value->second;
                }
            }
        }
        // Fragments with no driver start UNKNOWN, never with stale split data.
        net_values_.emplace(net, value);
    }
}

void StateUpdate::schedule(Simulator & simulator) const
{
    for (Component * component : state_.circuit().components())
    {
        simulator.enqueue(state_, component);
        if (dynamic_cast< Subcircuit * >(component) == nullptr)
        {
            continue;
        }

        SubcircuitState & subcircuit_state = dynamic_cast< SubcircuitState & >(*component_states_.at(component));
        CircuitState & child_state = subcircuit_state.child_state();
        StateUpdate * prepared_child = nullptr;
        for (StateUpdate * child : children_)
        {
            if (&child->state_ == &child_state)
            {
                prepared_child = child;
                break;
            }
        }

        if (prepared_child != nullptr)
        {
            prepared_child->schedule(simulator);
        }
        else
        {
            simulator.schedule(child_state);
        }
    }
}

void StateUpdate::commit() noexcept
{
    if (committed_)
    {
        return;
    }

    for (StateUpdate * child : children_)
    {
        child->commit();
    }

    state_.net_values_.swap(net_values_);
    state_.output_values_.swap(output_values_);
    state_.pin_shapes_.swap(pin_shapes_);
    state_.component_states_.swap(component_states_);

    for (const auto & old_state : component_states_)
    {
        auto current_state = state_.component_states_.find(old_state.first);
        if (current_state == state_.component_states_.end() || current_state->second != old_state.second)
        {
            delete old_state.second;
        }
    }
    committed_ = true;
    created_states_.clear();
}
