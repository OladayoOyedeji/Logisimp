/**
 * @file Project.cpp
 * @brief Implements circuit creation and changes across dependent circuits.
 * @author Cole Schwandt
 */

#include "Project.h"

#include "Gates.h"
#include "Pin.h"
#include "Subcircuit.h"
#include "WireEdge.h"
#include "WireNode.h"
#include "ProjectEdit.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <typeinfo>

namespace
{
/** @brief Folds ASCII letters without changing UTF-8 bytes or using a locale. */
char folded_name_character(char value)
{
    if (value >= 'A' && value <= 'Z')
    {
        return char(value - 'A' + 'a');
    }
    return value;
}

/** @brief Compares display names under the project's ASCII case policy. */
bool circuit_names_equal(const std::string & first, const std::string & second)
{
    if (first.size() != second.size())
    {
        return false;
    }
    for (std::size_t i = 0; i < first.size(); ++i)
    {
        if (folded_name_character(first[i]) != folded_name_character(second[i]))
        {
            return false;
        }
    }
    return true;
}

/** @brief Checks current circuits, including changes already made in a batch. */
bool circuit_name_used(const std::vector< Circuit * > & circuits, const std::string & name, const Circuit * ignored = nullptr)
{
    for (const Circuit * circuit : circuits)
    {
        if (circuit != ignored && circuit_names_equal(circuit->name(), name))
        {
            return true;
        }
    }
    return false;
}

/** @brief Trims the permitted surrounding whitespace and rejects blank names. */
std::string trimmed_circuit_name(const std::string & name)
{
    const std::string whitespace = " \t\n\r\f\v";
    std::string::size_type first = name.find_first_not_of(whitespace);
    if (first == std::string::npos)
    {
        throw std::invalid_argument("Circuit name must not be blank");
    }
    std::string::size_type last = name.find_last_not_of(whitespace);
    return name.substr(first, last - first + 1);
}
}

Project::~Project()
{
    // Registry pointers borrow the graph; drop them before deleting any blueprint.
    // The caller has already released queued work and runtime hierarchies.
    placements_.clear();

    for (Circuit * circuit : circuits_)
    {
        delete circuit;
    }
}

Circuit & Project::create_circuit()
{
    // Keep ownership and the placement registry in step if insertion fails.
    if (next_circuit_id_ == std::numeric_limits< int >::max())
    {
        throw std::overflow_error("Circuit IDs are exhausted");
    }
    unsigned long long number = next_circuit_id_;
    std::string name;
    do
    {
        ++number;
        name = "Circuit " + std::to_string(number);
    }
    while (circuit_name_used(circuits_, name));
    Circuit * circuit = new Circuit(CircuitId(next_circuit_id_++), name);
    circuit->edit_ = edit_;

    try
    {
        circuits_.push_back(circuit);

        try
        {
            placements_.emplace(circuit, std::vector< Placement >());
            try
            {
                circuits_by_id_.emplace(circuit->id().value, circuit);
            }
            catch (...)
            {
                placements_.erase(circuit);
                throw;
            }
        }
        catch (...) // in case of memory failure
        {
            circuits_.pop_back();
            throw;
        }
    }
    catch (...) // in case of memory failure
    {
        delete circuit;
        throw;
    }

    return *circuit;
}

Circuit & Project::copy_circuit(const Circuit & source)
{
    require_circuit(source);
    std::string base_name = source.name() + " copy";
    std::string name = base_name;
    unsigned long long suffix = 1;
    while (circuit_name_used(circuits_, name))
    {
        ++suffix;
        name = base_name + " " + std::to_string(suffix);
    }
    Circuit & destination = create_circuit();

    try
    {
        destination.name_.swap(name);
        std::unordered_map< const Component *, Component * > copied_components;
        std::unordered_map< const Port *, Port * > copied_ports;
        copied_components.reserve(source.components().size());

        for (const Component * component : source.components())
        {
            Component & component_copy = copy_component_impl(destination, *component);
            copied_components.emplace(component, &component_copy);

            for (int i = 0; i < component->input_count(); ++i)
            {
                copied_ports.emplace(component->input_port(i), component_copy.input_port(i));
            }
            for (int i = 0; i < component->output_count(); ++i)
            {
                copied_ports.emplace(component->output_port(i), component_copy.output_port(i));
            }
        }

        // Interface order can differ from component creation order after move_pin().
        for (int i = 0; i < int(source.input_pins().size()); ++i)
        {
            Pin & pin_copy = dynamic_cast< Pin & >(*copied_components.at(&source.input_pin(i)));
            move_pin(destination, pin_copy, i);
        }
        for (int i = 0; i < int(source.output_pins().size()); ++i)
        {
            Pin & pin_copy = dynamic_cast< Pin & >(*copied_components.at(&source.output_pin(i)));
            move_pin(destination, pin_copy, i);
        }

        std::unordered_map< const WireNode *, WireNode * > copied_nodes;
        copied_nodes.reserve(source.wire_nodes().size());
        for (const WireNode * node : source.wire_nodes())
        {
            WireNode & node_copy = create_wire_node(destination, node->width(), node->label());
            copied_nodes.emplace(node, &node_copy);
        }

        // Copy every segment, including cycles and segments without attached ports.
        for (const WireEdge * edge : source.wire_edges())
        {
            create_wire_edge(destination, *copied_nodes.at(edge->first()), *copied_nodes.at(edge->second()));
        }

        // Node order and attachment order retain deterministic listener scheduling.
        // Circuit derives fresh nets; no source net pointer survives the copy.
        for (const WireNode * node : source.wire_nodes())
        {
            for (const Port * port : node->ports())
            {
                connect(destination, *copied_ports.at(port), *copied_nodes.at(node));
            }
        }
    }
    catch (...)
    {
        // This new blueprint has no external placements. Removing it also
        // unregisters its copied placements from their shared child blueprints.
        remove_circuit(destination);
        throw;
    }

    return destination;
}

