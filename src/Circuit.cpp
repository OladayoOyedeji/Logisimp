/**
 * @file Circuit.cpp
 * @brief Implements circuit components, wire graphs, and derived nets.
 * @author Cole Schwandt
 */

#include "Circuit.h"
#include "Component.h"
#include "Gates.h"
#include "Port.h"
#include "Subcircuit.h"
#include "WireNode.h"
#include "WireEdge.h"
#include "Net.h"
#include "ProjectEdit.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <unordered_set>

AndGate & Circuit::create_and_gate(int input_count)
{
    require_editable();
    AndGate * component = new AndGate(next_component_id_, input_count);
    store_component(component);
    return *component;
}

OrGate & Circuit::create_or_gate(int input_count)
{
    require_editable();
    OrGate * component = new OrGate(next_component_id_, input_count);
    store_component(component);
    return *component;
}

NotGate & Circuit::create_not_gate()
{
    require_editable();
    NotGate * component = new NotGate(next_component_id_);
    store_component(component);
    return *component;
}

NandGate & Circuit::create_nand_gate(int input_count)
{
    require_editable();
    NandGate * component = new NandGate(next_component_id_, input_count);
    store_component(component);
    return *component;
}

NorGate & Circuit::create_nor_gate(int input_count)
{
    require_editable();
    NorGate * component = new NorGate(next_component_id_, input_count);
    store_component(component);
    return *component;
}

XorGate & Circuit::create_xor_gate(int input_count)
{
    require_editable();
    XorGate * component = new XorGate(next_component_id_, input_count);
    store_component(component);
    return *component;
}

XnorGate & Circuit::create_xnor_gate(int input_count)
{
    require_editable();
    XnorGate * component = new XnorGate(next_component_id_, input_count);
    store_component(component);
    return *component;
}

Pin & Circuit::create_pin(PinType type, int width)
{
    require_editable();
    if (type != INPUT_PIN && type != OUTPUT_PIN)
    {
        throw std::invalid_argument("Invalid pin type");
    }

    Pin * pin = new Pin(next_component_id_, type, width);
    store_component(pin);

    try
    {
        if (type == INPUT_PIN)
        {
            input_pins_.push_back(pin);
        }
        else
        {
            output_pins_.push_back(pin);
        }
    }
    catch (...)
    {
        // Interface registration is part of pin creation; do not retain a
        // component that the circuit's public interface cannot find.
        components_.pop_back();
        discard(pin);
        throw;
    }

    return *pin;
}

Subcircuit & Circuit::create_subcircuit(Circuit & circuit)
{
    require_editable();
    // Runtime child states recursively instantiate blueprints, so a dependency
    // cycle would make both construction and dependency traversal unbounded.
    if (&circuit == this || circuit.uses(*this))
    {
        throw std::invalid_argument("Recursive subcircuit definitions are not allowed");
    }

    Subcircuit * subcircuit = new Subcircuit(next_component_id_, circuit);
    store_component(subcircuit);

    try
    {
        sync_subcircuit(*subcircuit);
    }
    catch (...)
    {
        // A placement is useful only after its interface is synchronized.
        // Removal also detaches any partially created ports before destruction.
        remove_component(*subcircuit);
        throw;
    }

    return *subcircuit;
}

WireNode & Circuit::create_wire_node(int width, const std::string & label)
{
    require_editable();
    if (width < 0)
    {
        throw std::invalid_argument("Node width constraint cannot be negative");
    }
    if (next_node_id_ == std::numeric_limits< int >::max())
    {
        throw std::overflow_error("Node IDs are exhausted");
    }
    WireNode * node = new WireNode(*this, next_node_id_, width, label);
    try
    {
        wire_nodes_.push_back(node);
    }
    catch (...)
    {
        discard(node);
        throw;
    }
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        wire_nodes_.pop_back();
        discard(node);
        throw;
    }
    ++next_node_id_;
    return *node;
}

