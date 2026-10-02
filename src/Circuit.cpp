// Circuit.cpp

#include "Circuit.h"
#include "Component.h"
#include "Gates.h"
#include "LogicVector.h"
#include "Port.h"
#include "Simulator.h"
#include "Wire.h"

#include <algorithm>
#include <stdexcept>

AndGate & Circuit::create_and_gate(int input_count)
{
    AndGate * component = new AndGate(next_component_id_, input_count);

    store_component(component);

    return *component;
}

OrGate & Circuit::create_or_gate(int input_count)
{
    OrGate * component = new OrGate(next_component_id_, input_count);

    store_component(component);

    return *component;
}

NotGate & Circuit::create_not_gate()
{
    NotGate * component = new NotGate(next_component_id_);

    store_component(component);

    return *component;
}

NandGate & Circuit::create_nand_gate(int input_count)
{
    NandGate * component = new NandGate(next_component_id_, input_count);

    store_component(component);

    return *component;
}

NorGate & Circuit::create_nor_gate(int input_count)
{
    NorGate * component = new NorGate(next_component_id_, input_count);

    store_component(component);

    return *component;
}

XorGate & Circuit::create_xor_gate(int input_count)
{
    XorGate * component = new XorGate(next_component_id_, input_count);

    store_component(component);

    return *component;
}

XnorGate & Circuit::create_xnor_gate(int input_count)
{
    XnorGate * component = new XnorGate(next_component_id_, input_count);

    store_component(component);

    return *component;
}

Pin & Circuit::create_pin(PinType type, int width)
{
    Pin * pin = new Pin(next_component_id_, type, width);

    store_component(pin);

    if (type == INPUT_PIN)
    {
        input_pins_.push_back(pin);
    }
    else
    {
        output_pins_.push_back(pin);
    }

    return *pin;
}

Wire & Circuit::create_wire(int width, const std::string & label)
{
    Wire * wire = new Wire(next_wire_id_, width, label);

    wires_.push_back(wire);
    next_wire_id_++;

    return *wire;
}

void Circuit::connect(Port & port, Wire & wire)
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

void Circuit::disconnect(Port & port)
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
        std::vector< Port * >::iterator position =
            std::find(wire->listeners().begin(), wire->listeners().end(), &port);

        if (position != wire->listeners().end())
        {
            wire->listeners().erase(position);
        }
    }

    port.wire() = nullptr;
}

void Circuit::remove_component(Component & component)
{
    Component * component_pointer = &component;

    std::vector< Component * >::iterator position =
        std::find(components_.begin(), components_.end(), component_pointer);

    if (position == components_.end())
    {
        throw std::runtime_error("Component does not belong to this circuit");
    }

    Pin * pin = dynamic_cast< Pin * >(component_pointer);

    if (pin != nullptr)
    {
        remove_pin_reference(*pin);
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

    delete component_pointer;
}

void Circuit::remove_wire(Wire & wire)
{
    std::vector< Wire * >::iterator position =
        std::find(wires_.begin(), wires_.end(), &wire);

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

bool Circuit::initialize(Simulator & simulator)
{
    simulator.clear();

    for (Component * component : components_)
    {
        simulator.enqueue(component);
    }

    return simulator.run();
}

bool Circuit::reset(Simulator & simulator)
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

void Circuit::store_component(Component * component)
{
    components_.push_back(component);
    next_component_id_++;
}

void Circuit::remove_pin_reference(Pin & pin)
{
    if (pin.type() == INPUT_PIN)
    {
        std::vector< Pin * >::iterator position = std::find(input_pins_.begin(), input_pins_.end(), &pin);

        if (position == input_pins_.end())
        {
            throw std::logic_error("Input pin is missing from the circuit interface");
        }

        input_pins_.erase(position);
    }
    else
    {
        std::vector< Pin * >::iterator position = std::find(output_pins_.begin(), output_pins_.end(), &pin);

        if (position == output_pins_.end())
        {
            throw std::logic_error("Output pin is missing from the circuit interface");
        }

        output_pins_.erase(position);
    }
}

bool Circuit::owns_component(const Component & component) const
{
    return std::find(components_.begin(), components_.end(), &component) != components_.end();
}

bool Circuit::owns_wire(const Wire & wire) const
{
    return std::find(wires_.begin(), wires_.end(), &wire) != wires_.end();
}