Component & Project::copy_component(Circuit & destination, const Component & source)
{
    require_circuit(destination);

    for (const Circuit * circuit : circuits_)
    {
        if (circuit->owns_component(source))
        {
            return copy_component_impl(destination, source);
        }
    }

    throw std::invalid_argument("Source component does not belong to this project");
}

Component & Project::copy_component_impl(Circuit & destination, const Component & source)
{
    const std::type_info & source_type = typeid(source);
    Component * component_copy = nullptr;

    // Exact types prevent silently discarding behavior added by an unknown subclass.
    if (source_type == typeid(Pin))
    {
        require_interfaces_editable(destination);
        const Pin & pin = dynamic_cast< const Pin & >(source);
        // Stage metadata before exposing the new interface to parent placements.
        component_copy = &destination.create_pin(pin.type(), pin.width());
    }
    else if (source_type == typeid(Subcircuit))
    {
        const Subcircuit & placement = dynamic_cast< const Subcircuit & >(source);
        component_copy = &create_subcircuit(destination, *placement.circuit());
    }
    else if (source_type == typeid(AndGate))
    {
        component_copy = &create_and_gate(destination, source.input_count());
    }
    else if (source_type == typeid(OrGate))
    {
        component_copy = &create_or_gate(destination, source.input_count());
    }
    else if (source_type == typeid(NotGate))
    {
        component_copy = &create_not_gate(destination);
    }
    else if (source_type == typeid(NandGate))
    {
        component_copy = &create_nand_gate(destination, source.input_count());
    }
    else if (source_type == typeid(NorGate))
    {
        component_copy = &create_nor_gate(destination, source.input_count());
    }
    else if (source_type == typeid(XorGate))
    {
        component_copy = &create_xor_gate(destination, source.input_count());
    }
    else if (source_type == typeid(XnorGate))
    {
        component_copy = &create_xnor_gate(destination, source.input_count());
    }
    else
    {
        throw std::invalid_argument("Component type does not support copying");
    }

    try
    {
        component_copy->label() = source.label();

        for (int i = 0; i < source.input_count(); ++i)
        {
            component_copy->input_port(i)->label() = source.input_port(i)->label();
        }
        for (int i = 0; i < source.output_count(); ++i)
        {
            component_copy->output_port(i)->label() = source.output_port(i)->label();
        }

        if (source_type == typeid(Pin))
        {
            sync_interfaces(destination);
        }
    }
    catch (...)
    {
        if (source_type == typeid(Pin))
        {
            destination.remove_component(dynamic_cast< Pin & >(*component_copy));
            // Cleanup compares IDs without allocating or reading string labels,
            // so a failed allocation cannot leave partial parent interface ports.
            for (const Placement & placement : placements_.at(&destination))
            {
                placement.parent->remove_obsolete_subcircuit_ports(*placement.subcircuit, destination.input_pins(), INPUT);
                placement.parent->remove_obsolete_subcircuit_ports(*placement.subcircuit, destination.output_pins(), OUTPUT);
            }
        }
        else
        {
            remove_component(destination, *component_copy);
        }
        throw;
    }

    return *component_copy;
}

WireNode & Project::copy_wire_node(Circuit & destination, const WireNode & source)
{
    require_circuit(destination);

    for (const Circuit * circuit : circuits_)
    {
        if (circuit->owns_wire_node(source))
        {
            return create_wire_node(destination, source.width(), source.label());
        }
    }

    throw std::invalid_argument("Source wire node does not belong to this project");
}

