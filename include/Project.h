/**
 * @file Project.h
 * @brief Owns circuit blueprints and manages changes across dependent circuits.
 * @author Cole Schwandt
 */

#ifndef PROJECT_H
#define PROJECT_H

#include "Circuit.h"

#include <string>
#include <unordered_map>
#include <vector>

class Subcircuit;
class ProjectEdit;

/**
 * @brief Owns circuit blueprints and coordinates edits across their placements.
 *
 * Circuits belong to the project. Components, wire nodes, and wire edges
 * belong to their containing circuits. References returned by creation and
 * copying functions are borrowed and must not be deleted by the caller.
 *
 * @note Direct structural editing requires releasing affected runtime states.
 * Use ProjectController to edit live instances without managing that lifetime.
 */
class Project
{
public:
    /**
     * @brief Creates an empty project.
     */
    Project()
        : edit_(nullptr)
    {}

    /**
     * @brief Disallows copying ownership of the project's circuits.
     * @param other Source project; this constructor is deleted.
     */
    Project(const Project & other) = delete;

    /**
     * @brief Disallows copy assignment of circuit ownership and placement records.
     * @param other Source project; this operator is deleted.
     */
    Project & operator=(const Project & other) = delete;

    /**
     * @brief Disallows move construction of the project owner.
     * @param other Source project; this constructor is deleted.
     */
    Project(Project && other) = delete;

    /**
     * @brief Disallows move assignment of the project owner.
     * @param other Source project; this operator is deleted.
     */
    Project & operator=(Project && other) = delete;

    /**
     * @brief Deletes all circuit blueprints owned by this project.
     * @pre No runtime states or queued events depend on the owned circuits.
     *
     * Each circuit also deletes its components, ports, and wire graph.
     */
    ~Project();

    /**
     * @brief Creates an empty circuit blueprint.
     * @return Borrowed reference to the circuit owned by this project.
     */
    Circuit & create_circuit();

    /**
     * @brief Creates a separate copy of a circuit's structure.
     * @param source Circuit in this project to duplicate.
     * @return Borrowed reference to the new project-owned circuit.
     * @throws std::invalid_argument If source is foreign or contains an
     * unsupported component type.
     *
     * Copies components, labels, interface ordering, wire nodes, edges,
     * and port attachments. The destination receives newly created objects
     * and freshly derived nets. Runtime values are not copied.
     *
     * @note Copied subcircuit placements reference the same child blueprints
     * as their sources; nested blueprints are not recursively duplicated.
     * @note A failed copy removes the partially created destination.
     */
    Circuit & copy_circuit(const Circuit & source);

    /**
     * @brief Copies a component into a circuit with disconnected ports.
     * @param destination Circuit in this project that will own the copy.
     * @param source Component owned by a circuit in this project.
     * @return Borrowed reference to the new component.
     * @throws std::invalid_argument If either object is foreign, the exact
     * component type is unsupported, or copying a placement creates recursion.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * Copies configuration and component/port labels for Pin, Subcircuit,
     * and the seven primitive gate types. Custom subclasses are rejected.
     * A copied placement shares its source's child blueprint.
     *
     * @note Copying a pin also synchronizes the destination's placements.
     */
    Component & copy_component(Circuit & destination, const Component & source);

    /**
     * @brief Copies a node's width constraint and label into an unattached node.
     * @param destination Circuit in this project that will own the copy.
     * @param source Wire node owned by a circuit in this project.
     * @return Borrowed reference to the new node.
     * @throws std::invalid_argument If destination or source is foreign.
     * @throws std::logic_error If destination has live runtime states.
     * @throws std::overflow_error If node or derived-net IDs are exhausted.
     *
     * Incident edges and attached ports are not copied.
     */
    WireNode & copy_wire_node(Circuit & destination, const WireNode & source);

    /**
     * @brief Deletes a circuit blueprint and all of its contents.
     * @param circuit Circuit owned by this project.
     * @throws std::invalid_argument If circuit does not belong to this project.
     * @throws std::logic_error If placements still reference the circuit or
     * affected runtime states remain alive.
     *
     * Placements contained inside the circuit are unregistered before deletion.
     * References to the deleted circuit and its owned objects become invalid.
     */
    void remove_circuit(Circuit & circuit);

    /**
     * @brief Returns the circuit blueprints owned by this project.
     * @return Borrowed const reference to the circuit-pointer vector.
     *
     * The vector cannot be modified through this reference, but its pointers
     * still refer to mutable Circuit objects.
     */
    const std::vector< Circuit * > & circuits() const { return circuits_; }