WireEdge & Circuit::create_wire_edge(WireNode & first, WireNode & second)
{
    require_editable();
    if (!owns_wire_node(first) || !owns_wire_node(second))
    {
        throw std::invalid_argument("Edge endpoints must belong to this circuit");
    }
    if (&first == &second)
    {
        throw std::invalid_argument("Self edges are not allowed");
    }
    for (WireEdge * edge : first.edges_)
    {
        if (edge->first() == &second || edge->second() == &second)
        {
            throw std::invalid_argument("Duplicate undirected edges are not allowed");
        }
    }
    if (next_edge_id_ == std::numeric_limits< int >::max())
    {
        throw std::overflow_error("Edge IDs are exhausted");
    }
    // Reserve reciprocal adjacency storage before publishing any graph links.
    wire_edges_.reserve(wire_edges_.size() + 1);
    first.edges_.reserve(first.edges_.size() + 1);
    second.edges_.reserve(second.edges_.size() + 1);
    WireEdge * edge = new WireEdge(*this, next_edge_id_, first, second);
    wire_edges_.push_back(edge);
    first.edges_.push_back(edge);
    second.edges_.push_back(edge);
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        second.edges_.pop_back();
        first.edges_.pop_back();
        wire_edges_.pop_back();
        discard(edge);
        throw;
    }
    ++next_edge_id_;
    return *edge;
}

bool Circuit::uses(const Circuit & circuit) const
{
    for (Component * component : components_)
    {
        const Subcircuit * subcircuit = dynamic_cast< const Subcircuit * >(component);

        if (subcircuit != nullptr)
        {
            if (subcircuit->circuit() == &circuit || subcircuit->circuit()->uses(circuit))
            {
                return true;
            }
        }
    }

    return false;
}

void Circuit::set_pin_width(Pin & pin, int width)
{
    require_editable();
    if (!owns_component(pin))
    {
        throw std::invalid_argument("Pin does not belong to this circuit");
    }

    if (width <= 0)
    {
        throw std::invalid_argument("Pin width must be positive");
    }

    if (pin.width_ == width)
    {
        return;
    }

    Port * pin_port = pin.type_ == INPUT_PIN ? pin.output_port(0) : pin.input_port(0);
    // Do not resize a net shared with other ports. Detach this endpoint before
    // changing its shape so the remaining connections stay width-compatible.
    if (pin_port->net() != nullptr && pin_port->net()->width() != width)
    {
        disconnect(*pin_port);
    }
    pin_port->width_ = width;
    pin.width_ = width;
}

void Circuit::set_pin_type(Pin & pin, PinType type)
{
    require_editable();
    if (!owns_component(pin))
    {
        throw std::invalid_argument("Pin does not belong to this circuit");
    }

    if (type != INPUT_PIN && type != OUTPUT_PIN)
    {
        throw std::invalid_argument("Invalid pin type");
    }

    if (pin.type_ == type)
    {
        return;
    }

    std::vector< Pin * > & old_pins = pin.type_ == INPUT_PIN ? input_pins_ : output_pins_;
    std::vector< Pin * > & new_pins = type == INPUT_PIN ? input_pins_ : output_pins_;

    std::vector< Pin * >::iterator position = std::find(old_pins.begin(), old_pins.end(), &pin);

    if (position == old_pins.end())
    {
        throw std::logic_error("Pin is missing from the circuit interface");
    }

    Port * old_port = pin.type_ == INPUT_PIN ? pin.output_port(0) : pin.input_port(0);
    Port * new_port = nullptr;

    // Allocate the replacement and register the new interface entry first;
    // allocation failure must not destroy the old port or its connection.
    if (type == INPUT_PIN)
    {
        new_port = pin.add_output(pin.width_, "Y");
    }
    else
    {
        new_port = pin.add_input(pin.width_, "A");
    }

    try
    {
        new_pins.push_back(&pin);
    }
    catch (...)
    {
        pin.remove_port(*new_port);
        throw;
    }

    try
    {
        disconnect(*old_port);
    }
    catch (...)
    {
        new_pins.pop_back();
        pin.remove_port(*new_port);
        throw;
    }
    pin.remove_port(*old_port);
    old_pins.erase(position);
    pin.type_ = type;
}

void Circuit::set_pin_label(Pin & pin, const std::string & label)
{
    if (!owns_component(pin))
    {
        throw std::invalid_argument("Pin does not belong to this circuit");
    }

    pin.label() = label;
}

