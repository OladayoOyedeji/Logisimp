/**
 * @file ProjectController.cpp
 * @brief Coordinates atomic structural edits and persistent runtime instances.
 */

#include "ProjectController.h"

#include "CircuitState.h"
#include "ProjectEdit.h"
#include "StateUpdate.h"
#include "Subcircuit.h"
#include "SubcircuitState.h"

#include <algorithm>
#include <stdexcept>

namespace
{
/**
 * @brief Marks a controller operation as active until this guard leaves scope.
 *
 * Evaluation may call application code. Reject reentrant edits before granting
 * access to a candidate graph; this guard also restores access after exceptions.
 */
class ControllerActivity
{
public:
    /**
     * @brief Rejects nested controller operations and marks the controller busy.
     * @param busy Borrowed controller activity flag, which must outlive this guard.
     * @throws std::logic_error If the controller is already evaluating or editing.
     */
    ControllerActivity(bool & busy) : busy_(busy)
    {
        if (busy_)
        {
            throw std::logic_error("Controller is already evaluating or editing");
        }
        busy_ = true;
    }

    /** @brief Clears the activity flag when the guarded operation leaves scope. */
    ~ControllerActivity() { busy_ = false; }

private:
    /// Borrowed flag reset on destruction, including during exception unwinding.
    bool & busy_;
};
}

ProjectController::~ProjectController()
{
    simulator_.clear();
    for (CircuitState * state : root_states_)
    {
        delete state;
    }
}

CircuitState & ProjectController::start(Circuit & circuit)
{
    ControllerActivity activity(busy_);
    project_.require_circuit(circuit);
    CircuitState * state = new CircuitState(circuit);
    try
    {
        // Build replacement pending work separately so a failed start cannot
        // leave events borrowing a hierarchy that was never returned to callers.
        Simulator pending;
        for (CircuitState * root : root_states_)
        {
            pending.schedule(*root);
        }
        pending.schedule(*state);
        root_states_.push_back(state);
        simulator_.swap_pending(pending);
    }
    catch (...)
    {
        delete state;
        throw;
    }
    return *state;
}

void ProjectController::stop(CircuitState & state)
{
    ControllerActivity activity(busy_);
    std::vector< CircuitState * >::iterator position = std::find(root_states_.begin(), root_states_.end(), &state);
    if (position == root_states_.end())
    {
        throw std::invalid_argument("Only a root owned by this controller can be stopped");
    }
    // Filtering is prepared before destruction and retains every other root's
    // pending work, including events for their nested instances.
    simulator_.discard(state);
    root_states_.erase(position);
    delete &state;
}

void ProjectController::collect_instances(CircuitState & state, std::vector< CircuitState * > & instances) const
{
    instances.push_back(&state);
    for (Component * component : state.circuit().components())
    {
        Subcircuit * placement = dynamic_cast< Subcircuit * >(component);
        if (placement != nullptr)
        {
            SubcircuitState & child = dynamic_cast< SubcircuitState & >(state.component_state(*placement));
            collect_instances(child.child_state(), instances);
        }
    }
}

void ProjectController::require_managed_instances() const
{
    std::vector< CircuitState * > instances;
    for (CircuitState * root : root_states_)
    {
        collect_instances(*root, instances);
    }
    for (Circuit * circuit : project_.circuits())
    {
        int managed = 0;
        for (CircuitState * state : instances)
        {
            if (&state->circuit() == circuit)
            {
                ++managed;
            }
        }
        if (managed != circuit->active_states_)
        {
            throw std::logic_error("Live editing requires every runtime instance in the project to be controller-owned");
        }
    }
}

void ProjectController::require_state(const CircuitState & state) const
{
    std::vector< CircuitState * > instances;
    for (CircuitState * root : root_states_)
    {
        collect_instances(*root, instances);
    }
    if (std::find(instances.begin(), instances.end(), &state) == instances.end())
    {
        throw std::invalid_argument("Runtime instance is not owned by this controller");
    }
}

void ProjectController::require_pin(const CircuitState & state, const Pin & pin) const
{
    require_state(state);
    const std::vector< Component * > & components = state.circuit().components();
    if (std::find(components.begin(), components.end(), &pin) == components.end())
    {
        throw std::invalid_argument("Pin does not belong to this runtime instance's blueprint");
    }
}

void ProjectController::set(CircuitState & state, Pin & pin, const LogicVector & value)
{
    ControllerActivity activity(busy_);
    require_pin(state, pin);
    pin.set(value, simulator_, state);
}

void ProjectController::toggle(CircuitState & state, Pin & pin)
{
    ControllerActivity activity(busy_);
    require_pin(state, pin);
    pin.toggle(simulator_, state);
}

bool ProjectController::run()
{
    ControllerActivity activity(busy_);
    return simulator_.run();
}

void ProjectController::edit(const std::function< void() > & operation)
{
    ControllerActivity activity(busy_);
    // The raw Project API keeps its old guard. Permission here is private and
    // covers only states we can synchronize; an independently created state
    // must never silently become a dangling borrower of replaced nets.
    require_managed_instances();
    ProjectEdit structural_change(project_);
    std::vector< StateUpdate * > updates;
    try
    {
        operation();
        updates.reserve(root_states_.size());
        for (CircuitState * root : root_states_)
        {
            updates.push_back(new StateUpdate(*root));
        }
        Simulator pending;
        for (StateUpdate * update : updates)
        {
            update->schedule(pending);
        }

        // Everything that may allocate or validate has finished. Runtime maps
        // stop borrowing obsolete objects before structural ownership releases
        // them. All roots are scheduled so new listeners see unchanged drivers.
        for (StateUpdate * update : updates)
        {
            update->commit();
        }
        simulator_.swap_pending(pending);
        structural_change.commit();
    }
    catch (...)
    {
        for (StateUpdate * update : updates)
        {
            delete update;
        }
        // StateUpdate cleanup must precede the structural rollback: freshly
        // constructed child states still borrow the candidate blueprint here.
        throw;
    }
    for (StateUpdate * update : updates)
    {
        delete update;
    }
}