Subcircuit & Project::create_subcircuit(Circuit & parent, Circuit & circuit)
{
    require_circuit(parent);
    require_circuit(circuit);
    parent.require_editable();
    std::vector< Placement > & placements = placements_.at(&circuit);

    Subcircuit & subcircuit = parent.create_subcircuit(circuit);

    try
    {
        placements.push_back(Placement(parent, subcircuit));
    }
    catch (...)
    {
        parent.remove_component(subcircuit);
        throw;
    }

    return subcircuit;
}

void Project::sync_interfaces(const Circuit & circuit)
{
    // Only placements referencing this blueprint need synchronization.
    for (const Placement & placement : placements_.at(&circuit))
    {
        placement.parent->sync_subcircuit(*placement.subcircuit);
    }
}

Pin & Project::create_pin(Circuit & circuit, PinType type, int width)
{
    require_circuit(circuit);
    require_interfaces_editable(circuit);
    Pin & pin = circuit.create_pin(type, width);

    try
    {
        sync_interfaces(circuit);
    }
    catch (...)
    {
        circuit.remove_component(pin);
        // Removing a newly added interface needs no replacement allocations.
        sync_interfaces(circuit);
        throw;
    }

    return pin;
}

void Project::remove_component(Circuit & circuit, Component & component)
{
    require_circuit(circuit);
    // Overload resolution cannot identify a concrete type through Component &.
    Pin * pin = dynamic_cast< Pin * >(&component);
    if (pin != nullptr)
    {
        remove_component(circuit, *pin);
        return;
    }

    Subcircuit * subcircuit = dynamic_cast< Subcircuit * >(&component);
    if (subcircuit != nullptr)
    {
        remove_component(circuit, *subcircuit);
        return;
    }

    circuit.remove_component(component);
}

void Project::remove_component(Circuit & circuit, Pin & pin)
{
    require_circuit(circuit);
    require_interfaces_editable(circuit);
    circuit.remove_component(pin);

    sync_interfaces(circuit);
}

void Project::remove_component(Circuit & circuit, Subcircuit & subcircuit)
{
    require_circuit(circuit);
    circuit.require_editable();
    std::vector< Placement > & placements = placements_.at(subcircuit.circuit());

    // find pos of subcircuit
    // Verify the recorded owner before deletion, so a wrong-parent request cannot
    // remove the reference that protects the placement's blueprint from deletion.
    std::vector< Placement >::iterator position = placements.begin();
    while (position != placements.end() &&
           position->subcircuit != &subcircuit)
    {
        ++position;
    }

    // if subcircuit not found
    if (position == placements.end() ||
        position->parent != &circuit)
    {
        throw std::logic_error("Subcircuit placement is missing from the project registry");
    }

    circuit.remove_component(subcircuit);
    placements.erase(position);
}

void Project::set_pin_width(Circuit & circuit, Pin & pin, int width)
{
    require_circuit(circuit);
    require_interfaces_editable(circuit);
    circuit.set_pin_width(pin, width);

    sync_interfaces(circuit);
}

void Project::set_pin_type(Circuit & circuit, Pin & pin, PinType type)
{
    require_circuit(circuit);
    require_interfaces_editable(circuit);
    circuit.set_pin_type(pin, type);

    sync_interfaces(circuit);
}

void Project::set_pin_label(Circuit & circuit, Pin & pin, const std::string & label)
{
    require_circuit(circuit);
    circuit.set_pin_label(pin, label);

    sync_interfaces(circuit);
}

void Project::move_pin(Circuit & circuit, Pin & pin, int index)
{
    require_circuit(circuit);
    circuit.move_pin(pin, index);

    sync_interfaces(circuit);
}

AndGate & Project::create_and_gate(Circuit & circuit, int input_count)
{
    require_circuit(circuit);
    return circuit.create_and_gate(input_count);
}

OrGate & Project::create_or_gate(Circuit & circuit, int input_count)
{
    require_circuit(circuit);
    return circuit.create_or_gate(input_count);
}

NotGate & Project::create_not_gate(Circuit & circuit)
{
    require_circuit(circuit);
    return circuit.create_not_gate();
}

NandGate & Project::create_nand_gate(Circuit & circuit, int input_count)
{
    require_circuit(circuit);
    return circuit.create_nand_gate(input_count);
}

NorGate & Project::create_nor_gate(Circuit & circuit, int input_count)
{
    require_circuit(circuit);
    return circuit.create_nor_gate(input_count);
}

