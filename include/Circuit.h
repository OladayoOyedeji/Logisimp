
/**
 * @file Circuit.h
 * @brief Owns the components, wire graph, and nets that make up a circuit.
 * @author Cole Schwandt
 */
// TODO: add labels to circuits

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "Component.h"
#include "Pin.h"
#include "Port.h"
#include "WireNode.h"
#include "WireEdge.h"
#include "Net.h"

#include <string>
#include <vector>

class AndGate;
class NandGate;
class NorGate;
class NotGate;
class OrGate;
class Project;
class ProjectEdit;
class CircuitState;
class Subcircuit;
class XnorGate;
class XorGate;

/**
 * @brief Represents the structural blueprint of a circuit.
 *
 * Owns components, wire nodes, wire edges, and derived nets.
 * Components own their ports, while the circuit's pin lists
 * provide non-owning references to its interface components.
 *
 * Runtime signal values are stored separately in CircuitState.
 * Most structural edits are prohibited while runtime states exist.
 * ProjectController coordinates live edits by preparing replacement runtime
 * storage before releasing the old structural objects.
 *
 * Circuit objects are owned and managed by Project.
 */
class Circuit
{
public:
    /** @brief Circuits cannot be copied. */
    Circuit(const Circuit & other) = delete;

    /** @brief Circuits cannot be assigned. */
    Circuit & operator=(const Circuit & other) = delete;

    /** @brief Circuits cannot be moved. */
    Circuit(Circuit && other) = delete;

    /** @brief Circuits cannot be move-assigned. */
    Circuit & operator=(Circuit && other) = delete;

    /**
     * @brief Returns the number of components in the circuit.
     * @return Number of components, including gates, pins, and subcircuits.
     */
    int comp_size() const { return int(components_.size()); }

    /**
     * @brief Returns a component at the specified index.
     * @param index Index in the component list.
     * @return Borrowed reference to the component.
     * @throws std::out_of_range If the index is invalid.
     */
    Component & component_at(int index) { return *components_.at(index); }
    const Component & component_at(int index) const { return *components_.at(index); }

    /**
     * @brief Returns the circuit's components.
     * @return Non-owning pointers to all components.
     */
    const std::vector< Component * > & components() const { return components_; }

    /**
     * @brief Returns the circuit's wire nodes.
     * @return Non-owning pointers to all wire nodes.
     */
    const std::vector< WireNode * > & wire_nodes() const { return wire_nodes_; }

    /**
     * @brief Returns the circuit's wire edges.
     * @return Non-owning pointers to all wire edges.
     */
    const std::vector< WireEdge * > & wire_edges() const { return wire_edges_; }

    /**
     * @brief Returns the currently derived nets.
     * @return Non-owning pointers to all nets.
     * @note Net objects are replaced after a successful connectivity rebuild.
     */
    const std::vector< Net * > & nets() const { return nets_; }

    /**
     * @brief Returns the circuit's input interface pins.
     * @return Non-owning pointers in interface order.
     */
    const std::vector< Pin * > & input_pins() const { return input_pins_; }

    /**
     * @brief Returns the circuit's output interface pins.
     * @return Non-owning pointers in interface order.
     */
    const std::vector< Pin * > & output_pins() const { return output_pins_; }

    /**
     * @brief Returns an input pin at the specified interface index.
     * @param index Index in the input pin list.
     * @return Borrowed reference to the input pin.
     * @throws std::out_of_range If the index is invalid.
     */
    Pin & input_pin(int index) { return *input_pins_.at(index); }
    const Pin & input_pin(int index) const { return *input_pins_.at(index); }

    /**
     * @brief Returns an output pin at the specified interface index.
     * @param index Index in the output pin list.
     * @return Borrowed reference to the output pin.
     * @throws std::out_of_range If the index is invalid.
     */
    Pin & output_pin(int index) { return *output_pins_.at(index); }
    const Pin & output_pin(int index) const { return *output_pins_.at(index); }

    /**
     * @brief Checks whether this circuit depends on another circuit.
     * @param circuit Circuit to search for in the subcircuit hierarchy.
     * @return True if the circuit is used directly or indirectly.
     */
    bool uses(const Circuit & circuit) const;

private:
    friend class Project;
    friend class ProjectEdit;
    friend class ProjectController;
    friend class CircuitState;

    /**
     * @brief Constructs an empty circuit with initialized ID counters.
     */
    Circuit()
        : next_component_id_(0), next_node_id_(0), next_edge_id_(0), next_net_id_(0), active_states_(0), edit_(nullptr)
    {}

    /**
     * @brief Destroys the circuit and its owned structural objects.
     */
    ~Circuit()
    {
        clear();
    }

    /**
     * @brief Creates an AND gate.
     * @param input_count Number of input ports.
     * @return Borrowed reference to the new gate.
     */
    AndGate & create_and_gate(int input_count = 2);

