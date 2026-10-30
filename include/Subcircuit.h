
/**
 * @file Subcircuit.h
 * @brief Represents an instance of a custom circuit within another circuit.
 * @author Cole Schwandt
 */

#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "Component.h"

class Circuit;

/**
 * @brief Represents a placement of another circuit within a circuit.
 *
 * References a reusable Circuit blueprint and inherits the
 * component identity, label, and ports from Component.
 *
 * Multiple placements can reference the same blueprint while
 * maintaining independent runtime values through SubcircuitState.
 *
 * The referenced Circuit is not owned by this component.
 */
class Subcircuit : public Component
{
public:
    /**
     * @brief Creates a subcircuit placement referencing a circuit blueprint.
     * @param id Component identity within the containing circuit.
     * @param circuit Circuit blueprint to reference.
     */
    Subcircuit(int id, Circuit & circuit);

    /**
     * @brief Returns the circuit blueprint referenced by this placement.
     * @return Borrowed pointer to the referenced Circuit.
     */
    Circuit * circuit() const { return circuit_; }

    /**
     * @brief Creates runtime storage for this subcircuit placement.
     * @param state Runtime state of the containing circuit.
     * @return Newly allocated SubcircuitState through a ComponentState pointer.
     * @note Ownership of the returned state is transferred to the caller.
     */
    ComponentState * create_state(CircuitState & state) const override;

    /**
     * @brief Propagates signals between the parent and child circuits.
     * @param simulator Simulator used to drive nets and schedule evaluations.
     * @param state Runtime state of the containing circuit.
     *
     * Transfers parent input values to the child circuit's input pins
     * and drives parent output nets using the child's output pin values.
     *
     * @note Child evaluations are scheduled rather than completed
     * synchronously. Updated child outputs notify the parent.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Returns a description of the subcircuit placement.
     * @return String containing its identity, label, and port counts.
     */
    std::string to_string() const override;

private:
    /// Borrowed pointer to the referenced circuit blueprint.
    Circuit * circuit_;
};

#endif // SUBCIRCUIT_H
