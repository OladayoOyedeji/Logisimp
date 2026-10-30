
/**
 * @file Simulator.h
 * @brief Evaluates components whose input values have changed.
 * @author Cole Schwandt
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstddef>
#include <functional>
#include <ostream>
#include <queue>
#include <string>
#include <unordered_set>

class Component;
class CircuitState;
class LogicVector;
class Net;
class Port;

/**
 * @brief Schedules and evaluates component changes during simulation.
 *
 * Uses a FIFO event queue to evaluate components whose inputs
 * may have changed. Duplicate pending evaluations are prevented
 * using a hash set of component and circuit state pairs.
 *
 * Events reference existing components and runtime states without
 * owning them. Signal values are stored in CircuitState.
 *
 * The simulator propagates changes until the queue is empty
 * or the maximum number of evaluations is reached.
 */
class Simulator
{
public:
    /**
     * @brief Represents a pending component evaluation.
     *
     * Identifies a component within a particular CircuitState.
     * The same component blueprint can have separate events
     * for different circuit instances.
     */
    struct Event
    {
        /**
         * @brief Creates an event for a component in a circuit instance.
         * @param state Runtime state containing the component.
         * @param component Component to evaluate.
         */
        Event(CircuitState & state, Component & component)
            : state(&state), component(&component)
        {}

        /**
         * @brief Compares two events by their runtime state and component.
         * @param other Event to compare.
         * @return True if both events reference the same state and component.
         */
        bool operator==(const Event & other) const
        {
            return state == other.state && component == other.component;
        }

        /// Borrowed pointer to the circuit instance's runtime state.
        CircuitState * state;

        /// Borrowed pointer to the component being evaluated.
        Component * component;
    };

    /**
     * @brief Computes hash values for simulator events.
     *
     * Combines the hashes of the runtime state and component
     * pointers so events can be stored in an unordered set.
     *
     * The hash combination technique was adapted from Boost.ContainerHash.
     *
     * @see https://www.boost.org/doc/libs/latest/libs/container_hash/doc/html/hash.htm
     */
    struct EventHash
    {
        /**
         * @brief Computes a hash for a component evaluation event.
         * @param event Event to hash.
         * @return Combined hash of the state and component pointers.
         */
        std::size_t operator()(const Event & event) const
        {
            std::size_t state_hash = std::hash< CircuitState * >()(event.state);
            std::size_t component_hash = std::hash< Component * >()(event.component);
            return state_hash ^ (component_hash + (state_hash << 6) + (state_hash >> 2));
        }
    };

    /**
     * @brief Constructs a simulator with an empty event queue.
     */
    Simulator()
    {}

    /** @brief Simulators cannot be copied. */
    Simulator(const Simulator & other) = delete;

    /** @brief Simulators cannot be assigned. */
    Simulator & operator=(const Simulator & other) = delete;

    /** @brief Simulators cannot be moved. */
    Simulator(Simulator && other) = delete;

    /** @brief Simulators cannot be move-assigned. */
    Simulator & operator=(Simulator && other) = delete;

    /**
     * @brief Returns the pending evaluation queue.
     * @return Const reference to the FIFO event queue.
     */
    const std::queue< Event > & work_queue() const { return work_queue_; }

    /**
     * @brief Returns the set of currently queued events.
     * @return Const reference to the set of pending state-component pairs.
     */
    const std::unordered_set< Event, EventHash > & queued_components() const { return queued_components_; }

    /**
     * @brief Schedules a component for evaluation.
     * @param state Runtime state containing the component.
     * @param component Component to evaluate, or nullptr to do nothing.
     * @note Does not enqueue an event already pending for the same state and component.
     */
    void enqueue(CircuitState & state, Component * component);

    /**
     * @brief Schedules all components in a circuit instance hierarchy.
     * @param state Root runtime state of the hierarchy to schedule.
     * @note Recursively schedules nested subcircuit states without executing them.
     */
    void schedule(CircuitState & state);

    /**
     * @brief Schedules the parent placement of a nested circuit instance.
     * @param state Child runtime state whose parent must be notified.
     * @note Does nothing if the supplied state is a root instance.
     */
    void notify_parent(CircuitState & state);

    /**
     * @brief Updates a net value and schedules affected components.
     * @param state Runtime state containing the net.
     * @param net Net whose value is being driven.
     * @param value New signal value.
     * @throws std::invalid_argument If the signal width does not match the net.
     * @note Unchanged values do not schedule additional evaluations.
     */
    void drive(CircuitState & state, Net & net, const LogicVector & value);

    /**
     * @brief Records an output value and drives its currently connected net.
     * @param state Runtime instance containing the output port.
     * @param port Output port belonging to the instance's blueprint.
     * @param value Signal matching the port width.
     * @note Disconnected outputs retain their value for later live connections.
     */
    void drive(CircuitState & state, Port & port, const LogicVector & value);

    /**
     * @brief Processes pending component evaluations.
     *
     * Evaluates events in FIFO order until the queue is empty
     * or the maximum work limit is reached.
     *
     * @return True if all pending events were processed; false if the work limit was reached.
     * @note Clears remaining events if the work limit is reached or an exception occurs.
     * Previously updated signal values are not rolled back.
     */
    bool run();

    /**
     * @brief Removes all pending component evaluations.
     * @note Does not modify stored runtime signal values.
     */
    void clear();

    /**
     * @brief Removes pending work for one instance and its descendants.
     * @param state Runtime hierarchy whose events are removed before it is stopped.
     * @note Other roots retain their pending work and FIFO ordering.
     * Allocation failures leave the original queue and duplicate-event set unchanged.
     * The runtime states and their signal values are not destroyed or modified.
     */
    void discard(CircuitState & state);

    /**
     * @brief Exchanges pending work after its replacement has been prepared.
     * @param other Simulator whose queue and duplicate-event set are exchanged
     * with this simulator's pending work.
     * @note Does not allocate, evaluate events, or change runtime signal values.
     * Both queues continue to borrow their referenced components and states.
     */
    void swap_pending(Simulator & other) noexcept;

    /**
     * @brief Returns a description of the simulator's pending events.
     * @return String containing diagnostic information about the event queue.
     */
    std::string to_string() const;

private:
    /// FIFO queue of pending component evaluations.
    std::queue< Event > work_queue_;

    /// Set of pending events used to prevent duplicate scheduling.
    std::unordered_set< Event, EventHash > queued_components_;
};

/**
 * @brief Writes the simulator's diagnostic description to a stream.
 * @param cout Output stream.
 * @param simulator Simulator to describe.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const Simulator & simulator)
{
    cout << simulator.to_string();
    return cout;
}

#endif // SIMULATOR_H