    /**
     * @brief Places a circuit blueprint inside another circuit.
     * @param parent Circuit that will own the placement.
     * @param circuit Blueprint referenced by the placement.
     * @return Borrowed reference to the new placement owned by parent.
     * @throws std::invalid_argument If either circuit is foreign or the
     * placement would create a direct or indirect recursive dependency.
     * @throws std::logic_error If parent has live runtime states.
     *
     * Creates placement ports matching the blueprint's interface and records
     * the placement so later interface edits can update it.
     */
    Subcircuit & create_subcircuit(Circuit & parent, Circuit & circuit);

    /**
     * @brief Creates a circuit interface pin and updates existing placements.
     * @param circuit Circuit that will own the pin.
     * @param type INPUT_PIN or OUTPUT_PIN.
     * @param width Positive signal width in bits; defaults to one.
     * @return Borrowed reference to the new pin.
     * @throws std::invalid_argument If circuit is foreign, type is invalid,
     * or width is not positive.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * INPUT_PIN creates an internal output port. OUTPUT_PIN creates an
     * internal input port.
     */
    Pin & create_pin(Circuit & circuit, PinType type, int width = 1);

    /**
     * @brief Deletes a component and performs type-specific bookkeeping.
     * @param circuit Circuit that owns the component.
     * @param component Component to delete.
     * @throws std::invalid_argument If circuit is foreign or local component
     * ownership validation fails.
     * @throws std::logic_error If affected states are alive or a placement
     * registry record is inconsistent.
     *
     * Dispatches pins and subcircuits to their specialized overloads even
     * when passed through a Component reference. Attached ports are detached;
     * independent wire nodes and edges remain.
     */
    void remove_component(Circuit & circuit, Component & component);

    /**
     * @brief Deletes an interface pin and updates placements of its blueprint.
     * @param circuit Circuit that owns the pin.
     * @param pin Interface pin to delete.
     * @throws std::invalid_argument If circuit is foreign or pin is not owned
     * by circuit.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * Removes corresponding placement ports and their connections.
     */
    void remove_component(Circuit & circuit, Pin & pin);

    /**
     * @brief Deletes a placement while preserving its referenced blueprint.
     * @param circuit Circuit that owns the placement.
     * @param subcircuit Placement to delete.
     * @throws std::invalid_argument If circuit is foreign.
     * @throws std::logic_error If circuit has live runtime states or the
     * placement registry record is missing or belongs to another parent.
     * @throws std::out_of_range If the referenced blueprint is not registered
     * in this project.
     *
     * Detaches placement ports and removes its borrowing registry entry.
     */
    void remove_component(Circuit & circuit, Subcircuit & subcircuit);

    /**
     * @brief Changes a pin's width and synchronizes its blueprint's placements.
     * @param circuit Circuit that owns the pin.
     * @param pin Interface pin to resize.
     * @param width New positive signal width in bits.
     * @throws std::invalid_argument If circuit is foreign, pin is not owned
     * by circuit, or width is not positive.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * Detaches incompatible connections at the pin and corresponding placement
     * ports. Other ports and node constraints are not resized automatically.
     */
    void set_pin_width(Circuit & circuit, Pin & pin, int width);

    /**
     * @brief Changes a pin's interface role and synchronizes placements.
     * @param circuit Circuit that owns the pin.
     * @param pin Interface pin to change.
     * @param type New role: INPUT_PIN or OUTPUT_PIN.
     * @throws std::invalid_argument If circuit is foreign, pin is not owned
     * by circuit, or type is invalid.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * A changed role replaces the pin's internal port, removes its old
     * connection, and appends the pin to its new direction's interface list.
     */
    void set_pin_type(Circuit & circuit, Pin & pin, PinType type);

    /**
     * @brief Changes a pin's label and updates corresponding placement labels.
     * @param circuit Circuit that owns the pin.
     * @param pin Interface pin to relabel.
     * @param label New descriptive text.
     * @throws std::invalid_argument If circuit is foreign or pin is not owned
     * by circuit.
     *
     * Matching labels do not establish electrical connections. Pure label
     * changes preserve runtime storage and existing connections.
     */
    void set_pin_label(Circuit & circuit, Pin & pin, const std::string & label);

