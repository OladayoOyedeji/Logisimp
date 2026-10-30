/**
 * @file ProjectEdit.cpp
 * @brief Implements structural rollback without replacing surviving objects.
 */

#include "ProjectEdit.h"

#include <algorithm>
#include <stdexcept>

namespace
{
/**
 * @brief Checks pointer identity in a borrowed object list.
 * @tparam T Type of object referenced by the list.
 * @param objects Pointer list to search without changing ownership.
 * @param object Borrowed address to find; it is not dereferenced.
 * @return True if the address occurs in the list.
 */
template< typename T > bool contains(const std::vector< T * > & objects, const T * object)
{
    return std::find(objects.begin(), objects.end(), object) != objects.end();
}
}

ProjectEdit::CircuitSnapshot::CircuitSnapshot(Circuit & circuit)
    : circuit(&circuit), name(circuit.name_), components(circuit.components_), components_by_id(circuit.components_by_id_), nodes_by_id(circuit.nodes_by_id_), edges_by_id(circuit.edges_by_id_), nodes(circuit.wire_nodes_), edges(circuit.wire_edges_), nets(circuit.nets_), inputs(circuit.input_pins_), outputs(circuit.output_pins_)
{}

ProjectEdit::ComponentSnapshot::ComponentSnapshot(Component & component)
    : component(&component), label(component.label_), inputs(component.inputs_), outputs(component.outputs_), ports_by_id(component.ports_by_id_), pin(dynamic_cast< Pin * >(&component)), pin_type(pin == nullptr ? INPUT_PIN : pin->type_), pin_width(pin == nullptr ? 0 : pin->width_)
{}

ProjectEdit::PortSnapshot::PortSnapshot(Port & port)
    : port(&port), width(port.width_), direction(port.direction_), owner(port.owner_), node(port.node_), net(port.net_), label(port.label_)
{}

ProjectEdit::NodeSnapshot::NodeSnapshot(WireNode & node)
    : node(&node), width(node.width_), label(node.label_), edges(node.edges_), ports(node.ports_), net(node.net_)
{}

ProjectEdit::ProjectEdit(Project & project)
    : project_(project), finished_(false)
{
    if (project.edit_ != nullptr)
    {
        throw std::logic_error("A structural edit is already in progress");
    }

    // Every potentially allocating copy finishes before granting edit access.
    // Original objects are recorded even if an edit later removes their owner.
    circuits_ = project.circuits_;
    circuits_by_id_ = project.circuits_by_id_;
    placements_ = project.placements_;
    circuit_snapshots_.reserve(circuits_.size());
    for (Circuit * circuit : circuits_)
    {
        circuit_snapshots_.emplace_back(*circuit);
        retained_.insert(circuit);
        for (Component * component : circuit->components_)
        {
            component_snapshots_.emplace_back(*component);
            retained_.insert(component);
            for (Port * port : component->inputs_)
            {
                port_snapshots_.emplace_back(*port);
                retained_.insert(port);
            }
            for (Port * port : component->outputs_)
            {
                port_snapshots_.emplace_back(*port);
                retained_.insert(port);
            }
        }
        for (WireNode * node : circuit->wire_nodes_)
        {
            node_snapshots_.emplace_back(*node);
            retained_.insert(node);
        }
        for (WireEdge * edge : circuit->wire_edges_)
        {
            retained_.insert(edge);
        }
        for (Net * net : circuit->nets_)
        {
            retained_.insert(net);
        }
    }

    project.edit_ = this;
    for (Circuit * circuit : circuits_)
    {
        circuit->edit_ = this;
        for (Component * component : circuit->components_)
        {
            component->edit_ = this;
        }
    }
}

ProjectEdit::~ProjectEdit()
{
    if (!finished_)
    {
        rollback();
    }
}

bool ProjectEdit::retains(const void * object) const noexcept
{
    return retained_.find(object) != retained_.end();
}

void ProjectEdit::disable() noexcept
{
    project_.edit_ = nullptr;
    // Some original circuits and components may no longer be in the project.
    for (CircuitSnapshot & snapshot : circuit_snapshots_)
    {
        snapshot.circuit->edit_ = nullptr;
        for (Component * component : snapshot.circuit->components_)
        {
            component->edit_ = nullptr;
        }
    }
    for (ComponentSnapshot & snapshot : component_snapshots_)
    {
        snapshot.component->edit_ = nullptr;
    }
    for (Circuit * circuit : project_.circuits_)
    {
        circuit->edit_ = nullptr;
        for (Component * component : circuit->components_)
        {
            component->edit_ = nullptr;
        }
    }
}

