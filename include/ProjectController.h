/**
 * @file ProjectController.h
 * @brief Coordinates project ownership, simulation, and live structural edits.
 */

#ifndef PROJECTCONTROLLER_H
#define PROJECTCONTROLLER_H

#include "Project.h"
#include "RuntimePath.h"
#include "Simulator.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

class CircuitState;
class CircuitSession;

/**
 * @brief Application API that keeps runtime instances synchronized with edits.
 *
 * Owns the Project, Simulator, and root CircuitStates. SubcircuitState owns each
 * nested instance. Successful edits preserve surviving CircuitState objects and
 * component states whose configuration is unchanged. A resized or redirected
 * pin receives a fresh PinState; removing a placement destroys its child tree.
 *
 * Edits prepare replacement storage and pending evaluations before committing.
 * If preparation throws, the previous structure, runtime storage, and queue
 * remain usable. Call run() after editing or setting inputs to settle signals.
 *
 * @note This is an untimed simulator: structural edits reschedule all roots.
 * Descriptive label edits preserve runtime storage and pending work.
 * No timestamps or in-flight signal delays are preserved by this policy.
 * @warning Use from one thread. Reentrant edits during evaluation are rejected.
 * Do not destroy/reset owned states or manage additional states for these
 * blueprints independently; edits reject untracked live instances.
 *
 * All circuit arguments must belong to this controller's project. Structural
 * edit methods reject any untracked runtime instance anywhere in the project;
 * mutating calls also reject reentry while another call is evaluating or editing.
 * Allocation failures can propagate from operations that prepare storage or work.
 * Creation and copying return borrowed objects; the controller retains ownership.
 * Successful connectivity rebuilds replace Net objects, so reacquire net pointers
 * through surviving ports or nodes after an edit.
 * Electrical width mismatches and multiple output attachments are retained in
 * the wire graph. After run(), inspect CircuitState::diagnostics() for the
 * displayed runtime instance. A settled queue can still contain electrical faults.
 *
 * @see @ref gui_api_reference "GUI API reference"
 *
 * @par Example
 * @code
 * ProjectController controller;
 * Circuit & circuit = controller.create_circuit();
 * Pin & circuit_input = controller.create_pin(circuit, INPUT_PIN);
 * Pin & circuit_output = controller.create_pin(circuit, OUTPUT_PIN);
 * WireNode & node = controller.create_wire_node(circuit);
 * controller.connect(circuit, *circuit_input.output_port(0), node);
 * controller.connect(circuit, *circuit_output.input_port(0), node);
 * CircuitState & state = controller.start(circuit);
 * controller.set(state, circuit_input, LogicVector(1, HIGH));
 * bool settled = controller.run();
 * controller.create_not_gate(circuit); // state remains the same object.
 * settled = controller.run();
 * @endcode
 */
class ProjectController
{
public:
    /** @brief Creates an empty project controller with no active runtime roots. */
    ProjectController() : lifetime_(std::make_shared< int >(0)), next_runtime_id_(0), revision_(0), busy_(false), batching_(false), batch_failed_(false), structural_changed_(false) {}

    /**
     * @brief Disallows copying ownership of blueprints and runtime hierarchies.
     * @param other Source controller; this constructor is deleted.
     */
    ProjectController(const ProjectController & other) = delete;

    /**
     * @brief Disallows copy assignment of project and simulation ownership.
     * @param other Source controller; this operator is deleted.
     * @return No value is returned because this operator is deleted.
     */
    ProjectController & operator=(const ProjectController & other) = delete;

    /**
     * @brief Disallows moving the owner of borrowed circuit and state references.
     * @param other Source controller; this constructor is deleted.
     */
    ProjectController(ProjectController && other) = delete;

    /**
     * @brief Disallows move assignment of project and simulation ownership.
     * @param other Source controller; this operator is deleted.
     * @return No value is returned because this operator is deleted.
     */
    ProjectController & operator=(ProjectController && other) = delete;

    /**
     * @brief Clears pending work and destroys roots before their blueprints.
     * @note Destroys nested states through their owning SubcircuitState objects.
     * All borrowed references to this controller's objects then become invalid.
     */
    ~ProjectController();

    /** @brief Opens a lightweight blueprint view without starting simulation. */
    CircuitSession open(CircuitId circuit);