    /**
     * @brief Reorders a pin within its input or output interface list.
     * @param circuit Circuit that owns the pin.
     * @param pin Interface pin to move.
     * @param index New zero-based position within the pin's current direction.
     * @throws std::invalid_argument If circuit is foreign or pin is not owned
     * by circuit.
     * @throws std::out_of_range If index is outside that interface list.
     * @throws std::logic_error If pin is missing from its interface list.
     *
     * Synchronizes placement ordering while preserving surviving port objects,
     * their connections, and runtime storage.
     */
    void move_pin(Circuit & circuit, Pin & pin, int index);

    /**
     * @brief Creates a disconnected single-bit AND gate.
     * @param circuit Circuit that will own the gate.
     * @param input_count Positive number of inputs; defaults to two.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is
     * not positive.
     * @throws std::logic_error If circuit has live runtime states.
     */
    AndGate & create_and_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected single-bit OR gate.
     * @param circuit Circuit that will own the gate.
     * @param input_count Positive number of inputs; defaults to two.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is
     * not positive.
     * @throws std::logic_error If circuit has live runtime states.
     */
    OrGate & create_or_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected single-bit NOT gate with one input.
     * @param circuit Circuit that will own the gate.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign.
     * @throws std::logic_error If circuit has live runtime states.
     */
    NotGate & create_not_gate(Circuit & circuit);

    /**
     * @brief Creates a disconnected single-bit NAND gate.
     * @param circuit Circuit that will own the gate.
     * @param input_count Positive number of inputs; defaults to two.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is
     * not positive.
     * @throws std::logic_error If circuit has live runtime states.
     */
    NandGate & create_nand_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected single-bit NOR gate.
     * @param circuit Circuit that will own the gate.
     * @param input_count Positive number of inputs; defaults to two.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is
     * not positive.
     * @throws std::logic_error If circuit has live runtime states.
     */
    NorGate & create_nor_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected single-bit odd-parity XOR gate.
     * @param circuit Circuit that will own the gate.
     * @param input_count Positive number of inputs; defaults to two.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is
     * not positive.
     * @throws std::logic_error If circuit has live runtime states.
     */
    XorGate & create_xor_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates a disconnected single-bit even-parity XNOR gate.
     * @param circuit Circuit that will own the gate.
     * @param input_count Positive number of inputs; defaults to two.
     * @return Borrowed reference to the new gate.
     * @throws std::invalid_argument If circuit is foreign or input_count is
     * not positive.
     * @throws std::logic_error If circuit has live runtime states.
     */
    XnorGate & create_xnor_gate(Circuit & circuit, int input_count = 2);

    /**
     * @brief Creates an unattached junction or endpoint in the wire graph.
     * @param circuit Circuit that will own the node.
     * @param width Width constraint in bits; zero means unconstrained.
     * @param label Descriptive text; matching labels do not connect nodes.
     * @return Borrowed reference to the new node.
     * @throws std::invalid_argument If circuit is foreign or width is negative.
     * @throws std::logic_error If circuit has live runtime states.
     * @throws std::overflow_error If node or derived-net IDs are exhausted.
     *
     * Rebuilds derived nets, invalidating their previous pointers and references
     * on success. A region without attached ports receives no Net.
     */
    WireNode & create_wire_node(Circuit & circuit, int width = 0, const std::string & label = "");

    /**
     * @brief Creates an undirected wire segment between two nodes.
     * @param circuit Circuit that owns both endpoints and will own the edge.
     * @param first First endpoint.
     * @param second Second endpoint.
     * @return Borrowed reference to the new edge.
     * @throws std::invalid_argument If circuit or either endpoint is foreign,
     * the endpoints are identical, the edge already exists, widths conflict,
     * or the resulting connection has multiple output drivers.
     * @throws std::logic_error If circuit has live runtime states.
     * @throws std::overflow_error If edge or derived-net IDs are exhausted.
     *
     * Endpoint order does not imply signal direction. Successful creation
     * rebuilds the circuit's derived nets.
     */
    WireEdge & create_wire_edge(Circuit & circuit, WireNode & first, WireNode & second);

    /**
     * @brief Deletes a wire segment while preserving its endpoint nodes.
     * @param circuit Circuit that owns the edge.
     * @param edge Segment to delete.
     * @throws std::invalid_argument If circuit is foreign or edge is not owned
     * by circuit.
     * @throws std::logic_error If circuit has live runtime states.
     *
     * Rebuilds derived nets. Other paths may keep the endpoints electrically
     * connected after this edge is removed.
     */
    void remove_wire_edge(Circuit & circuit, WireEdge & edge);