    /**
     * @brief Creates an OR gate.
     * @param input_count Number of input ports.
     * @return Borrowed reference to the new gate.
     */
    OrGate & create_or_gate(int input_count = 2);

    /**
     * @brief Creates a NOT gate.
     * @return Borrowed reference to the new gate.
     */
    NotGate & create_not_gate();

    /**
     * @brief Creates a NAND gate.
     * @param input_count Number of input ports.
     * @return Borrowed reference to the new gate.
     */
    NandGate & create_nand_gate(int input_count = 2);

    /**
     * @brief Creates a NOR gate.
     * @param input_count Number of input ports.
     * @return Borrowed reference to the new gate.
     */
    NorGate & create_nor_gate(int input_count = 2);

    /**
     * @brief Creates an XOR gate.
     * @param input_count Number of input ports.
     * @return Borrowed reference to the new gate.
     */
    XorGate & create_xor_gate(int input_count = 2);

    /**
     * @brief Creates an XNOR gate.
     * @param input_count Number of input ports.
     * @return Borrowed reference to the new gate.
     */
    XnorGate & create_xnor_gate(int input_count = 2);

    /**
     * @brief Creates a circuit interface pin.
     * @param type INPUT_PIN or OUTPUT_PIN.
     * @param width Positive signal width.
     * @return Borrowed reference to the new pin.
     */
    Pin & create_pin(PinType type, int width = 1);

    /**
     * @brief Creates a placement of another circuit.
     * @param circuit Blueprint referenced by the new subcircuit.
     * @return Borrowed reference to the new placement.
     * @throws std::invalid_argument If the placement would create a recursive dependency.
     */
    Subcircuit & create_subcircuit(Circuit & circuit);

    /**
     * @brief Creates an unattached wire node.
     * @param width Width constraint; zero means unspecified.
     * @param label Descriptive text; matching labels do not connect nodes.
     * @return Borrowed reference to the new node.
     */
    WireNode & create_wire_node(int width = 0, const std::string & label = "");

    /**
     * @brief Creates an undirected edge between two wire nodes.
     * @param first First endpoint.
     * @param second Second endpoint.
     * @return Borrowed reference to the new edge.
     * @throws std::invalid_argument If endpoints are invalid, identical, or already directly connected.
     */
    WireEdge & create_wire_edge(WireNode & first, WireNode & second);

    /**
     * @brief Changes an interface pin's signal width.
     * @param pin Pin to modify.
     * @param width New positive signal width.
     * @note Disconnects the pin if its current net has an incompatible width.
     */
    void set_pin_width(Pin & pin, int width);

    /**
     * @brief Changes an interface pin's direction.
     * @param pin Pin to modify.
     * @param type New pin type.
     * @note Replaces the pin's port and disconnects its previous connection.
     */
    void set_pin_type(Pin & pin, PinType type);

    /**
     * @brief Changes an interface pin's label.
     * @param pin Pin to modify.
     * @param label New descriptive label.
     */
    void set_pin_label(Pin & pin, const std::string & label);

    /**
     * @brief Moves a pin within its input or output interface list.
     * @param pin Pin to move.
     * @param index New zero-based interface index.
     * @throws std::out_of_range If the index is invalid.
     * @note Preserves the pin object and its connections.
     */
    void move_pin(Pin & pin, int index);

    /**
     * @brief Synchronizes a subcircuit's ports with its blueprint.
     * @param subcircuit Placement to synchronize.
     * @note Removes obsolete ports and updates port order, widths, and labels.
     */
    void sync_subcircuit(Subcircuit & subcircuit);

    /**
     * @brief Removes placement ports absent from the blueprint interface.
     * @param subcircuit Placement being updated.
     * @param interface_pins Current blueprint pins for the direction.
     * @param direction Port direction to examine.
     */
    void remove_obsolete_subcircuit_ports(Subcircuit & subcircuit, const std::vector< Pin * > & interface_pins, PortDirection direction);

    /**
     * @brief Matches placement ports to the blueprint interface by pin ID.
     * @param subcircuit Placement being updated.
     * @param interface_pins Current blueprint pins in interface order.
     * @param direction Port direction to synchronize.
     * @note Preserves compatible ports and their existing connections.
     */
    void sync_subcircuit_ports(Subcircuit & subcircuit, const std::vector< Pin * > & interface_pins, PortDirection direction);

    /**
     * @brief Attaches a component port to a wire node.
     * @param port Port to attach.
     * @param node Target wire node.
     * @throws std::invalid_argument If either object is foreign or the port is already attached.
     * @note Rebuilds nets and rejects incompatible widths or multiple drivers.
     */
    void connect(Port & port, WireNode & node);

    /**
     * @brief Detaches a component port from its wire node.
     * @param port Port to detach.
     * @note Rebuilds nets if the port was connected.
     */
    void disconnect(Port & port);