void Circuit::move_pin(Pin & pin, int index)
{
    if (!owns_component(pin))
    {
        throw std::invalid_argument("Pin does not belong to this circuit");
    }

    std::vector< Pin * > & pins = pin.type() == INPUT_PIN ? input_pins_ : output_pins_;

    if (index < 0 || index >= int(pins.size()))
    {
        throw std::out_of_range("Pin index is out of range");
    }

    std::vector< Pin * >::iterator position = std::find(pins.begin(), pins.end(), &pin);

    if (position == pins.end())
    {
        throw std::logic_error("Pin is missing from the circuit interface");
    }

    int old_index = int(position - pins.begin());

    if (old_index == index)
    {
        return;
    }

    // Rotate existing entries without allocating or recreating pins.
    if (old_index < index)
    {
        std::rotate(position, position + 1, pins.begin() + index + 1);
    }
    else
    {
        std::rotate(pins.begin() + index, position, position + 1);
    }
}

void Circuit::sync_subcircuit(Subcircuit & subcircuit)
{
    if (!owns_component(subcircuit))
    {
        throw std::invalid_argument("Subcircuit does not belong to this circuit");
    }

    const Circuit & blueprint = *subcircuit.circuit();

    // Remove obsolete ports from both directions first so a pin
    // that switches direction can reuse its original port ID.
    remove_obsolete_subcircuit_ports(subcircuit, blueprint.input_pins(), INPUT);
    remove_obsolete_subcircuit_ports(subcircuit, blueprint.output_pins(), OUTPUT);

    sync_subcircuit_ports(subcircuit, blueprint.input_pins(), INPUT);
    sync_subcircuit_ports(subcircuit, blueprint.output_pins(), OUTPUT);
}

void Circuit::remove_obsolete_subcircuit_ports(Subcircuit & subcircuit, const std::vector< Pin * > & interface_pins, PortDirection direction)
{
    std::vector< Port * > & placement_ports = direction == INPUT ? subcircuit.inputs_ : subcircuit.outputs_;

    for (int i = int(placement_ports.size()) - 1; i >= 0; --i)
    {
        // Walk backward because removal shifts later entries toward the front.
        Port * port = placement_ports[i];
        bool found = false;

        for (Pin * interface_pin : interface_pins)
        {
            if (interface_pin->id() == port->id())
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            disconnect(*port);
            subcircuit.remove_port(*port);
        }
    }
}

void Circuit::sync_subcircuit_ports(Subcircuit & subcircuit, const std::vector< Pin * > & interface_pins, PortDirection direction)
{
    std::vector< Port * > & placement_ports = direction == INPUT ? subcircuit.inputs_ : subcircuit.outputs_;

    for (int i = 0; i < int(interface_pins.size()); ++i)
    {
        Pin * interface_pin = interface_pins[i];
        Port * matching_port = nullptr;

        // IDs survive reordering; matching by index would attach old parent
        // connections to the wrong child interface pins after an interface edit.
        for (Port * port : placement_ports)
        {
            if (port->id() == interface_pin->id())
            {
                matching_port = port;
                break;
            }
        }

        if (matching_port == nullptr)
        {
            require_editable();
            if (direction == INPUT)
            {
                matching_port = subcircuit.add_input(interface_pin->width(), interface_pin->label(), interface_pin->id());
            }
            else
            {
                matching_port = subcircuit.add_output(interface_pin->width(), interface_pin->label(), interface_pin->id());
            }
        }
        else if (matching_port->width() != interface_pin->width())
        {
            require_editable();
            if (matching_port->net() != nullptr && matching_port->net()->width() != interface_pin->width())
            {
                disconnect(*matching_port);
            }
            matching_port->width_ = interface_pin->width();
        }

        if (matching_port->label() != interface_pin->label())
        {
            matching_port->label() = interface_pin->label();
        }

        // Preserve port objects and connections while matching interface order.
        std::vector< Port * >::iterator position = std::find(placement_ports.begin() + i, placement_ports.end(), matching_port);
        std::rotate(placement_ports.begin() + i, position, position + 1);
    }
}

void Circuit::connect(Port & port, WireNode & node)
{
    require_editable();
    if (!owns_port(port) || !owns_wire_node(node))
    {
        throw std::invalid_argument("Port and node must belong to this circuit");
    }
    if (port.node() != nullptr)
    {
        throw std::invalid_argument("Port is already attached to a node");
    }
    node.ports_.push_back(&port);
    port.node_ = &node;
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        port.node_ = nullptr;
        node.ports_.pop_back();
        throw;
    }
}

