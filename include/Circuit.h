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
class CircuitState;
class Subcircuit;
class XnorGate;
class XorGate;

class Circuit
{
public:
    // Don't allow copying or moving.
    Circuit(const Circuit & other) = delete;
    Circuit & operator=(const Circuit & other) = delete;
    Circuit(Circuit && other) = delete;
    Circuit & operator=(Circuit && other) = delete;

    int comp_size() const { return int(components_.size()); }

    Component & component_at(int index) { return *components_.at(index); }
    const Component & component_at(int index) const { return *components_.at(index); }

    const std::vector< Component * > & components() const { return components_; }
    const std::vector< WireNode * > & wire_nodes() const { return wire_nodes_; }
    const std::vector< WireEdge * > & wire_edges() const { return wire_edges_; }
    const std::vector< Net * > & nets() const { return nets_; }

    const std::vector< Pin * > & input_pins() const { return input_pins_; }
    const std::vector< Pin * > & output_pins() const { return output_pins_; }

    Pin & input_pin(int index) { return *input_pins_.at(index); }
    const Pin & input_pin(int index) const { return *input_pins_.at(index); }

    Pin & output_pin(int index) { return *output_pins_.at(index); }
    const Pin & output_pin(int index) const { return *output_pins_.at(index); }

    bool uses(const Circuit & circuit) const;

private:
    friend class Project;
    friend class CircuitState;

    Circuit()
        : next_component_id_(0), next_node_id_(0), next_edge_id_(0), next_net_id_(0), active_states_(0)
    {}

    ~Circuit()
    {
        clear();
    }

    AndGate & create_and_gate(int input_count = 2);
    OrGate & create_or_gate(int input_count = 2);
    NotGate & create_not_gate();
    NandGate & create_nand_gate(int input_count = 2);
    NorGate & create_nor_gate(int input_count = 2);
    XorGate & create_xor_gate(int input_count = 2);
    XnorGate & create_xnor_gate(int input_count = 2);

    Pin & create_pin(PinType type, int width = 1);
    Subcircuit & create_subcircuit(Circuit & circuit);
    WireNode & create_wire_node(int width = 0, const std::string & label = "");
    WireEdge & create_wire_edge(WireNode & first, WireNode & second);

    void set_pin_width(Pin & pin, int width);
    void set_pin_type(Pin & pin, PinType type);
    void set_pin_label(Pin & pin, const std::string & label);
    void move_pin(Pin & pin, int index);

    void sync_subcircuit(Subcircuit & subcircuit);
    void remove_obsolete_subcircuit_ports(Subcircuit & subcircuit, const std::vector< Pin * > & interface_pins, PortDirection direction);
    void sync_subcircuit_ports(Subcircuit & subcircuit, const std::vector< Pin * > & interface_pins, PortDirection direction);

    void connect(Port & port, WireNode & node);
    void disconnect(Port & port);

    void remove_component(Component & component);
    void remove_component(Pin & pin);
    void remove_wire_edge(WireEdge & edge);
    void remove_wire_node(WireNode & node);
    void clear();

    void store_component(Component * component);
    void remove_pin_reference(Pin & pin);

    bool owns_component(const Component & component) const;
    bool owns_wire_node(const WireNode & node) const;
    bool owns_wire_edge(const WireEdge & edge) const;
    bool owns_port(const Port & port) const;
    void require_editable() const;
    void rebuild_nets();
    void detach_ports(const std::vector< Port * > & ports);

    std::vector< Component * > components_;
    std::vector< WireNode * > wire_nodes_;
    std::vector< WireEdge * > wire_edges_;
    std::vector< Net * > nets_;

    // these do not own the pins;
    // the pins are owned through components_
    std::vector< Pin * > input_pins_;
    std::vector< Pin * > output_pins_;

    int next_component_id_;
    int next_node_id_;
    int next_edge_id_;
    int next_net_id_;
    mutable int active_states_;
};

#endif // CIRCUIT_H