    /** @brief Opens a view bound to a particular live root or nested instance. */
    CircuitSession open(CircuitId circuit, const RuntimePath & instance);

    /**
     * @brief Lists live root and nested instances using a circuit blueprint.
     * @param circuit Existing project-scoped circuit ID.
     * @return Value paths in root creation and component traversal order.
     * @note Does not create, stop, reset, or evaluate any runtime instance.
     * Callers must choose explicitly when more than one instance is available.
     * @throws std::invalid_argument If the circuit ID is unknown or deleted.
     * @throws std::logic_error If inspection is attempted during an operation.
     */
    std::vector< RuntimePath > instances(CircuitId circuit) const;

    /** @brief Returns value snapshots of project circuit IDs and display names. */
    std::vector< CircuitInfo > circuit_infos() const;

    /**
     * @brief Renames a circuit without rebuilding its structure or runtime state.
     * @param circuit Existing project-scoped circuit ID.
     * @param name Nonblank project-unique name; surrounding ASCII whitespace is trimmed.
     * @note Uniqueness ignores ASCII letter case. Pending simulation work survives.
     * Failed edits and batches restore the previous name through ProjectEdit.
     */
    void set_circuit_name(CircuitId circuit, const std::string & name);

    /** @brief Returns a copy of the current display name for a live circuit ID. */
    std::string circuit_name(CircuitId circuit) const;

    /** @brief Starts an independent root, returning its non-owning identity. */
    RuntimePath start_instance(CircuitId circuit);

    /** @brief Stops a root identified by a path with no placement steps. */
    void stop(const RuntimePath & instance);

    /** @brief Returns this blueprint's shared default root, creating it lazily. */
    RuntimePath default_instance(CircuitId circuit);

    /**
     * @brief Applies related edits in one transaction and one runtime update.
     * @note A failed inner mutation invalidates the whole batch even if caught.
     * Nested batches and runtime operations inside a batch are rejected.
     */
    void batch(const std::function< void() > & operation);

    /** @brief Monotonic successful edit count; clients can poll for changes. */
    unsigned long revision() const { return revision_; }
    /** @brief Reports evaluation/edit activity so view metadata can wait for a committed graph. */
    bool operation_active() const { return busy_; }

    /** @brief Changes a component label without rescheduling simulation. */
    void set_label(Circuit & circuit, Component & component, const std::string & label);

    /**
     * @brief Returns borrowed project-owned circuit pointers.
     * @return Reference to the owned blueprint list, valid for this controller's lifetime.
     * @note Creation/removal can invalidate list iterators. Each circuit pointer
     * remains valid until remove_circuit() or controller destruction.
     */
    const std::vector< Circuit * > & circuits() const { return project_.circuits(); }

    /**
     * @brief Returns borrowed roots; stop() invalidates the stopped hierarchy.
     * @return Reference to the controller's root-state list; nested instances are not listed.
     * @note start()/stop() can invalidate list iterators. Never delete these pointers.
     */
    const std::vector< CircuitState * > & root_states() const { return root_states_; }

    /**
     * @brief Provides read-only queue inspection.
     * @return Borrowed const reference to the controller's Simulator.
     * @note Queue contents change during input updates, edits, start(), stop(), and run().
     */
    const Simulator & simulator() const { return simulator_; }

    /**
     * @brief Creates an independent runtime hierarchy and schedules it.
     * @param circuit Blueprint owned by this controller, including all referenced children.
     * @throws std::invalid_argument If circuit belongs to another project.
     * @throws std::logic_error If another controller operation is in progress.
     * @return Borrowed state, valid until stop() or controller destruction.
     * @note Does not run evaluations or reset other roots' input values.
     * Reschedules all roots. Repeated calls for the same blueprint create
     * independent instances; a failed start leaves previous roots and work intact.
     */
    CircuitState & start(Circuit & circuit);

    /**
     * @brief Destroys an owned root and removes only its hierarchy's events.
     * @param state Root returned by this controller's start(); not a nested instance.
     * @throws std::invalid_argument If state is foreign or is a nested instance.
     * @throws std::logic_error If another controller operation is in progress.
     * @note Invalidates state and all its descendants. Other roots, their values,
     * and their pending event order survive. The circuit blueprint remains owned.
     */
    void stop(CircuitState & state);