    /**
     * @brief Removes and destroys a component.
     * @param component Component to remove.
     * @note Detaches its ports and removes its interface entry if it is a pin.
     */
    void remove_component(Component & component);

    /**
     * @brief Removes and destroys a circuit interface pin.
     * @param pin Pin to remove.
     */
    void remove_component(Pin & pin);

    /**
     * @brief Removes and destroys a wire edge.
     * @param edge Edge to remove.
     * @note Preserves both endpoint nodes and rebuilds connectivity.
     */
    void remove_wire_edge(WireEdge & edge);

    /**
     * @brief Removes and destroys a wire node.
     * @param node Node to remove.
     * @note Removes incident edges and detaches attached ports.
     */
    void remove_wire_node(WireNode & node);

    /**
     * @brief Destroys all owned components, graph objects, and nets.
     * @note Clears interface lists but does not reset ID counters.
     */
    void clear();

    /**
     * @brief Registers a newly allocated component with the circuit.
     * @param component Component whose ownership is transferred to the circuit.
     * @note Assigns the next component ID and destroys the object if registration fails.
     */
    void store_component(Component * component);

    /**
     * @brief Removes a pin from its input or output interface list.
     * @param pin Pin whose interface reference is removed.
     * @note Does not destroy the pin.
     */
    void remove_pin_reference(Pin & pin);

    /**
     * @brief Checks whether this circuit owns a component.
     * @param component Component to examine.
     * @return True if the component is in the owned component list.
     */
    bool owns_component(const Component & component) const;

    /**
     * @brief Checks whether this circuit owns a wire node.
     * @param node Node to examine.
     * @return True if the node is in the owned node list.
     */
    bool owns_wire_node(const WireNode & node) const;

    /**
     * @brief Checks whether this circuit owns a wire edge.
     * @param edge Edge to examine.
     * @return True if the edge is in the owned edge list.
     */
    bool owns_wire_edge(const WireEdge & edge) const;

    /**
     * @brief Checks whether a port belongs to an owned component.
     * @param port Port to examine.
     * @return True if the port is registered with an owned component.
     */
    bool owns_port(const Port & port) const;

    /**
     * @brief Ensures live runtime states are protected during structural editing.
     * @throws std::logic_error If active states exist outside a coordinated edit.
     * @note ProjectController grants temporary edit access after validating
     * that it manages every affected runtime instance.
     */
    void require_editable() const;

    /**
     * @brief Checks whether an active edit keeps an original object alive.
     * @param object Borrowed address to compare without dereferencing it.
     * @return True if the active ProjectEdit retains this pre-edit object.
     * @note Keeps pre-edit objects alive until runtime storage has been updated.
     */
    bool preserves(const void * object) const noexcept;

    /**
     * @brief Deletes an obsolete object unless a structural edit retains it.
     * @tparam T Type of structural object being retired.
     * @param object Pointer to delete or leave alive for ProjectEdit to resolve.
     * @note Retained original objects are released at commit or restored on rollback.
     * Objects created during the edit are not retained by its original snapshot.
     */
    template< typename T > void discard(T * object)
    {
        if (!preserves(object))
        {
            delete object;
        }
    }

    /**
     * @brief Reconstructs nets from connected wire nodes and attached ports.
     *
     * Each connected region containing ports becomes a net.
     * Validates signal widths and permits at most one OUTPUT driver.
     * Regions without ports do not produce nets.
     *
     * @note On success, replaces all existing Net objects and updates
     * node and port net references. Existing Net pointers become invalid.
     */
    void rebuild_nets();

    /**
     * @brief Detaches a collection of ports from their wire nodes.
     * @param ports Ports to detach.
     * @note Rebuilds nets once and restores attachments if rebuilding fails.
     */
    void detach_ports(const std::vector< Port * > & ports);

    /// Components owned by this circuit.
    std::vector< Component * > components_;

    /// Wire nodes owned by this circuit.
    std::vector< WireNode * > wire_nodes_;

    /// Wire edges owned by this circuit.
    std::vector< WireEdge * > wire_edges_;

    /// Derived nets owned by this circuit and replaced during rebuilding.
    std::vector< Net * > nets_;

    /// Non-owning input interface references; pins are owned through components_.
    std::vector< Pin * > input_pins_;

    /// Non-owning output interface references; pins are owned through components_.
    std::vector< Pin * > output_pins_;

    /// Next component ID; removed IDs are not reused.
    int next_component_id_;

    /// Next wire node ID.
    int next_node_id_;

    /// Next wire edge ID.
    int next_edge_id_;

    /// Next derived net ID.
    int next_net_id_;

    /// Number of active CircuitState objects using this blueprint.
    mutable int active_states_;

    ProjectEdit * edit_; ///< Borrowed transaction authorizing coordinated edits.
};

#endif // CIRCUIT_H
