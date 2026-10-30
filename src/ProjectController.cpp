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
#include <limits>
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
    lifetime_.reset();
    simulator_.clear();
    for (CircuitState * state : root_states_)
    {
        delete state;
    }
}

CircuitSession ProjectController::open(CircuitId circuit)
{
    if (project_.find_circuit(circuit) == nullptr)
    {
        throw std::invalid_argument("Circuit ID does not belong to this project");
    }
    return CircuitSession(*this, circuit);
}

CircuitSession ProjectController::open(CircuitId circuit, const RuntimePath & instance)
{
    require_runtime_idle();
    Circuit * blueprint = project_.find_circuit(circuit);
    if (blueprint == nullptr || &resolve_instance(instance).circuit() != blueprint)
    {
        throw std::invalid_argument("Runtime instance does not use the selected blueprint");
    }
    return CircuitSession(*this, circuit, instance);
}

std::vector< RuntimePath > ProjectController::instances(CircuitId circuit) const
{
    if (busy_)
    {
        throw std::logic_error("Runtime inspection is not allowed while evaluating or editing");
    }
    if (project_.find_circuit(circuit) == nullptr)
    {
        throw std::invalid_argument("Circuit ID does not belong to this project");
    }
    std::vector< int > roots;
    for (const auto & root : runtime_roots_)
    {
        roots.push_back(root.first);
    }
    std::sort(roots.begin(), roots.end());
    std::vector< RuntimePath > result;
    for (int root : roots)
    {
        collect_instance_paths(*runtime_roots_.at(root), RuntimePath(RuntimeId(root)), circuit, result);
    }
    return result;
}

std::vector< CircuitInfo > ProjectController::circuit_infos() const
{
    return project_.circuit_infos();
}

void ProjectController::set_circuit_name(CircuitId circuit, const std::string & name)
{
    change([&]() { project_.set_circuit_name(circuit, name); }, false);
}

std::string ProjectController::circuit_name(CircuitId circuit) const
{
    return project_.circuit_name(circuit);
}

void ProjectController::require_runtime_idle()
{
    if (batching_)
    {
        batch_failed_ = true;
        throw std::logic_error("Runtime operations are not allowed inside an edit batch");
    }
}

RuntimePath ProjectController::start_instance(CircuitId circuit)
{
    require_runtime_idle();
    Circuit * blueprint = project_.find_circuit(circuit);
    if (blueprint == nullptr)
    {
        throw std::invalid_argument("Circuit ID does not belong to this project");
    }
    RuntimeId id(next_runtime_id_);
    start(*blueprint);
    return RuntimePath(id);
}

RuntimePath ProjectController::default_instance(CircuitId circuit)
{
    require_runtime_idle();
    if (busy_)
    {
        throw std::logic_error("Controller is already evaluating or editing");
    }
    if (project_.find_circuit(circuit) == nullptr)
    {
        throw std::invalid_argument("Circuit ID does not belong to this project");
    }
    auto existing = default_roots_.find(circuit.value);
    if (existing != default_roots_.end())
    {
        return RuntimePath(existing->second);
    }

    // Allocate the default-map entry before starting, so failure cannot leave
    // a successful but unreachable default simulation behind.
    default_roots_.emplace(circuit.value, RuntimeId());
    try
    {
        RuntimePath instance = start_instance(circuit);
        default_roots_.at(circuit.value) = instance.root;
        return instance;
    }
    catch (...)
    {
        default_roots_.erase(circuit.value);
        throw;
    }
}

CircuitState & ProjectController::resolve_instance(const RuntimePath & instance) const
{
    if (busy_)
    {
        throw std::logic_error("Runtime inspection is not allowed while evaluating or editing");
    }
    auto root = runtime_roots_.find(instance.root.value);
    if (root == runtime_roots_.end())
    {
        throw std::invalid_argument("Runtime root has been stopped or does not exist");
    }
    CircuitState * state = root->second;
    for (ComponentId id : instance.placements)
    {
        if (id.is_scoped() && !id.matches(state->circuit().id(), lifetime_))
        {
            throw std::invalid_argument("Runtime placement handle belongs to another circuit or controller");
        }
        const Component * component = state->circuit().find_component(id.value);
        const Subcircuit * placement = dynamic_cast< const Subcircuit * >(component);
        if (placement == nullptr)
        {
            throw std::invalid_argument("Runtime placement has been deleted or does not exist");
        }
        SubcircuitState & child = dynamic_cast< SubcircuitState & >(state->component_state(*placement));
        state = &child.child_state();
    }
    return *state;
}

void ProjectController::stop(const RuntimePath & instance)
{
    require_runtime_idle();
    if (!instance.placements.empty())
    {
        throw std::invalid_argument("Only a root runtime instance can be stopped");
    }
    stop(resolve_instance(instance));
}

CircuitState & ProjectController::start(Circuit & circuit)
{
    require_runtime_idle();
    ControllerActivity activity(busy_);
    project_.require_circuit(circuit);
    if (next_runtime_id_ == std::numeric_limits< int >::max())
    {
        throw std::overflow_error("Runtime root IDs exhausted");
    }
    int id = next_runtime_id_++;
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
        runtime_roots_.emplace(id, state);
        root_states_.push_back(state);
        simulator_.swap_pending(pending);
    }
    catch (...)
    {
        runtime_roots_.erase(id);
        delete state;
        throw;
    }
    return *state;
}

