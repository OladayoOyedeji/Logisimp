// Circuit.h
/**
 * Owns the components, wires, and connections that make up a circuit.
 */

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "Component.h"
#include "Pin.h"
#include "Wire.h"

#include <string>
#include <vector>

class AndGate;
class NandGate;
class NorGate;
class NotGate;
class OrGate;
class Port;
class Simulator;
class XnorGate;
class XorGate;

class Circuit
{
public:
    Circuit()
        : next_component_id_(0), next_wire_id_(0)
    {}

    // Don't allow copying or moving.
    Circuit(const Circuit & other) = delete;
    Circuit & operator=(const Circuit & other) = delete;
    Circuit(Circuit && other) = delete;
    Circuit & operator=(Circuit && other) = delete;

    ~Circuit()
    {
        for (Component * component : components_)
        {
            delete component;
        }

        for (Wire * wire : wires_)
        {
            delete wire;
        }
    }

    int comp_size() const { return int(components_.size()); }

    Component & component_at(int index) { return *components_.at(index); }
    const Component & component_at(int index) const { return *components_.at(index); }

    const std::vector< Component * > & components() const { return components_; }
    const std::vector< Wire * > & wires() const { return wires_; }

    const std::vector< Pin * > & input_pins() const { return input_pins_; }
    const std::vector< Pin * > & output_pins() const { return output_pins_; }

    Pin & input_pin(int index) { return *input_pins_.at(index); }
    const Pin & input_pin(int index) const { return *input_pins_.at(index); }

    Pin & output_pin(int index) { return *output_pins_.at(index); }
    const Pin & output_pin(int index) const { return *output_pins_.at(index); }

    AndGate & create_and_gate(int input_count = 2);
    OrGate & create_or_gate(int input_count = 2);
    NotGate & create_not_gate();
    NandGate & create_nand_gate(int input_count = 2);
    NorGate & create_nor_gate(int input_count = 2);
    XorGate & create_xor_gate(int input_count = 2);
    XnorGate & create_xnor_gate(int input_count = 2);

    Pin & create_pin(PinType type, int width = 1);
    Wire & create_wire(int width = 1, const std::string & label = "");

    void connect(Port & port, Wire & wire);
    void disconnect(Port & port);

    void remove_component(Component & component);
    void remove_wire(Wire & wire);

    bool initialize(Simulator & simulator);
    bool reset(Simulator & simulator);

private:
    void store_component(Component * component);
    void remove_pin_reference(Pin & pin);

    bool owns_component(const Component & component) const;
    bool owns_wire(const Wire & wire) const;

    std::vector< Component * > components_;
    std::vector< Wire * > wires_;

    // These do not own the pins.
    // The pins are owned through components_.
    std::vector< Pin * > input_pins_;
    std::vector< Pin * > output_pins_;

    int next_component_id_;
    int next_wire_id_;
};

#endif // CIRCUIT_H
