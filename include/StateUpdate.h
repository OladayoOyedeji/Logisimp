/**
 * @file StateUpdate.h
 * @brief Prepares runtime storage for a controlled blueprint edit.
 */

#ifndef STATEUPDATE_H
#define STATEUPDATE_H

#include "CircuitState.h"

#include <vector>

class Simulator;

/**
 * @brief Stages replacement maps without changing an existing circuit instance.
 *
 * Surviving component states and child instances retain their addresses. The
 * structural transaction keeps retired graph objects alive until commit(), so
 * allocation failures can discard this preparation and restore the old graph.
 * This helper borrows its CircuitState; it never replaces the root state.
 * It owns newly allocated component states until commit() transfers them into
 * that instance, and owns nested preparations for surviving child instances.
 */
class StateUpdate
{
public:
    /**
     * @brief Prepares runtime maps for the instance's candidate circuit structure.
     * @param state Existing instance whose identity and compatible component states
     * must survive the edit.
     * @pre Simulation is paused and ProjectEdit retains the original structure.
     * @note New or reshaped pins start LOW for INPUT_PIN and UNKNOWN for OUTPUT_PIN.
     * Surviving compatible output values seed rebuilt nets; fragments without
     * drivers start UNKNOWN.
     * Preparation failures release newly allocated states without publishing maps.
     */
    explicit StateUpdate(CircuitState & state);

    /**
     * @brief Prevents copying ownership of candidate states and nested preparations.
     * @param other Preparation that cannot be copied.
     */
    StateUpdate(const StateUpdate & other) = delete;

    /**
     * @brief Prevents assigning another preparation's owned candidate storage.
     * @param other Preparation that cannot be assigned.
     * @return Assignment is deleted; no reference is returned.
     */
    StateUpdate & operator=(const StateUpdate & other) = delete;

    /**
     * @brief Releases nested preparations and any uncommitted newly created states.
     * @note After commit(), published component states remain owned by CircuitState.
     * An uncommitted update must be destroyed before ProjectEdit rolls back.
     */
    ~StateUpdate();

    /**
     * @brief Queues the candidate hierarchy without evaluating its uncommitted maps.
     * @param simulator Separate pending-work simulator receiving candidate events.
     * @pre The candidate structure and its prepared runtime states remain alive.
     * @warning Do not run the receiving simulator until these runtime updates commit.
     * Existing CircuitState maps still describe the original structure before then.
     */
    void schedule(Simulator & simulator) const;

    /**
     * @brief Publishes prepared storage and releases obsolete component states.
     * @pre Simulation is paused and the candidate structure is still retained.
     * @note Commits surviving child instances first, without allocating. Compatible
     * state objects retain their addresses; removed or reshaped states are deleted.
     * ProjectEdit must release obsolete graph objects only after this returns.
     * Repeated calls after a successful commit do nothing.
     */
    void commit() noexcept;

private:
    /**
     * @brief Builds candidate values, reuses compatible states, and stages children.
     * @note Called during construction. Borrowed existing states are never deleted
     * if preparation fails; only newly created states are released during cleanup.
     */
    void prepare();

    /**
     * @brief Deletes owned nested preparations and uncommitted new component states.
     * @note Does not delete published states or reused states borrowed from state_.
     */
    void clear_prepared() noexcept;

    /// Borrowed existing instance whose runtime maps are replaced on commit.
    CircuitState & state_;

    /// Candidate net values; after commit, holds the old value map until destruction.
    std::unordered_map< const Net *, LogicVector > net_values_;

    /// Candidate last-driven values keyed by borrowed output-port identities.
    std::unordered_map< const Port *, LogicVector > output_values_;

    /// Candidate pin role and width pairs used to determine state compatibility.
    std::unordered_map< const Component *, std::pair< int, int > > pin_shapes_;

    /// Candidate component-state pointers; borrowed reused states and owned new states.
    std::unordered_map< const Component *, ComponentState * > component_states_;

    /// Newly allocated states owned here until commit transfers ownership to state_.
    std::vector< ComponentState * > created_states_;

    /// Owned preparations for existing child instances whose addresses are preserved.
    std::vector< StateUpdate * > children_;

    /// True after publishing maps; prevents cleanup from deleting transferred states.
    bool committed_;
};

#endif // STATEUPDATE_H