void ProjectController::stop(CircuitState & state)
{
    require_runtime_idle();
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
    for (auto root = runtime_roots_.begin(); root != runtime_roots_.end(); ++root)
    {
        if (root->second == &state)
        {
            for (auto selected = default_roots_.begin(); selected != default_roots_.end();)
            {
                if (selected->second.value == root->first)
                {
                    selected = default_roots_.erase(selected);
                }
                else
                {
                    ++selected;
                }
            }
            runtime_roots_.erase(root);
            break;
        }
    }
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

void ProjectController::collect_instance_paths(const CircuitState & state, const RuntimePath & path, CircuitId circuit, std::vector< RuntimePath > & instances) const
{
    if (state.circuit().id() == circuit)
    {
        instances.push_back(path);
    }
    for (const Component * component : state.circuit().components())
    {
        const Subcircuit * placement = dynamic_cast< const Subcircuit * >(component);
        if (placement == nullptr)
        {
            continue;
        }
        RuntimePath child_path = path;
        child_path.placements.push_back(ComponentId(placement->id(), state.circuit().id(), lifetime_));
        const SubcircuitState & child = dynamic_cast< const SubcircuitState & >(state.component_state(*placement));
        collect_instance_paths(child.child_state(), child_path, circuit, instances);
    }
}

void ProjectController::require_managed_instances() const
{
    std::vector< CircuitState * > instances;
    for (CircuitState * root : root_states_)
    {
        collect_instances(*root, instances);
    }
    std::unordered_map< const Circuit *, int > counts;
    for (CircuitState * state : instances)
    {
        ++counts[&state->circuit()];
    }
    for (Circuit * circuit : project_.circuits())
    {
        auto managed = counts.find(circuit);
        int count = managed == counts.end() ? 0 : managed->second;
        if (count != circuit->active_states_)
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
    if (state.circuit().find_component(pin.id()) != &pin)
    {
        throw std::invalid_argument("Pin does not belong to this runtime instance's blueprint");
    }
}

void ProjectController::set(CircuitState & state, Pin & pin, const LogicVector & value)
{
    require_runtime_idle();
    ControllerActivity activity(busy_);
    require_pin(state, pin);
    pin.set(value, simulator_, state);
}

void ProjectController::toggle(CircuitState & state, Pin & pin)
{
    require_runtime_idle();
    ControllerActivity activity(busy_);
    require_pin(state, pin);
    pin.toggle(simulator_, state);
}

bool ProjectController::run()
{
    require_runtime_idle();
    ControllerActivity activity(busy_);
    return simulator_.run();
}

void ProjectController::edit(const std::function< void() > & operation)
{
    change(operation, true);
}

void ProjectController::batch(const std::function< void() > & operation)
{
    if (batching_)
    {
        batch_failed_ = true;
        throw std::logic_error("Nested edit batches are not supported");
    }
    change(operation, false, true);
}

void ProjectController::change(const std::function< void() > & operation, bool structural, bool grouped)
{
    if (batching_)
    {
        try
        {
            operation();
            structural_changed_ = structural_changed_ || structural;
        }
        catch (...)
        {
            batch_failed_ = true;
            throw;
        }
        return;
    }
    ControllerActivity activity(busy_);
    // The raw Project API keeps its old guard. Permission here is private and
    // covers only states we can synchronize; an independently created state
    // must never silently become a dangling borrower of replaced nets.
    require_managed_instances();
    ProjectEdit structural_change(project_);
    std::vector< StateUpdate * > updates;
    batching_ = grouped;
    batch_failed_ = false;
    structural_changed_ = structural;
    try
    {
        operation();
        batching_ = false;
        if (batch_failed_)
        {
            throw std::logic_error("An edit failed inside the batch; all changes were rolled back");
        }
        if (structural_changed_)
        {
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

            // Prepare all allocation and validation before retiring old graph
            // objects. Metadata-only transactions preserve the existing queue.
            for (StateUpdate * update : updates)
            {
                update->commit();
            }
            simulator_.swap_pending(pending);
        }
        structural_change.commit();
        ++revision_;
    }
    catch (...)
    {
        batching_ = false;
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

CircuitId ProjectController::add_circuit(const std::string & name)
{
    CircuitId result;
    edit([&]()
    {
        Circuit & circuit = project_.create_circuit();
        project_.set_circuit_name(circuit.id(), name);
        result = circuit.id();
    });
    return result;
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
    change([&]() { project_.set_pin_label(circuit, pin, label); }, false);
}

void ProjectController::set_label(Circuit & circuit, Component & component, const std::string & label)
{
    change([&]()
    {
        project_.require_circuit(circuit);
        if (circuit.find_component(component.id()) != &component)
        {
            throw std::invalid_argument("Component does not belong to this circuit");
        }
        Pin * pin = dynamic_cast< Pin * >(&component);
        if (pin != nullptr)
        {
            project_.set_pin_label(circuit, *pin, label);
        }
        else
        {
            component.label() = label;
        }
    }, false);
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
