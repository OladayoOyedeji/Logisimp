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

class Project
{
public:
    Project()
    {}

    Project(const Project & other) = delete;
    Project & operator=(const Project & other) = delete;
    Project(Project && other) = delete;
    Project & operator=(Project && other) = delete;

    ~Project();

    Circuit & create_circuit();

    Circuit & copy_circuit(const Circuit & source);

    Component & copy_component(Circuit & destination, const Component & source);

    WireNode & copy_wire_node(Circuit & destination, const WireNode & source);

    void remove_circuit(Circuit & circuit);
    const std::vector< Circuit * > & circuits() const { return circuits_; }

    Subcircuit & create_subcircuit(Circuit & parent, Circuit & circuit);
   
    Pin & create_pin(Circuit & circuit, PinType type, int width = 1);

    void remove_component(Circuit & circuit, Component & component);
    void remove_component(Circuit & circuit, Pin & pin);
    void remove_component(Circuit & circuit, Subcircuit & subcircuit);

    void set_pin_width(Circuit & circuit, Pin & pin, int width);
    void set_pin_type(Circuit & circuit, Pin & pin, PinType type);
    void set_pin_label(Circuit & circuit, Pin & pin, const std::string & label);
    void move_pin(Circuit & circuit, Pin & pin, int index);

    AndGate & create_and_gate(Circuit & circuit, int input_count = 2);
    OrGate & create_or_gate(Circuit & circuit, int input_count = 2);
    NotGate & create_not_gate(Circuit & circuit);
    NandGate & create_nand_gate(Circuit & circuit, int input_count = 2);
    NorGate & create_nor_gate(Circuit & circuit, int input_count = 2);
    XorGate & create_xor_gate(Circuit & circuit, int input_count = 2);
    XnorGate & create_xnor_gate(Circuit & circuit, int input_count = 2);

    WireNode & create_wire_node(Circuit & circuit, int width = 0, const std::string & label = "");

    WireEdge & create_wire_edge(Circuit & circuit, WireNode & first, WireNode & second);

    void remove_wire_edge(Circuit & circuit, WireEdge & edge);

    void remove_wire_node(Circuit & circuit, WireNode & node);

    void connect(Circuit & circuit, Port & port, WireNode & node);

    void disconnect(Circuit & circuit, Port & port);

    void clear_circuit(Circuit & circuit);

private:
    Component & copy_component_impl(Circuit & destination, const Component & source);

    struct Placement
    {
        Placement(Circuit & parent, Subcircuit & subcircuit)
            : parent(&parent), subcircuit(&subcircuit)
        {}

        Circuit * parent;
        Subcircuit * subcircuit;
    };

    void sync_interfaces(const Circuit & circuit);
    void require_circuit(const Circuit & circuit) const;

    void require_interfaces_editable(const Circuit & circuit) const;

    std::vector< Circuit * > circuits_;

    std::unordered_map< const Circuit *, std::vector< Placement > > placements_;
};

#endif // PROJECT_H