void Circuit::disconnect(Port & port)
{
    require_editable();
    if (!owns_port(port))
    {
        throw std::invalid_argument("Port does not belong to this circuit");
    }
    if (port.node() == nullptr)
    {
        return;
    }
    WireNode * node = port.node();
    auto position = std::find(node->ports_.begin(), node->ports_.end(), &port);
    int index = int(position - node->ports_.begin());
    node->ports_.erase(position);
    port.node_ = nullptr;
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        // Erasure retains capacity, so restoring this pointer cannot allocate.
        node->ports_.insert(node->ports_.begin() + index, &port);
        port.node_ = node;
        throw;
    }
}

void Circuit::detach_ports(const std::vector< Port * > & ports)
{
    bool attached = false;
    for (Port * port : ports)
    {
        attached = attached || port->connected();
    }
    if (!attached)
    {
        // Copy-failure cleanup removes new, disconnected ports without allocating.
        return;
    }
    std::vector< std::vector< Port * > > saved_ports;
    std::vector< WireNode * > saved_nodes;
    saved_ports.reserve(wire_nodes_.size());
    saved_nodes.reserve(ports.size());
    for (WireNode * node : wire_nodes_)
    {
        saved_ports.push_back(node->ports_);
    }
    for (Port * port : ports)
    {
        saved_nodes.push_back(port->node());
    }
    for (Port * port : ports)
    {
        if (port->node() != nullptr)
        {
            auto & attached_ports = port->node_->ports_;
            attached_ports.erase(std::find(attached_ports.begin(), attached_ports.end(), port));
            port->node_ = nullptr;
        }
    }
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        for (int i = 0; i < int(wire_nodes_.size()); ++i)
        {
            wire_nodes_[i]->ports_.swap(saved_ports[i]);
        }
        for (int i = 0; i < int(ports.size()); ++i)
        {
            ports[i]->node_ = saved_nodes[i];
        }
        throw;
    }
}

void Circuit::remove_component(Component & component)
{
    require_editable();
    Component * component_pointer = &component;
    auto position = std::find(components_.begin(), components_.end(), component_pointer);
    if (position == components_.end())
    {
        throw std::invalid_argument("Component does not belong to this circuit");
    }
    bool attached = false;
    for (Port * port : component.inputs())
    {
        attached = attached || port->connected();
    }
    for (Port * port : component.outputs())
    {
        attached = attached || port->connected();
    }
    if (attached)
    {
        std::vector< Port * > ports = component.inputs();
        ports.insert(ports.end(), component.outputs().begin(), component.outputs().end());
        detach_ports(ports);
    }
    // Remove the interface entry only after every fallible connectivity operation.
    Pin * pin = dynamic_cast< Pin * >(&component);
    if (pin != nullptr)
    {
        remove_pin_reference(*pin);
    }
    components_.erase(position);
    discard(component_pointer);
}

void Circuit::remove_component(Pin & pin)
{
    Component & component = pin;
    remove_component(component);
}

void Circuit::remove_wire_edge(WireEdge & edge)
{
    require_editable();
    if (!owns_wire_edge(edge))
    {
        throw std::invalid_argument("Edge does not belong to this circuit");
    }
    WireNode * first = edge.first();
    WireNode * second = edge.second();
    auto position = std::find(wire_edges_.begin(), wire_edges_.end(), &edge);
    auto first_position = std::find(first->edges_.begin(), first->edges_.end(), &edge);
    auto second_position = std::find(second->edges_.begin(), second->edges_.end(), &edge);
    int index = int(position - wire_edges_.begin());
    int first_index = int(first_position - first->edges_.begin());
    int second_index = int(second_position - second->edges_.begin());
    wire_edges_.erase(position);
    first->edges_.erase(first_position);
    second->edges_.erase(second_position);
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        wire_edges_.insert(wire_edges_.begin() + index, &edge);
        first->edges_.insert(first->edges_.begin() + first_index, &edge);
        second->edges_.insert(second->edges_.begin() + second_index, &edge);
        throw;
    }
    discard(&edge);
}

