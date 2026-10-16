/**
 * @file SubcircuitState.h
 * @brief Stores the runtime state of a subcircuit instance.
 * @author Cole Schwandt
 */

// SubcircuitState.h

#ifndef SUBCIRCUITSTATE_H
#define SUBCIRCUITSTATE_H

#include "CircuitState.h"
#include "ComponentState.h"
#include "Subcircuit.h"

class SubcircuitState : public ComponentState
{
public:
    SubcircuitState(const Subcircuit & subcircuit, CircuitState & parent)
        : child_state_(*subcircuit.circuit(), &parent, &subcircuit)
    {}

    CircuitState & child_state() { return child_state_; }
    const CircuitState & child_state() const { return child_state_; }

private:
    CircuitState child_state_;
};

#endif // SUBCIRCUITSTATE_H
