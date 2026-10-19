
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
#include <utility>

class Circuit;
class Component;
class ComponentState;
class Subcircuit;
class Simulator;
class Net;
class Port;
class StateUpdate;

/**
 * @brief Stores the runtime values of one circuit instance.
 *
 * Associates a circuit blueprint with its net values and
 * component-specific states. Multiple instances can share
 * the same Circuit while maintaining independent values.
 *
 * Nested instances reference their containing CircuitState
 * and the Subcircuit component representing their placement.
 *
 * Owns component-specific states but borrows structural
 * objects from the circuit blueprint.
 */
class CircuitState
{
public:
    /**
     * @brief Creates runtime storage for a circuit instance.
     * @param circuit Circuit blueprint being instantiated.
     * @param parent Containing runtime state, or nullptr for a root instance.
     * @param parent_subcircuit Placement representing this instance, or nullptr for a root.
     * @note Registers an active state with the blueprint and initializes runtime storage.
     */
    CircuitState(const Circuit & circuit, CircuitState * parent = nullptr, const Subcircuit * parent_subcircuit = nullptr);

        /** @brief Circuit states cannot be copied. */
    CircuitState(const CircuitState & other) = delete;

    /** @brief Circuit states cannot be assigned. */
    CircuitState & operator=(const CircuitState & other) = delete;

    /** @brief Circuit states cannot be moved. */
    CircuitState(CircuitState && other) = delete;

    /** @brief Circuit states cannot be move-assigned. */
    CircuitState & operator=(CircuitState && other) = delete;
    
    /**
     * @brief Destroys runtime storage and unregisters the circuit instance.
     * @note Clear simulator events referencing this hierarchy before destruction.
     */
    ~CircuitState();

    /**
     * @brief Returns the circuit blueprint used by this instance.
     * @return Borrowed reference to the circuit.
     */
    const Circuit & circuit() const { return *circuit_; }

    /**
     * @brief Returns the containing runtime state.
     * @return Borrowed parent pointer, or nullptr for a root instance.
     */
    CircuitState * parent() const { return parent_; }

    /**
     * @brief Returns the subcircuit placement representing this instance.
     * @return Borrowed placement pointer, or nullptr for a root instance.
     */
    const Subcircuit * placement() const { return parent_subcircuit_; }

    /**
     * @brief Returns the stored value of a net.
     * @param net Net belonging to this instance's blueprint.
     * @return Mutable reference to the net's LogicVector.
     * @throws std::out_of_range If the net has no stored value.
     * @note Modifying the returned value does not automatically schedule propagation.
     */
    LogicVector & value(const Net & net);

    /**
     * @brief Returns a copy of a net's stored value.
     * @param net Net belonging to this instance's blueprint.
     * @return Copy of the net's LogicVector.
     * @throws std::out_of_range If the net has no stored value.
     */
    LogicVector value(const Net & net) const;

    /**
     * @brief Returns a component's runtime-specific state.
     * @param component Component belonging to the circuit blueprint.
     * @return Mutable reference to its ComponentState.
     * @throws std::out_of_range If the component has no runtime state.
     */
    ComponentState & component_state(const Component & component);

    /**
     * @brief Returns a component's runtime-specific state.
     * @param component Component belonging to the circuit blueprint.
     * @return Const reference to its ComponentState.
     * @throws std::out_of_range If the component has no runtime state.
     */
    const ComponentState & component_state(const Component & component) const;

    /**
     * @brief Recreates the instance's runtime storage.
     *
     * Destroys existing component states and resets net values
     * and component-specific states to their initial values.
     *
     * @note Does not clear or execute a Simulator. Pending events
     * referencing this hierarchy must be cleared beforehand.
     */
    void reset();

    /**
     * @brief Resets the instance and initializes its simulation.
     * @param simulator Simulator used to evaluate the circuit.
     * @return True if initialization completes successfully; false if the simulator reaches its work limit.
     * @note Clears the simulator's entire pending event queue.
     */
    bool reset(Simulator & simulator);

    /**
     * @brief Schedules and evaluates the circuit's current runtime state.
     * @param simulator Simulator used to evaluate the circuit.
     * @return True if evaluation completes successfully; false if the simulator reaches its work limit.
     * @note Preserves existing values but clears the simulator's pending queue before scheduling.
     */
    bool initialize(Simulator & simulator);

    /**
     * @brief Returns a description of the instance's local net values.
     * @return String containing the circuit state's diagnostic information.
     */
    std::string to_string() const;

private:
    friend class Simulator;
    friend class StateUpdate;

    /**
     * @brief Deletes all owned component-specific runtime states.
     * @note Also destroys nested child states owned by SubcircuitState objects.
     */
    void clear_component_states();

    /// Borrowed pointer to the circuit blueprint.
    const Circuit * circuit_;

    /// Borrowed pointer to the containing runtime state, or nullptr for a root.
    CircuitState * parent_;

    /// Borrowed pointer to the Subcircuit representing this instance, or nullptr for a root.
    const Subcircuit * parent_subcircuit_;

    /// Runtime net values indexed by borrowed Net pointers.
    std::unordered_map< const Net *, LogicVector > net_values_;

    /// Last driven output values survive disconnection and replacement of derived nets.
    std::unordered_map< const Port *, LogicVector > output_values_;

    /// Pin type and width used to construct each existing PinState.
    std::unordered_map< const Component *, std::pair< int, int > > pin_shapes_;

    /// Owned component-specific states indexed by borrowed Component pointers.
    std::unordered_map< const Component *, ComponentState * > component_states_;
};

/**
 * @brief Writes a circuit state's diagnostic description to a stream.
 * @param cout Output stream.
 * @param state Circuit state to describe.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const CircuitState & state)
{
    cout << state.to_string();
    return cout;
}

#endif // CIRCUITSTATE_H
