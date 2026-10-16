/**
 * @file CircuitState.h
 * @brief Stores the runtime state of a circuit instance.
 * @author Cole Schwandt
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
class Simulator;
class Net;

class CircuitState
{
public:
    CircuitState(const Circuit & circuit, CircuitState * parent = nullptr, const Subcircuit * parent_subcircuit = nullptr);

    CircuitState(const CircuitState & other) = delete;
    CircuitState & operator=(const CircuitState & other) = delete;
    CircuitState(CircuitState && other) = delete;
    CircuitState & operator=(CircuitState && other) = delete;

    ~CircuitState();

    const Circuit & circuit() const { return *circuit_; }

    CircuitState * parent() const { return parent_; }
    const Subcircuit * placement() const { return parent_subcircuit_; }

    LogicVector & value(const Net & net);
    LogicVector value(const Net & net) const;

    ComponentState & component_state(const Component & component);
    const ComponentState & component_state(const Component & component) const;

    // Clear the simulator queue before resetting or destroying a hierarchy.
    void reset();
    bool reset(Simulator & simulator);
    bool initialize(Simulator & simulator);

    std::string to_string() const;

private:
    void clear_component_states();

    const Circuit * circuit_;

    // these two below are only relevant when a circuit is
    // nested inside another circuit:
    // will be nullptr otherwise
    CircuitState * parent_; // state of the circuit containing this instance
    const Subcircuit * parent_subcircuit_; // the component that represents this instance
    std::unordered_map< const Net *, LogicVector > net_values_;
    std::unordered_map< const Component *, ComponentState * > component_states_;
};

inline std::ostream & operator<<(std::ostream & cout, const CircuitState & state)
{
    cout << state.to_string();
    return cout;
}

#endif // CIRCUITSTATE_H
