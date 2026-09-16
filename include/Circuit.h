// Circuit.h

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "Component.h"
#include "Gates.h"
#include "Pin.h"
#include "Port.h"
#include "Simulator.h"
#include "Wire.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

class Circuit
{
public:
    Circuit()
        : next_component_id_(0), next_wire_id_(0)
    {}

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

    Circuit(const Circuit & other) = delete;
    Circuit & operator=(const Circuit & other) = delete;

    std::vector< Component * > & components() { return components_; }
    std::vector< Component * > components() const { return components_; }

    std::vector< Wire * > & wires() { return wires_; }
    std::vector< Wire * > wires() const { return wires_; }

    // Creators
    //----------------------------------------------
    AndGate & create_and_gate(int input_count = 2)
    {
        AndGate * component = new AndGate(next_component_id_, input_count);

        store_component(component);

        return *component;
    }

    OrGate & create_or_gate(int input_count = 2)
    {
        OrGate * component = new OrGate(next_component_id_, input_count);

        store_component(component);

        return *component;
    }

    NotGate & create_not_gate()
    {
        NotGate * component = new NotGate(next_component_id_);

        store_component(component);

        return *component;
    }

    NandGate & create_nand_gate(int input_count = 2)
    {
        NandGate * component = new NandGate(next_component_id_, input_count);

        store_component(component);

        return *component;
    }

    NorGate & create_nor_gate(int input_count = 2)
    {
        NorGate * component = new NorGate(next_component_id_, input_count);

        store_component(component);

        return *component;
    }

    XorGate & create_xor_gate(int input_count = 2)
    {
        XorGate * component = new XorGate(next_component_id_, input_count);

        store_component(component);

        return *component;
    }

    XnorGate & create_xnor_gate(int input_count = 2)
    {
        XnorGate * component = new XnorGate(next_component_id_, input_count);

        store_component(component);

        return *component;
    }

    Pin & create_pin(PinType type, int width = 1)
    {
        Pin * component = new Pin(next_component_id_, type, width);

        store_component(component);

        return *component;
    }

    Wire & create_wire(int width = 1, const std::string & label = "")
    {
        Wire * wire = new Wire(next_wire_id_, width, label);

        wires_.push_back(wire);
        next_wire_id_++;

        return *wire;
    }

    void connect(Port & port, Wire & wire)
    {
        if (port.owner() == nullptr)
        {
            throw std::runtime_error("Port does not have an owner");
        }

        if (!owns_component(*port.owner()))
        {
            throw std::runtime_error("Port owner does not belong to this circuit");
        }

        if (!owns_wire(wire))
        {
            throw std::runtime_error("Wire does not belong to this circuit");
        }

        if (port.wire() != nullptr)
        {
            throw std::runtime_error("Port is already connected");
        }

        if (port.width() != wire.width())
        {
            throw std::runtime_error("Port and wire widths do not match");
        }

        if (port.direction() == OUTPUT)
        {
            if (wire.driver() != nullptr)
            {
                throw std::runtime_error("Wire already has a driver");
            }

            wire.driver() = &port;
        }
        else
        {
            wire.listeners().push_back(&port);
        }

        port.wire() = &wire;
    }

    // removes references to the port
    // (used before the port is deleted when deleting a component)
    void disconnect(Port & port)
    {
        if (port.wire() == nullptr)
        {
            return;
        }

        Wire * wire = port.wire();

        if (port.direction() == OUTPUT)
        {
            if (wire->driver() == &port)
            {
                wire->driver() = nullptr;
            }
        }
        else
        {
            std::vector< Port * >::iterator position = std::find(wire->listeners().begin(), wire->listeners().end(), &port);

            if (position != wire->listeners().end())
            {
                wire->listeners().erase(position);
            }
        }

        port.wire() = nullptr;
    }

    void remove_component(Component & component)
    {
        std::vector< Component * >::iterator position = std::find(components_.begin(), components_.end(), &component);

        if (position == components_.end())
        {
            throw std::runtime_error("Component does not belong to this circuit");
        }

        for (Port * port : component.inputs())
        {
            disconnect(*port);
        }

        for (Port * port : component.outputs())
        {
            disconnect(*port);
        }

        components_.erase(position);

        delete &component;
    }

    void remove_wire(Wire & wire)
    {
        std::vector< Wire * >::iterator position = std::find(wires_.begin(), wires_.end(), &wire);

        if (position == wires_.end())
        {
            throw std::runtime_error("Wire does not belong to this circuit");
        }

        if (wire.driver() != nullptr)
        {
            disconnect(*wire.driver());
        }

        while (!wire.listeners().empty())
        {
            disconnect(*wire.listeners().back());
        }

        wires_.erase(position);

        delete &wire;
    }

    bool initialize(Simulator & simulator)
    {
        simulator.clear();

        for (Component * component : components_)
        {
            simulator.enqueue(component);
        }

        return simulator.run();
    }

    bool reset(Simulator & simulator)
    {
        simulator.clear();

        for (Wire * wire : wires_)
        {
            wire->value() = LogicVector(wire->width(), UNKNOWN);
        }

        for (Component * component : components_)
        {
            component->reset();
            simulator.enqueue(component);
        }

        return simulator.run();
    }

private:
    void store_component(Component * component)
    {
        components_.push_back(component);
        next_component_id_++;
    }

    bool owns_component(const Component & component) const
    {
        return std::find(components_.begin(), components_.end(), &component) != components_.end();
    }

    bool owns_wire(const Wire & wire) const
    {
        return std::find(wires_.begin(), wires_.end(), &wire) != wires_.end();
    }

    std::vector< Component * > components_;
    std::vector< Wire * > wires_;

    int next_component_id_;
    int next_wire_id_;
};

#endif // CIRCUIT_H