    /**
     * @brief Sets an owned instance's input pin and queues its listeners.
     * @param state Owned root or nested instance containing the pin's runtime value.
     * @param pin INPUT_PIN belonging to state.circuit().
     * @param value New bits; value.width() must equal pin.width().
     * @throws std::invalid_argument If the state/pin ownership or value width is invalid.
     * @throws std::logic_error If pin is an OUTPUT_PIN or another operation is in progress.
     * @note Call run() to settle downstream values. A nested input can be overwritten
     * when its parent placement next evaluates its connected inputs.
     */
    void set(CircuitState & state, Pin & pin, const LogicVector & value);

    /**
     * @brief Toggles all bits of an owned instance's input pin.
     * @param state Owned root or nested instance containing the pin's runtime value.
     * @param pin INPUT_PIN belonging to state.circuit().
     * @throws std::invalid_argument If the state or pin ownership is invalid.
     * @throws std::logic_error If pin is an OUTPUT_PIN or another operation is in progress.
     * @note Exchanges LOW and HIGH; UNKNOWN and ERROR retain their values.
     * Call run() to settle listeners.
     */
    void toggle(CircuitState & state, Pin & pin);

    /**
     * @brief Evaluates pending events; returns false if feedback fails to settle.
     * @return True when the queue is empty, including when no work was pending;
     * false when the simulator's evaluation limit is reached.
     * @throws std::logic_error If another controller operation is in progress.
     * @note Propagates evaluation exceptions. Pending work is cleared after an
     * evaluation exception or limit failure; signal writes are not rolled back.
     * A true result reports queue completion, not circuit validity. Inspect
     * CircuitState::diagnostics() after settling to identify width mismatches,
     * conflicting driven bits, and propagated ERROR values.
     */
    bool run();

    /**
     * @brief Creates a circuit blueprint.
     * @return Borrowed reference to a new, empty blueprint owned by this controller.
     * @note Does not create a runtime instance; use start() when simulation is needed.
     */
    Circuit & create_circuit();

    /**
     * @brief Creates and names an empty circuit in one transaction.
     * @param name Nonblank project-unique name; follows set_circuit_name() validation.
     * @return The new circuit's project-scoped ID.
     * @note A rejected name leaves no circuit behind. Does not open an editor or start simulation.
     */
    CircuitId add_circuit(const std::string & name);

    /**
     * @brief Copies structure, sharing referenced child blueprints, without runtime values.
     * @param source Blueprint owned by this controller to duplicate.
     * @return Borrowed reference to the new blueprint with separate components and wires.
     * @throws std::invalid_argument If source is foreign or a component type is unsupported.
     * @note Preserves labels, interface order, and connections. Copied placements
     * reuse the source's child blueprints; this is not a recursive blueprint copy.
     */
    Circuit & copy_circuit(const Circuit & source);

    /**
     * @brief Copies a supported component with disconnected ports.
     * @param destination Blueprint owned by this controller that will own the copy.
     * @param source Component in any blueprint owned by this controller.
     * @return Borrowed reference to the new component, retaining configuration and labels.
     * @throws std::invalid_argument If ownership is invalid, the exact type is
     * unsupported, or copying a placement would create recursive circuit use.
     * @note Supports Pin, Subcircuit, and the seven primitive gate types. A copied
     * placement shares its child blueprint and receives independent runtime storage.
     */
    Component & copy_component(Circuit & destination, const Component & source);

    /**
     * @brief Creates an unattached copy of a node's width constraint and label.
     * @param destination Blueprint owned by this controller that will own the node.
     * @param source Node in any blueprint owned by this controller.
     * @return Borrowed reference to a new node with no incident edges or attached ports.
     * @throws std::invalid_argument If either object's project ownership is invalid.
     */
    WireNode & copy_wire_node(Circuit & destination, const WireNode & source);

    /**
     * @brief Deletes an unused blueprint; stop its roots and remove its placements first.
     * @param circuit Blueprint owned by this controller to destroy.
     * @throws std::invalid_argument If circuit belongs to another project.
     * @throws std::logic_error If live instances or referencing placements remain,
     * another operation is in progress, or untracked runtime states exist.
     * @note Invalidates the blueprint and all its contents. Removes placements
     * contained inside it while preserving the child blueprints those placements used.
     */
    void remove_circuit(Circuit & circuit);