    /**
     * @brief Deletes a node, its incident edges, and its port attachments.
     * @param circuit Circuit that owns the node.
     * @param node Node to delete.
     * @throws std::invalid_argument If circuit is foreign or node is not owned
     * by circuit.
     * @throws std::logic_error If circuit has live runtime states.
     *
     * Attached components and their ports survive in a detached state.
     * Remaining neighboring nodes are not automatically joined together.
     */
    void remove_wire_node(Circuit & circuit, WireNode & node);

    /**
     * @brief Attaches a component port to a wire node.
     * @param circuit Circuit that owns the port and node.
     * @param port Previously unattached port.
     * @param node Node that receives the attachment.
     * @throws std::invalid_argument If ownership is invalid, port is already
     * attached, widths conflict, or the resulting net has multiple drivers.
     * @throws std::logic_error If circuit has live runtime states.
     *
     * Creates no nodes or edges. Repeated attachment to the same node is also
     * rejected. A successful connection rebuilds derived nets.
     */
    void connect(Circuit & circuit, Port & port, WireNode & node);

    /**
     * @brief Detaches a port while preserving its component and wire geometry.
     * @param circuit Circuit that owns the port.
     * @param port Port to detach.
     * @throws std::invalid_argument If circuit is foreign or port is not owned
     * by circuit.
     * @throws std::logic_error If circuit has live runtime states.
     *
     * An already detached port requires no connectivity change. Ownership and
     * editability are still checked before returning.
     */
    void disconnect(Circuit & circuit, Port & port);

    /**
     * @brief Deletes a circuit's contents while retaining its blueprint object.
     * @param circuit Circuit owned by this project.
     * @throws std::invalid_argument If circuit is foreign.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * Deletes local components and wire geometry, unregisters contained
     * placements, and updates external placements to the empty interface.
     * Other blueprints remain owned by the project.
     */
    void clear_circuit(Circuit & circuit);

private:
    friend class ProjectEdit;
    friend class ProjectController;
    /**
     * @brief Constructs a supported component copy and copies its metadata.
     * @param destination Circuit that will own the disconnected copy.
     * @param source Component whose exact type and configuration are copied.
     * @return Borrowed reference to the new component.
     * @pre Destination and source belong to this project.
     * @throws std::invalid_argument If the exact type is unsupported or a
     * copied placement would introduce recursion.
     * @throws std::logic_error If affected circuits have live runtime states.
     *
     * Synchronizes copied pins and removes partially created objects if
     * metadata copying fails.
     */
    Component & copy_component_impl(Circuit & destination, const Component & source);

    /**
     * @brief Records a borrowed placement and the circuit containing it.
     */
    struct Placement
    {
        /**
         * @brief Stores the containing circuit and placement addresses.
         * @param parent Circuit containing the placement.
         * @param subcircuit Placement owned by parent.
         */
        Placement(Circuit & parent, Subcircuit & subcircuit)
            : parent(&parent), subcircuit(&subcircuit)
        {}

        Circuit * parent; ///< Borrowed circuit containing the placement.
        Subcircuit * subcircuit; ///< Borrowed placement referencing a blueprint.
    };

    /**
     * @brief Updates every placement referencing a circuit blueprint.
     * @param circuit Blueprint whose current interface should be reflected.
     * @pre Circuit is registered in this project.
     *
     * Synchronizes placement port membership, widths, labels, and ordering.
     * Earlier updates may remain applied if a later synchronization throws.
     */
    void sync_interfaces(const Circuit & circuit);

    /**
     * @brief Checks that a circuit belongs to this project.
     * @param circuit Circuit to validate.
     * @throws std::invalid_argument If circuit is not registered here.
     */
    void require_circuit(const Circuit & circuit) const;

    /**
     * @brief Checks that an interface change can safely affect its placements.
     * @param circuit Blueprint whose interface may change.
     * @pre Circuit is registered in this project.
     * @throws std::logic_error If circuit or a circuit containing one of its
     * placements has live runtime states.
     */
    void require_interfaces_editable(const Circuit & circuit) const;

    std::vector< Circuit * > circuits_; ///< Owned circuit blueprints.

    /**
     * @brief Tracks placements by the blueprint they reference.
     *
     * Each record borrows its parent circuit and placement component.
     * A blueprint with no placements still has an empty registry entry.
     */
    std::unordered_map< const Circuit *, std::vector< Placement > > placements_;

    ProjectEdit * edit_; ///< Borrowed active structural transaction, if any.
};

#endif // PROJECT_H