XorGate & Project::create_xor_gate(Circuit & circuit, int input_count)
{
    require_circuit(circuit);
    return circuit.create_xor_gate(input_count);
}

XnorGate & Project::create_xnor_gate(Circuit & circuit, int input_count)
{
    require_circuit(circuit);
    return circuit.create_xnor_gate(input_count);
}

WireNode & Project::create_wire_node(Circuit & circuit, int width, const std::string & label)
{
    require_circuit(circuit);
    return circuit.create_wire_node(width, label);
}

WireEdge & Project::create_wire_edge(Circuit & circuit, WireNode & first, WireNode & second)
{
    require_circuit(circuit);
    return circuit.create_wire_edge(first, second);
}

void Project::remove_wire_edge(Circuit & circuit, WireEdge & edge)
{
    require_circuit(circuit);
    circuit.remove_wire_edge(edge);
}

void Project::remove_wire_node(Circuit & circuit, WireNode & node)
{
    require_circuit(circuit);
    circuit.remove_wire_node(node);
}

void Project::connect(Circuit & circuit, Port & port, WireNode & node)
{
    require_circuit(circuit);
    circuit.connect(port, node);
}

void Project::disconnect(Circuit & circuit, Port & port)
{
    require_circuit(circuit);
    circuit.disconnect(port);
}

void Project::clear_circuit(Circuit & circuit)
{
    require_circuit(circuit);
    require_interfaces_editable(circuit);
    bool has_pins = (!circuit.input_pins().empty()) || (!circuit.output_pins().empty());

    // remove placements owned by this circuit before their
    // subcircuits are destroyed
    // Forget contained placements before clear() deletes them. Keep entries for
    // placements in other parents: they still reference this now-empty blueprint.
    for (Circuit * blueprint : circuits_)
    {
        std::vector< Placement > & placements = placements_.at(blueprint);

        std::vector< Placement >::iterator position = placements.begin();

        while (position != placements.end())
        {
            if (position->parent == &circuit)
            {
                position = placements.erase(position);
            }
            else
            {
                ++position;
            }
        }
    }

    circuit.clear();

    if (has_pins)
    {
        sync_interfaces(circuit);
    }
}

void Project::remove_circuit(Circuit & circuit)
{
    require_circuit(circuit);
    if (!placements_.at(&circuit).empty())
    {
        throw std::logic_error("Remove placements of this circuit before deleting it");
    }

    // unregister placements owned by this circuit before destroying it
    clear_circuit(circuit);

    std::vector< Circuit * >::iterator position = std::find(circuits_.begin(), circuits_.end(), &circuit);

    placements_.erase(&circuit);
    circuits_by_id_.erase(circuit.id().value);
    circuits_.erase(position);

    if (edit_ == nullptr || !edit_->retains(&circuit))
    {
        delete &circuit;
    }
}

void Project::require_circuit(const Circuit & circuit) const
{
    if (placements_.find(&circuit) == placements_.end())
    {
        throw std::invalid_argument("Circuit does not belong to this project");
    }
}

void Project::require_interfaces_editable(const Circuit & circuit) const
{
    circuit.require_editable();

    // A failed parent reset can leave the parent registered without a child
    // state. Check every placement owner before changing the shared interface.
    for (const Placement & placement : placements_.at(&circuit))
    {
        placement.parent->require_editable();
    }
}

Circuit * Project::find_circuit(CircuitId id)
{
    auto found = circuits_by_id_.find(id.value);
    return found == circuits_by_id_.end() ? nullptr : found->second;
}

const Circuit * Project::find_circuit(CircuitId id) const
{
    auto found = circuits_by_id_.find(id.value);
    return found == circuits_by_id_.end() ? nullptr : found->second;
}

void Project::set_circuit_name(CircuitId id, const std::string & name)
{
    Circuit * circuit = find_circuit(id);
    if (circuit == nullptr)
    {
        throw std::invalid_argument("Circuit ID does not belong to this project");
    }
    std::string trimmed = trimmed_circuit_name(name);
    if (circuit_name_used(circuits_, trimmed, circuit))
    {
        throw std::invalid_argument("Circuit name is already used in this project");
    }
    circuit->name_.swap(trimmed);
}

std::string Project::circuit_name(CircuitId id) const
{
    const Circuit * circuit = find_circuit(id);
    if (circuit == nullptr)
    {
        throw std::invalid_argument("Circuit ID does not belong to this project");
    }
    return circuit->name();
}

std::vector< CircuitInfo > Project::circuit_infos() const
{
    std::vector< CircuitInfo > result;
    result.reserve(circuits_.size());
    for (const Circuit * circuit : circuits_)
    {
        result.push_back({circuit->id(), circuit->name()});
    }
    return result;
}