    /**
     * @brief Adds a placement and creates its runtime child in every live parent.
     * @param parent Blueprint owned by this controller that will contain the placement.
     * @param circuit Child blueprint owned by the same controller to reuse.
     * @return Borrowed reference to the new Subcircuit component owned by parent.
     * @throws std::invalid_argument If either blueprint is foreign or the placement
     * would create direct or indirect recursive circuit use.
     * @note Placement ports follow the child's interface pin order. Each parent
     * runtime instance gets an independent child state; connections remain explicit.
     */
    Subcircuit & create_subcircuit(Circuit & parent, Circuit & circuit);

    /**
     * @brief Adds an interface pin and synchronizes every placement and live instance.
     * @param circuit Blueprint owned by this controller that will own the pin.
     * @param type INPUT_PIN for an external input or OUTPUT_PIN for an external output.
     * @param width Positive number of signal bits; defaults to one.
     * @return Borrowed reference to the new interface component.
     * @throws std::invalid_argument If ownership, type, or width is invalid.
     * @note An INPUT_PIN drives an internal OUTPUT port; an OUTPUT_PIN reads an
     * internal INPUT port. New runtime values are LOW and UNKNOWN, respectively.
     */
    Pin & create_pin(Circuit & circuit, PinType type, int width = 1);

    /**
     * @brief Deletes a component and its runtime storage, retaining other states.
     * @param circuit Blueprint owned by this controller that contains component.
     * @param component Gate, pin, or placement owned by circuit to remove.
     * @throws std::invalid_argument If circuit or a component ownership check fails.
     * @throws std::logic_error If a placement registry check fails or editing is disallowed.
     * @note Invalidates the component and its ports; wire geometry remains.
     * Removing a placement destroys its child states in every live instance of
     * this parent blueprint, while other placements and the child blueprint survive.
     * Removing a pin also updates every placement's corresponding interface port.
     */
    void remove_component(Circuit & circuit, Component & component);

    /**
     * @brief Resizes a pin, resetting its state while retaining its connections.
     * @param circuit Blueprint owned by this controller that contains pin.
     * @param pin Interface component owned by circuit to resize.
     * @param width New positive signal width in bits.
     * @throws std::invalid_argument If ownership is invalid or width is not positive.
     * @note An actual width change resets this pin's runtime storage, updates
     * placement port widths, and preserves node attachments at both levels.
     * Incompatible connected widths are reported by Net::width_mismatch() and
     * CircuitState::diagnostics(); other ports and node constraints are not resized.
     * Supplying the existing width preserves the pin's runtime storage.
     */
    void set_pin_width(Circuit & circuit, Pin & pin, int width);

    /**
     * @brief Changes a pin's role, replacing its ports and resetting its state.
     * @param circuit Blueprint owned by this controller that contains pin.
     * @param pin Interface component owned by circuit to change.
     * @param type New interface role: INPUT_PIN or OUTPUT_PIN.
     * @throws std::invalid_argument If ownership or type is invalid.
     * @note An actual role change invalidates the pin's old port and matching placement ports,
     * detaches their connections, and appends the pin to its new direction's list.
     * The Pin object survives; its state is reset. An unchanged role preserves its storage.
     */
    void set_pin_type(Circuit & circuit, Pin & pin, PinType type);

    /**
     * @brief Relabels an interface pin and its placement ports.
     * @param circuit Blueprint owned by this controller that contains pin.
     * @param pin Interface component owned by circuit to relabel.
     * @param label New descriptive text; an empty label is allowed.
     * @throws std::invalid_argument If circuit or pin ownership is invalid.
     * @note Preserves connections and runtime values. Matching labels do not connect wires.
     */
    void set_pin_label(Circuit & circuit, Pin & pin, const std::string & label);

    /**
     * @brief Reorders interface pins while retaining compatible port connections.
     * @param circuit Blueprint owned by this controller that contains pin.
     * @param pin Interface component to move within its current input or output list.
     * @param index New zero-based position in that direction's interface list.
     * @throws std::invalid_argument If circuit or pin ownership is invalid.
     * @throws std::out_of_range If index is outside that direction's current list.
     * @note Updates placement port order while retaining surviving port identities and wiring.
     */
    void move_pin(Circuit & circuit, Pin & pin, int index);