Circuit & ProjectController::create_circuit()
{
    Circuit * result = nullptr;
    edit([&]() { result = &project_.create_circuit(); });
    return *result;
}

Circuit & ProjectController::copy_circuit(const Circuit & source)
{
    Circuit * result = nullptr;
    edit([&]() { result = &project_.copy_circuit(source); });
    return *result;
}

Component & ProjectController::copy_component(Circuit & destination, const Component & source)
{
    Component * result = nullptr;
    edit([&]() { result = &project_.copy_component(destination, source); });
    return *result;
}

WireNode & ProjectController::copy_wire_node(Circuit & destination, const WireNode & source)
{
    WireNode * result = nullptr;
    edit([&]() { result = &project_.copy_wire_node(destination, source); });
    return *result;
}

void ProjectController::remove_circuit(Circuit & circuit)
{
    edit([&]()
    {
        project_.require_circuit(circuit);
        if (circuit.active_states_ != 0)
        {
            throw std::logic_error("Stop root instances and remove placements before deleting a blueprint");
        }
        project_.remove_circuit(circuit);
    });
}

Subcircuit & ProjectController::create_subcircuit(Circuit & parent, Circuit & circuit)
{
    Subcircuit * result = nullptr;
    edit([&]() { result = &project_.create_subcircuit(parent, circuit); });
    return *result;
}

Pin & ProjectController::create_pin(Circuit & circuit, PinType type, int width)
{
    Pin * result = nullptr;
    edit([&]() { result = &project_.create_pin(circuit, type, width); });
    return *result;
}

void ProjectController::remove_component(Circuit & circuit, Component & component)
{
    edit([&]() { project_.remove_component(circuit, component); });
}

void ProjectController::set_pin_width(Circuit & circuit, Pin & pin, int width)
{
    edit([&]() { project_.set_pin_width(circuit, pin, width); });
}

void ProjectController::set_pin_type(Circuit & circuit, Pin & pin, PinType type)
{
    edit([&]() { project_.set_pin_type(circuit, pin, type); });
}

void ProjectController::set_pin_label(Circuit & circuit, Pin & pin, const std::string & label)
{
    edit([&]() { project_.set_pin_label(circuit, pin, label); });
}

void ProjectController::move_pin(Circuit & circuit, Pin & pin, int index)
{
    edit([&]() { project_.move_pin(circuit, pin, index); });
}

AndGate & ProjectController::create_and_gate(Circuit & circuit, int input_count)
{
    AndGate * result = nullptr;
    edit([&]() { result = &project_.create_and_gate(circuit, input_count); });
    return *result;
}

OrGate & ProjectController::create_or_gate(Circuit & circuit, int input_count)
{
    OrGate * result = nullptr;
    edit([&]() { result = &project_.create_or_gate(circuit, input_count); });
    return *result;
}

NotGate & ProjectController::create_not_gate(Circuit & circuit)
{
    NotGate * result = nullptr;
    edit([&]() { result = &project_.create_not_gate(circuit); });
    return *result;
}

NandGate & ProjectController::create_nand_gate(Circuit & circuit, int input_count)
{
    NandGate * result = nullptr;
    edit([&]() { result = &project_.create_nand_gate(circuit, input_count); });
    return *result;
}

NorGate & ProjectController::create_nor_gate(Circuit & circuit, int input_count)
{
    NorGate * result = nullptr;
    edit([&]() { result = &project_.create_nor_gate(circuit, input_count); });
    return *result;
}

XorGate & ProjectController::create_xor_gate(Circuit & circuit, int input_count)
{
    XorGate * result = nullptr;
    edit([&]() { result = &project_.create_xor_gate(circuit, input_count); });
    return *result;
}

XnorGate & ProjectController::create_xnor_gate(Circuit & circuit, int input_count)
{
    XnorGate * result = nullptr;
    edit([&]() { result = &project_.create_xnor_gate(circuit, input_count); });
    return *result;
}

WireNode & ProjectController::create_wire_node(Circuit & circuit, int width, const std::string & label)
{
    WireNode * result = nullptr;
    edit([&]() { result = &project_.create_wire_node(circuit, width, label); });
    return *result;
}

WireEdge & ProjectController::create_wire_edge(Circuit & circuit, WireNode & first, WireNode & second)
{
    WireEdge * result = nullptr;
    edit([&]() { result = &project_.create_wire_edge(circuit, first, second); });
    return *result;
}

void ProjectController::remove_wire_edge(Circuit & circuit, WireEdge & edge)
{
    edit([&]() { project_.remove_wire_edge(circuit, edge); });
}

void ProjectController::remove_wire_node(Circuit & circuit, WireNode & node)
{
    edit([&]() { project_.remove_wire_node(circuit, node); });
}

void ProjectController::connect(Circuit & circuit, Port & port, WireNode & node)
{
    edit([&]() { project_.connect(circuit, port, node); });
}

void ProjectController::disconnect(Circuit & circuit, Port & port)
{
    edit([&]() { project_.disconnect(circuit, port); });
}

void ProjectController::clear_circuit(Circuit & circuit)
{
    edit([&]() { project_.clear_circuit(circuit); });
}
