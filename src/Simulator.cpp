/**
 * @file Simulator.cpp
 * @brief Implements queued component evaluation and signal propagation.
 * @author Cole Schwandt
 */

#include "Simulator.h"
#include "Circuit.h"
#include "CircuitState.h"
#include "Component.h"
#include "LogicVector.h"
#include "Port.h"
#include "Subcircuit.h"
#include "SubcircuitState.h"
#include "Net.h"

#include <algorithm>
#include <stdexcept>

void Simulator::enqueue(CircuitState & state, Component * component)
{
    if (component == nullptr)
    {
        return;
    }

    const std::vector< Component * > & components = state.circuit().components();
    if (std::find(components.begin(), components.end(), component) == components.end())
    {
        throw std::invalid_argument("Component does not belong to this circuit state");
    }

    Event event(state, *component);
    // Several changed inputs can share one pending evaluation: the event reads
    // the latest runtime values, so storing a second event would add no snapshot.
    std::pair< std::unordered_set< Event, EventHash >::iterator, bool > insertion = queued_components_.insert(event);
    if (!insertion.second)
    {
        return;
    }

    try
    {
        work_queue_.push(event);
    }
    catch (...)
    {
        // A set entry without a queue entry would suppress all future attempts
        // to schedule this component, so undo only the new identity insertion.
        queued_components_.erase(insertion.first);
        throw;
    }
}

void Simulator::schedule(CircuitState & state)
{
    for (Component * component : state.circuit().components())
    {
        enqueue(state, component);

        Subcircuit * subcircuit = dynamic_cast< Subcircuit * >(component);
        if (subcircuit != nullptr)
        {
            SubcircuitState & subcircuit_state = dynamic_cast< SubcircuitState & >(state.component_state(*subcircuit));
            // Even children with unconnected inputs need an initial evaluation.
            schedule(subcircuit_state.child_state());
        }
    }
}

void Simulator::notify_parent(CircuitState & state)
{
    if (state.parent() != nullptr)
    {
        // Resolve the non-owning const placement through its parent's graph.
        // Evaluation mutates runtime state, not the component blueprint.
        for (Component * component : state.parent()->circuit().components())
        {
            if (component == state.placement())
            {
                enqueue(*state.parent(), component);
                return;
            }
        }

        throw std::logic_error("Parent placement is no longer in its blueprint");
    }
}

void Simulator::drive(CircuitState & state, Net & net, const LogicVector & value)
{
    if (net.width() != value.width())
    {
        throw std::runtime_error("Cannot drive a net with a value of a different width");
    }

    LogicVector & current_value = state.value(net);
    if (net.driver() != nullptr)
    {
        // Keep low-level net drives consistent with the output-value cache too.
        state.output_values_.at(net.driver()) = value;
    }
    if (current_value == value)
    {
        // Stable signals, including identical UNKNOWN bits, must not keep
        // feedback paths alive by repeatedly scheduling their listeners.
        return;
    }

    current_value = value;

    // Store the signal before scheduling: deduplicated listeners will observe
    // the most recent net value when their FIFO event is eventually evaluated.
    for (Port * listener : net.listeners())
    {
        enqueue(state, listener->owner());
    }
}

void Simulator::drive(CircuitState & state, Port & port, const LogicVector & value)
{
    if (port.direction() != OUTPUT || port.width() != value.width())
    {
        throw std::invalid_argument("Output drive requires an output port with matching width");
    }

    // The map also validates that this output belongs to the runtime instance.
    // A disconnected output still has a value, independently of derived nets.
    state.output_values_.at(&port) = value;
    if (port.net() != nullptr)
    {
        drive(state, *port.net(), value);
    }
}

bool Simulator::run()
{
    // Untimed feedback may never settle; bound work without inventing a delay
    // or treating the evaluation count as elapsed simulation time.
    const int max_evals = 100000;
    int evaluation_count = 0;

    try
    {
        while (!work_queue_.empty())
        {
            if (evaluation_count >= max_evals)
            {
                clear();
                return false;
            }

            Event event = work_queue_.front();

            // Remove the identity before evaluating so a feedback-driven change
            // can queue this same instance/component pair for a later pass.
            work_queue_.pop();
            queued_components_.erase(event);

            evaluation_count++;

            event.component->evaluate(*this, *event.state);
        }
    }
    catch (...)
    {
        // Keep no stale work after a partially completed evaluation. Signal
        // writes already made by that evaluation are intentionally not undone.
        clear();
        throw;
    }

    return true;
}

void Simulator::clear()
{
    while (!work_queue_.empty())
    {
        work_queue_.pop();
    }

    queued_components_.clear();
}

void Simulator::discard(CircuitState & state)
{
    Simulator replacement;
    std::queue< Event > pending = work_queue_;
    while (!pending.empty())
    {
        Event event = pending.front();
        pending.pop();

        CircuitState * ancestor = event.state;
        while (ancestor != nullptr && ancestor != &state)
        {
            ancestor = ancestor->parent();
        }

        if (ancestor == nullptr)
        {
            replacement.enqueue(*event.state, event.component);
        }
    }

    // Allocation failures above leave the original queue and set untouched.
    swap_pending(replacement);
}

void Simulator::swap_pending(Simulator & other) noexcept
{
    work_queue_.swap(other.work_queue_);
    queued_components_.swap(other.queued_components_);
}

std::string Simulator::to_string() const
{
    std::queue< Event > queue = work_queue_;
    std::string ret;

    ret += "Simulator(queued=";
    ret += std::to_string(queue.size());
    ret += ')';

    if (!queue.empty())
    {
        ret += "\nQueue:";

        while (!queue.empty())
        {
            ret += "\n  ";

            ret += "state@";
            ret += std::to_string(std::hash< CircuitState * >()(queue.front().state));
            ret += " ";
            ret += queue.front().component->to_string();

            queue.pop();
        }
    }

    return ret;
}