void Circuit::remove_wire_node(WireNode & node)
{
    require_editable();
    if (!owns_wire_node(node))
    {
        throw std::invalid_argument("Node does not belong to this circuit");
    }
    // Save before mutating. Swapping these vectors restores adjacency without
    // allocating even when constructing a replacement net failed to allocate.
    std::vector< WireNode * > saved_nodes = wire_nodes_;
    std::vector< WireEdge * > saved_edges = wire_edges_;
    std::vector< std::vector< WireEdge * > > saved_adjacency;
    saved_adjacency.reserve(wire_nodes_.size());
    for (WireNode * graph_node : wire_nodes_)
    {
        saved_adjacency.push_back(graph_node->edges_);
    }
    std::vector< WireEdge * > incident_edges = node.edges_;
    std::vector< Port * > attached_ports = node.ports_;
    for (WireEdge * edge : incident_edges)
    {
        WireNode * other = edge->first() == &node ? edge->second() : edge->first();
        other->edges_.erase(std::find(other->edges_.begin(), other->edges_.end(), edge));
        wire_edges_.erase(std::find(wire_edges_.begin(), wire_edges_.end(), edge));
    }
    for (Port * port : attached_ports)
    {
        port->node_ = nullptr;
    }
    wire_nodes_.erase(std::find(wire_nodes_.begin(), wire_nodes_.end(), &node));
    try
    {
        rebuild_nets();
    }
    catch (...)
    {
        wire_nodes_.swap(saved_nodes);
        wire_edges_.swap(saved_edges);
        for (int i = 0; i < int(wire_nodes_.size()); ++i)
        {
            wire_nodes_[i]->edges_.swap(saved_adjacency[i]);
        }
        for (Port * port : attached_ports)
        {
            port->node_ = &node;
        }
        throw;
    }
    for (WireEdge * edge : incident_edges)
    {
        discard(edge);
    }
    discard(&node);
}

void Circuit::clear()
{
    require_editable();
    // Detach both ends while components and their ports are still alive.
    // Cleanup allocates no replacement nets: it is also the copy rollback path.
    for (WireNode * node : wire_nodes_)
    {
        for (Port * port : node->ports_)
        {
            port->node_ = nullptr;
            port->net_ = nullptr;
        }
    }
    for (Net * net : nets_)
    {
        discard(net);
    }
    for (WireEdge * edge : wire_edges_)
    {
        discard(edge);
    }
    for (WireNode * node : wire_nodes_)
    {
        discard(node);
    }
    for (Component * component : components_)
    {
        discard(component);
    }
    nets_.clear();
    wire_edges_.clear();
    wire_nodes_.clear();
    components_.clear();
    input_pins_.clear();
    output_pins_.clear();
}

void Circuit::store_component(Component * component)
{
    try
    {
        require_editable();
    }
    catch (...)
    {
        delete component;
        throw;
    }
    if (next_component_id_ == std::numeric_limits< int >::max())
    {
        delete component;
        throw std::overflow_error("Component IDs are exhausted");
    }

    try
    {
        component->edit_ = edit_;
        components_.push_back(component);
    }
    catch (...)
    {
        delete component;
        throw;
    }

    // Advance only after ownership is recorded. Removed IDs are never recycled,
    // including IDs consumed by later failed interface registration.
    next_component_id_++;
}

