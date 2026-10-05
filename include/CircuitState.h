// CircuitState.h
/**
 * Stores the runtime state of a circuit instance.
 * Multiple CircuitState objects can reference the same
 * circuit blueprint while having independent values.
 */

#ifndef CIRCUITSTATE_H
#define CIRCUITSTATE_H

#include "LogicVector.h"

#include <ostream>
#include <string>
#include <unordered_map>

class Circuit;
class Component;
class ComponentState;
class Subcircuit;
class Wire;

class CircuitState
{
public:
    CircuitState(const Circuit & circuit)
        : circuit_(&circuit)
    {
        reset();
    }

    CircuitState(const CircuitState & other) = delete;
    CircuitState & operator=(const CircuitState & other) = delete;
    CircuitState(CircuitState && other) = delete;
    CircuitState & operator=(CircuitState && other) = delete;

    ~CircuitState()
    {
        clear_component_states();
    }

    const Circuit & circuit() const { return *circuit_; }

    CircuitState * parent() const { return parent_; }
    const Subcircuit * placement() const { return parent_subcircuit_; }

    LogicVector & value(const Wire & wire);
    LogicVector value(const Wire & wire) const;

    ComponentState & component_state(const Component & component);
    const ComponentState & component_state(const Component & component)

    void reset();

    std::string to_string() const;

private:
    void clear_component_states();

    const Circuit * circuit_;

    // detecting circular definitions of circuits?
    // custom circuits that depend on each other?
    // these two below are only relevant when a circuit is
    // nested inside another circuit:
    // will be nullptr otherwise
    CircuitState * parent_; // state of the circuit containing this instance
    const Subcircuit * parent_subcircuit_; // the component that represents this instance
    std::unordered_map<const Wire *, LogicVector > wire_values_;
};

inline
std::ostream & operator<<(std::ostream & cout, const CircuitState & state)
{
    cout << state.to_string();
    return cout;
}

#endif // CIRCUITSTATE_H
