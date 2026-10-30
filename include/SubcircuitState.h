
/**
 * @file SubcircuitState.h
 * @brief Stores the runtime state of a subcircuit instance.
 * @author Cole Schwandt
 */

#ifndef SUBCIRCUITSTATE_H
#define SUBCIRCUITSTATE_H

#include "CircuitState.h"
#include "ComponentState.h"
#include "Subcircuit.h"

/**
 * @brief Stores the runtime state of a subcircuit placement.
 *
 * Owns a child CircuitState representing one instance of a
 * referenced circuit blueprint.
 *
 * Multiple subcircuit placements can share the same blueprint
 * while maintaining independent runtime values.
 *
 * Inherits from ComponentState so the containing CircuitState
 * can manage it alongside other component-specific states.
 */
class SubcircuitState : public ComponentState
{
public:
    /**
     * @brief Creates the runtime state for a subcircuit placement.
     * @param subcircuit Subcircuit component representing the placement.
     * @param parent Runtime state of the containing circuit.
     * @note Initializes the child state using the referenced circuit
     * blueprint, parent runtime state, and subcircuit placement.
     */
    SubcircuitState(const Subcircuit & subcircuit, CircuitState & parent)
        : child_state_(*subcircuit.circuit(), &parent, &subcircuit)
    {}

    /**
     * @brief Returns the child circuit's runtime state.
     * @return Mutable reference to the owned child CircuitState.
     */
    CircuitState & child_state() { return child_state_; }

    /**
     * @brief Returns the child circuit's runtime state.
     * @return Const reference to the owned child CircuitState.
     */
    const CircuitState & child_state() const { return child_state_; }

private:
    /// Owned runtime state of the referenced subcircuit instance.
    CircuitState child_state_;
};

#endif // SUBCIRCUITSTATE_H