void ProjectEdit::rollback() noexcept
{
    disable();

    // Delete only candidate objects. Original objects are still owned by the
    // snapshot, including components removed before an interface update failed.
    for (CircuitSnapshot & snapshot : circuit_snapshots_)
    {
        Circuit & circuit = *snapshot.circuit;
        for (Component * component : circuit.components_)
        {
            if (!retains(component))
            {
                delete component;
            }
        }
        for (Net * net : circuit.nets_)
        {
            if (!retains(net))
            {
                delete net;
            }
        }
        for (WireEdge * edge : circuit.wire_edges_)
        {
            if (!retains(edge))
            {
                delete edge;
            }
        }
        for (WireNode * node : circuit.wire_nodes_)
        {
            if (!retains(node))
            {
                delete node;
            }
        }
    }
    for (ComponentSnapshot & snapshot : component_snapshots_)
    {
        Component & component = *snapshot.component;
        for (Port * port : component.inputs_)
        {
            if (!retains(port))
            {
                delete port;
            }
        }
        for (Port * port : component.outputs_)
        {
            if (!retains(port))
            {
                delete port;
            }
        }
    }
    for (Circuit * circuit : project_.circuits_)
    {
        if (!retains(circuit))
        {
            delete circuit;
        }
    }

    // Swaps restore storage without allocating, even during bad_alloc handling.
    for (ComponentSnapshot & snapshot : component_snapshots_)
    {
        Component & component = *snapshot.component;
        component.label_.swap(snapshot.label);
        component.inputs_.swap(snapshot.inputs);
        component.outputs_.swap(snapshot.outputs);
        component.ports_by_id_.swap(snapshot.ports_by_id);
        // Consumed IDs and port generations remain retired after rollback.
        if (snapshot.pin != nullptr)
        {
            snapshot.pin->type_ = snapshot.pin_type;
            snapshot.pin->width_ = snapshot.pin_width;
        }
    }
    for (PortSnapshot & snapshot : port_snapshots_)
    {
        Port & port = *snapshot.port;
        port.width_ = snapshot.width;
        port.direction_ = snapshot.direction;
        port.owner_ = snapshot.owner;
        port.node_ = snapshot.node;
        port.net_ = snapshot.net;
        port.label_.swap(snapshot.label);
    }
    for (NodeSnapshot & snapshot : node_snapshots_)
    {
        WireNode & node = *snapshot.node;
        node.width_ = snapshot.width;
        node.label_.swap(snapshot.label);
        node.edges_.swap(snapshot.edges);
        node.ports_.swap(snapshot.ports);
        node.net_ = snapshot.net;
    }
    for (CircuitSnapshot & snapshot : circuit_snapshots_)
    {
        Circuit & circuit = *snapshot.circuit;
        circuit.name_.swap(snapshot.name);
        circuit.components_.swap(snapshot.components);
        circuit.wire_nodes_.swap(snapshot.nodes);
        circuit.wire_edges_.swap(snapshot.edges);
        circuit.nets_.swap(snapshot.nets);
        circuit.input_pins_.swap(snapshot.inputs);
        circuit.output_pins_.swap(snapshot.outputs);
        circuit.components_by_id_.swap(snapshot.components_by_id);
        circuit.nodes_by_id_.swap(snapshot.nodes_by_id);
        circuit.edges_by_id_.swap(snapshot.edges_by_id);
        // Counters remain monotonic: handles exposed by a failed edit stay stale.
    }
    project_.circuits_.swap(circuits_);
    project_.circuits_by_id_.swap(circuits_by_id_);
    project_.placements_.swap(placements_);
    finished_ = true;
}

void ProjectEdit::commit() noexcept
{
    if (finished_)
    {
        return;
    }
    disable();

    // A removed port is no longer owned by its component's current port lists.
    // Ports still in a removed component are deleted by that component instead.
    for (PortSnapshot & snapshot : port_snapshots_)
    {
        Component & owner = *snapshot.owner;
        if (!contains(owner.inputs_, snapshot.port) && !contains(owner.outputs_, snapshot.port))
        {
            delete snapshot.port;
        }
    }
    for (CircuitSnapshot & snapshot : circuit_snapshots_)
    {
        Circuit & circuit = *snapshot.circuit;
        for (Component * component : snapshot.components)
        {
            if (!contains(circuit.components_, component))
            {
                delete component;
            }
        }
        for (Net * net : snapshot.nets)
        {
            if (!contains(circuit.nets_, net))
            {
                delete net;
            }
        }
        for (WireEdge * edge : snapshot.edges)
        {
            if (!contains(circuit.wire_edges_, edge))
            {
                delete edge;
            }
        }
        for (WireNode * node : snapshot.nodes)
        {
            if (!contains(circuit.wire_nodes_, node))
            {
                delete node;
            }
        }
        if (!contains(project_.circuits_, &circuit))
        {
            delete &circuit;
        }
    }
    finished_ = true;
}