    /**
     * @brief Creates a disconnected AND gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @param input_count Positive number of single-bit input ports; defaults to two.
     * @return Borrowed reference to the new single-output gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is not positive.
     */
    AndGate & create_and_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected OR gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @param input_count Positive number of single-bit input ports; defaults to two.
     * @return Borrowed reference to the new single-output gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is not positive.
     */
    OrGate & create_or_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected NOT gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @return Borrowed reference to the new gate with one single-bit input and output.
     * @throws std::invalid_argument If circuit belongs to another project.
     */
    NotGate & create_not_gate(Circuit & circuit);

    /**
     * @brief Creates a disconnected NAND gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @param input_count Positive number of single-bit input ports; defaults to two.
     * @return Borrowed reference to the new single-output gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is not positive.
     */
    NandGate & create_nand_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected NOR gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @param input_count Positive number of single-bit input ports; defaults to two.
     * @return Borrowed reference to the new single-output gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is not positive.
     */
    NorGate & create_nor_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected XOR gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @param input_count Positive number of single-bit input ports; defaults to two.
     * @return Borrowed reference to the new single-output gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is not positive.
     */
    XorGate & create_xor_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected XNOR gate.
     * @param circuit Blueprint owned by this controller that will own the gate.
     * @param input_count Positive number of single-bit input ports; defaults to two.
     * @return Borrowed reference to the new single-output gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is not positive.
     */
    XnorGate & create_xnor_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates an unattached wire node; zero width means unconstrained.
     * @param circuit Blueprint owned by this controller that will own the node.
     * @param width Nonnegative width constraint in bits; zero imposes no constraint.
     * @param label Descriptive text; matching labels do not create connections.
     * @return Borrowed reference to a node with no edges or attached ports.
     * @throws std::invalid_argument If circuit is foreign or width is negative.
     * @note WireNode::width() reports the constraint; a connected net reports the
     * resolved signal width. Rebuilds nets even when adding unattached geometry.
     */
    WireNode & create_wire_node(Circuit & circuit, int width = 0, const std::string & label = "");

    /**
     * @brief Creates an explicit wire segment while retaining electrical faults.
     * @param circuit Blueprint owned by this controller that contains both endpoints.
     * @param first First existing wire node owned by circuit.
     * @param second Distinct second node owned by circuit.
     * @return Borrowed reference to the new undirected edge owned by circuit.
     * @throws std::invalid_argument If ownership is invalid, endpoints coincide,
     * or a direct edge already exists.
     * @note Joins electrical regions without automatically creating nodes. Geometric
     * crossings and matching labels do not join regions unless edges/nodes connect them.
     * Width mismatches and multiple output attachments are accepted. Call run(),
     * then query CircuitState::diagnostics() for faults in each runtime instance.
     */
    WireEdge & create_wire_edge(Circuit & circuit, WireNode & first, WireNode & second);

    /**
     * @brief Removes a segment and recomputes its resulting electrical regions.
     * @param circuit Blueprint owned by this controller that contains edge.
     * @param edge Wire segment owned by circuit to destroy.
     * @throws std::invalid_argument If circuit or edge ownership is invalid.
     * @note Invalidates edge, retains both endpoints, and rebuilds nets. Alternate
     * paths can keep endpoints connected; fragments without a driver become UNKNOWN.
     */
    void remove_wire_edge(Circuit & circuit, WireEdge & edge);

    /**
     * @brief Removes a node and incident segments, detaching its ports.
     * @param circuit Blueprint owned by this controller that contains node.
     * @param node Wire node owned by circuit to destroy.
     * @throws std::invalid_argument If circuit or node ownership is invalid.
     * @note Invalidates node and every incident edge. Attached components and
     * ports survive disconnected; remaining neighbors are not joined automatically.
     */
    void remove_wire_node(Circuit & circuit, WireNode & node);