void Circuit::remove_pin_reference(Pin & pin)
{
    if (pin.type() == INPUT_PIN)
    {
        std::vector< Pin * >::iterator position =
            std::find(input_pins_.begin(), input_pins_.end(), &pin);

        if (position == input_pins_.end())
        {
            throw std::logic_error("Input pin is missing from the circuit interface");
        }

        input_pins_.erase(position);
    }
    else
    {
        std::vector< Pin * >::iterator position =
            std::find(output_pins_.begin(), output_pins_.end(), &pin);

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

bool Circuit::owns_wire_node(const WireNode & node) const
{
    return std::find(wire_nodes_.begin(), wire_nodes_.end(), &node) != wire_nodes_.end();
}

bool Circuit::owns_wire_edge(const WireEdge & edge) const
{
    return std::find(wire_edges_.begin(), wire_edges_.end(), &edge) != wire_edges_.end();
}

bool Circuit::owns_port(const Port & port) const
{
    Component * owner = port.owner();
    return owner != nullptr && owns_component(*owner) &&
        (std::find(owner->inputs().begin(), owner->inputs().end(), &port) != owner->inputs().end() ||
         std::find(owner->outputs().begin(), owner->outputs().end(), &port) != owner->outputs().end());
}

void Circuit::require_editable() const
{
    if (active_states_ != 0 && edit_ == nullptr)
    {
        throw std::logic_error("Use ProjectController for live edits, or clear affected simulator queues and destroy runtime states before direct Project editing");
    }
}

bool Circuit::preserves(const void * object) const noexcept
{
    return edit_ != nullptr && edit_->retains(object);
}

void Circuit::rebuild_nets()
{
    std::vector< Net * > staged_nets;
    std::unordered_set< WireNode * > visited;
    int next_id = next_net_id_;
    try
    {
        for (WireNode * start : wire_nodes_)
        {
            if (!visited.insert(start).second)
            {
                continue;
            }
            std::vector< WireNode * > component_nodes;
            component_nodes.push_back(start);
            for (int i = 0; i < int(component_nodes.size()); ++i)
            {
                WireNode * node = component_nodes[i];
                for (WireEdge * edge : node->edges_)
                {
                    WireNode * neighbor = edge->first() == node ? edge->second() : edge->first();
                    if (visited.insert(neighbor).second)
                    {
                        component_nodes.push_back(neighbor);
                    }
                }
            }
            std::sort(component_nodes.begin(), component_nodes.end(), [](WireNode * first, WireNode * second)
            {
                return first->id() < second->id();
            });
            int width = 0;
            Port * driver = nullptr;
            std::vector< Port * > ports;
            std::vector< Port * > listeners;
            for (WireNode * node : component_nodes)
            {
                if (node->width() != 0)
                {
                    if (width != 0 && width != node->width())
                    {
                        throw std::invalid_argument("Connected node width constraints do not match");
                    }
                    width = node->width();
                }
                for (Port * port : node->ports_)
                {
                    if (width != 0 && width != port->width())
                    {
                        throw std::invalid_argument("Attached port widths do not match the connected graph");
                    }
                    width = port->width();
                    ports.push_back(port);
                    if (port->direction() == OUTPUT)
                    {
                        // The signal model has no multiple-driver resolution. Reject a second
                        // source instead of making propagation depend on evaluation order.
                        if (driver != nullptr)
                        {
                            throw std::invalid_argument("Connected graph already has an OUTPUT driver");
                        }
                        driver = port;
                    }
                    else
                    {
                        listeners.push_back(port);
                    }
                }
            }
            // Bare geometry can be built before its signal width is known.
            // Only components containing ports need runtime signal storage.
            if (ports.empty())
            {
                continue;
            }
            if (next_id == std::numeric_limits< int >::max())
            {
                throw std::overflow_error("Derived net IDs are exhausted");
            }
            Net * net = new Net(*this, next_id, width);
            try
            {
                staged_nets.push_back(net);
            }
            catch (...)
            {
                discard(net);
                throw;
            }
            ++next_id;
            net->driver_ = driver;
            net->nodes_.swap(component_nodes);
            net->ports_.swap(ports);
            net->listeners_.swap(listeners);
        }
    }
    catch (...)
    {
        for (Net * net : staged_nets)
        {
            discard(net);
        }
        throw;
    }
    // Nothing below allocates or validates. Publish every derived relationship
    // together, then delete obsolete nets only after all caches point elsewhere.
    for (Component * component : components_)
    {
        for (Port * port : component->inputs())
        {
            port->net_ = nullptr;
        }
        for (Port * port : component->outputs())
        {
            port->net_ = nullptr;
        }
    }
    for (WireNode * node : wire_nodes_)
    {
        node->net_ = nullptr;
    }
    nets_.swap(staged_nets);
    next_net_id_ = next_id;
    for (Net * net : nets_)
    {
        for (WireNode * node : net->nodes_)
        {
            node->net_ = net;
        }
        for (Port * port : net->ports_)
        {
            port->net_ = net;
        }
    }
    for (Net * net : staged_nets)
    {
        discard(net);
    }
}