    /**
     * @brief Attaches a port to an explicit node and migrates runtime net values.
     * @param circuit Blueprint owned by this controller that contains port and node.
     * @param port Previously unattached component port owned by circuit.
     * @param node Existing wire node owned by circuit to receive the attachment.
     * @throws std::invalid_argument If ownership is invalid or port is already attached.
     * @note Creates no nodes or edges. Reattaching to the same node is also rejected;
     * use create_wire_edge() to connect two wire nodes, and run() to settle listeners.
     * Retains incompatible widths and multiple output drivers for runtime diagnosis.
     * Matching driven bits can share a net; opposing LOW/HIGH bits resolve to ERROR.
     */
    void connect(Circuit & circuit, Port & port, WireNode & node);

    /** @brief Connects ports atomically, preserving their existing wire branches. */
    void connect(Circuit & circuit, Port & first, Port & second);

    /**
     * @brief Detaches a port; fragments without a driver become UNKNOWN.
     * @param circuit Blueprint owned by this controller that contains port.
     * @param port Owned component port to detach; an already detached port is allowed.
     * @throws std::invalid_argument If circuit or port ownership is invalid.
     * @note Retains the port, component, and wire geometry. A detached output's
     * last driven value remains cached independently of wire connectivity.
     */
    void disconnect(Circuit & circuit, Port & port);

    /**
     * @brief Clears a blueprint while preserving its own surviving runtime instances.
     * @param circuit Blueprint owned by this controller whose contents will be removed.
     * @throws std::invalid_argument If circuit belongs to another project.
     * @note Destroys all contained components, nodes, edges, nets, and component
     * states. The Circuit and its root/child instances remain alive and empty;
     * referencing placements remain but lose their interface ports. IDs are not reset.
     */
    void clear_circuit(Circuit & circuit);

private:
    friend class CircuitSession;

    void change(const std::function< void() > & operation, bool structural, bool grouped = false);
    void require_runtime_idle();
    CircuitState & resolve_instance(const RuntimePath & instance) const;
    /**
     * @brief Runs a structural operation and commits matching runtime storage and work.
     * @param operation Callable that edits this controller's Project once permission is granted.
     * @throws std::logic_error If evaluation/editing is active or any instance is untracked.
     * @note Preparation failures restore the previous graph, runtime maps, and queue.
     */
    void edit(const std::function< void() > & operation);

    /**
     * @brief Appends an instance and all its nested instances to a borrowed list.
     * @param state Instance at the start of the subtree to traverse.
     * @param[in,out] instances Destination list; existing entries are retained.
     */
    void collect_instances(CircuitState & state, std::vector< CircuitState * > & instances) const;

    /** @brief Appends occurrence paths for a blueprint, including nested placements. */
    void collect_instance_paths(const CircuitState & state, const RuntimePath & path, CircuitId circuit, std::vector< RuntimePath > & instances) const;

    /**
     * @brief Checks that every live instance of every owned blueprint is tracked.
     * @throws std::logic_error If live-state counts differ from the managed hierarchy counts.
     */
    void require_managed_instances() const;

    /**
     * @brief Checks that a state appears in an owned runtime hierarchy.
     * @param state Root or nested instance whose ownership is checked.
     * @throws std::invalid_argument If state does not belong to this controller.
     */
    void require_state(const CircuitState & state) const;

    /**
     * @brief Checks that a state is managed and its blueprint contains a pin.
     * @param state Runtime instance whose blueprint is examined.
     * @param pin Interface component whose membership is checked.
     * @throws std::invalid_argument If state is foreign or its blueprint does not own pin.
     */
    void require_pin(const CircuitState & state, const Pin & pin) const;

    Project project_; ///< Owns all blueprints and their placement registry.
    Simulator simulator_; ///< Owns pending events borrowing the managed runtime hierarchies.
    std::vector< CircuitState * > root_states_; ///< Owns roots; nested instances are owned by placements' runtime states.
    std::shared_ptr< int > lifetime_; ///< Sessions only retain a weak lifetime token.
    std::unordered_map< int, CircuitState * > runtime_roots_; ///< Live root identities.
    std::unordered_map< int, RuntimeId > default_roots_; ///< Lazily selected roots per blueprint.
    int next_runtime_id_; ///< IDs are consumed even when starting a root fails.
    unsigned long revision_; ///< Successful transaction count, including metadata.
    bool busy_; ///< True during a mutating operation or evaluation; prevents reentry on one thread.
    bool batching_;
    bool batch_failed_;
    bool structural_changed_;
};

#include "CircuitSession.h"

#endif // PROJECTCONTROLLER_H
