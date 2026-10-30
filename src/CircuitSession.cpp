/** @file CircuitSession.cpp @brief Session facade over controller operations. */

#include "CircuitSession.h"

#include "CircuitState.h"
#include "Gates.h"
#include "ProjectController.h"
#include "Subcircuit.h"

#include <stdexcept>

CircuitSession::CircuitSession(ProjectController & controller, CircuitId circuit)
    : controller_(&controller), lifetime_(controller.lifetime_), circuit_id_(circuit), explicit_instance_(false)
{}

CircuitSession::CircuitSession(ProjectController & controller, CircuitId circuit, const RuntimePath & instance)
    : controller_(&controller), lifetime_(controller.lifetime_), circuit_id_(circuit), explicit_instance_(true), instance_(instance)
{}

ProjectController & CircuitSession::controller() const
{
    if (lifetime_.expired())
    {
        throw std::logic_error("Session controller has been destroyed");
    }
    return *controller_;
}

Circuit & CircuitSession::circuit() const
{
    Circuit * result = controller().project_.find_circuit(circuit_id_);
    if (result == nullptr)
    {
        throw std::invalid_argument("Session circuit has been deleted");
    }
    return *result;
}

Component & CircuitSession::component(ComponentId id) const
{
    Circuit & blueprint = circuit();
    if (id.is_scoped() && !id.matches(circuit_id_, lifetime_))
    {
        throw std::invalid_argument("Component ID belongs to a different controller or circuit");
    }
    Component * result = blueprint.find_component(id.value);
    if (result == nullptr)
    {
        throw std::invalid_argument("Component ID is not present in this circuit");
    }
    return *result;
}

ComponentId CircuitSession::component_id(int value) const
{
    return ComponentId(value, circuit_id_, lifetime_);
}

PortId CircuitSession::port_id(const Port & port) const
{
    return PortId(component_id(port.owner()->id()), port.id(), port.generation());
}

Pin & CircuitSession::pin(ComponentId id) const
{
    Pin * result = dynamic_cast< Pin * >(&component(id));
    if (result == nullptr)
    {
        throw std::invalid_argument("Component is not a pin");
    }
    return *result;
}

Port & CircuitSession::port(PortId id) const
{
    Port * result = component(id.component).find_port(id.value);
    if (result == nullptr || result->generation() != id.generation)
    {
        throw std::invalid_argument("Port ID has been deleted or replaced");
    }
    return *result;
}

WireNode & CircuitSession::node(WireNodeId id) const
{
    WireNode * result = circuit().find_wire_node(id.value);
    if (result == nullptr)
    {
        throw std::invalid_argument("Wire node ID is not present in this circuit");
    }
    return *result;
}

WireEdge & CircuitSession::edge(WireEdgeId id) const
{
    WireEdge * result = circuit().find_wire_edge(id.value);
    if (result == nullptr)
    {
        throw std::invalid_argument("Wire edge ID is not present in this circuit");
    }
    return *result;
}

void CircuitSession::modify(const std::function< void() > & operation)
{
    ProjectController & owner = controller();
    try
    {
        operation();
    }
    catch (...)
    {
        if (owner.batching_)
        {
            owner.batch_failed_ = true;
        }
        throw;
    }
}

std::vector< ComponentId > CircuitSession::components() const
{
    std::vector< ComponentId > result;
    for (Component * item : circuit().components())
    {
        result.push_back(component_id(item->id()));
    }
    return result;
}

std::vector< WireNodeId > CircuitSession::nodes() const
{
    std::vector< WireNodeId > result;
    for (WireNode * item : circuit().wire_nodes())
    {
        result.push_back(WireNodeId(item->id()));
    }
    return result;
}

std::vector< WireEdgeId > CircuitSession::edges() const
{
    std::vector< WireEdgeId > result;
    for (WireEdge * item : circuit().wire_edges())
    {
        result.push_back(WireEdgeId(item->id()));
    }
    return result;
}

bool CircuitSession::contains(ComponentId id) const
{
    Circuit & blueprint = circuit();
    return (!id.is_scoped() || id.matches(circuit_id_, lifetime_)) &&
           blueprint.find_component(id.value) != nullptr;
}

bool CircuitSession::contains_scoped(ComponentId id) const
{
    Circuit & blueprint = circuit();
    return id.matches(circuit_id_, lifetime_) && blueprint.find_component(id.value) != nullptr;
}

CircuitId CircuitSession::subcircuit_blueprint(ComponentId placement) const
{
    const Subcircuit * item = dynamic_cast< const Subcircuit * >(&component(placement));
    if (item == nullptr)
    {
        throw std::invalid_argument("Component is not a subcircuit placement");
    }
    return item->circuit()->id();
}

std::string CircuitSession::circuit_name() const
{
    return controller().circuit_name(circuit_id_);
}

void CircuitSession::set_circuit_name(const std::string & name)
{
    modify([&]() { controller().set_circuit_name(circuit_id_, name); });
}

ComponentKind CircuitSession::kind(ComponentId id) const
{
    Component & item = component(id);
    if (Pin * item_pin = dynamic_cast< Pin * >(&item))
    {
        return item_pin->type() == INPUT_PIN ? ComponentKind::InputPin : ComponentKind::OutputPin;
    }
    if (dynamic_cast< AndGate * >(&item))
    {
        return ComponentKind::AndGate;
    }
    if (dynamic_cast< OrGate * >(&item))
    {
        return ComponentKind::OrGate;
    }
    if (dynamic_cast< NotGate * >(&item))
    {
        return ComponentKind::NotGate;
    }
    if (dynamic_cast< NandGate * >(&item))
    {
        return ComponentKind::NandGate;
    }
    if (dynamic_cast< NorGate * >(&item))
    {
        return ComponentKind::NorGate;
    }
    if (dynamic_cast< XorGate * >(&item))
    {
        return ComponentKind::XorGate;
    }
    if (dynamic_cast< XnorGate * >(&item))
    {
        return ComponentKind::XnorGate;
    }
    if (dynamic_cast< Subcircuit * >(&item))
    {
        return ComponentKind::Subcircuit;
    }
    throw std::invalid_argument("Unsupported component type");
}

std::string CircuitSession::label(ComponentId id) const
{
    return component(id).label();
}

int CircuitSession::pin_width(ComponentId id) const
{
    return pin(id).width();
}

PinType CircuitSession::pin_type(ComponentId id) const
{
    return pin(id).type();
}

ComponentId CircuitSession::create(ComponentKind kind)
{
    bool pin_or_inverter = kind == ComponentKind::InputPin || kind == ComponentKind::OutputPin || kind == ComponentKind::NotGate;
    return create(kind, pin_or_inverter ? 1 : 2);
}

ComponentId CircuitSession::create(ComponentKind kind, int size)
{
    ComponentId result;
    modify([&]()
    {
        ProjectController & owner = controller();
        Circuit & target = circuit();
        if (size <= 0 || (kind == ComponentKind::NotGate && size != 1))
        {
            throw std::invalid_argument("Component size must be positive; NOT gates have one input");
        }
        Component * created = nullptr;
        switch (kind)
        {
        case ComponentKind::InputPin: created = &owner.create_pin(target, INPUT_PIN, size); break;
        case ComponentKind::OutputPin: created = &owner.create_pin(target, OUTPUT_PIN, size); break;
        case ComponentKind::AndGate: created = &owner.create_and_gate(target, size); break;
        case ComponentKind::OrGate: created = &owner.create_or_gate(target, size); break;
        case ComponentKind::NotGate: created = &owner.create_not_gate(target); break;
        case ComponentKind::NandGate: created = &owner.create_nand_gate(target, size); break;
        case ComponentKind::NorGate: created = &owner.create_nor_gate(target, size); break;
        case ComponentKind::XorGate: created = &owner.create_xor_gate(target, size); break;
        case ComponentKind::XnorGate: created = &owner.create_xnor_gate(target, size); break;
        default: throw std::invalid_argument("Use create_subcircuit for a placement");
        }
        result = component_id(created->id());
    });
    return result;
}

ComponentId CircuitSession::create_subcircuit(CircuitId blueprint)
{
    ComponentId result;
    modify([&]()
    {
        Circuit * child = controller().project_.find_circuit(blueprint);
        if (child == nullptr)
        {
            throw std::invalid_argument("Subcircuit blueprint ID is not present");
        }
        result = component_id(controller().create_subcircuit(circuit(), *child).id());
    });
    return result;
}

ComponentId CircuitSession::duplicate_component(ComponentId id)
{
    ComponentId result;
    modify([&]() { result = component_id(controller().copy_component(circuit(), component(id)).id()); });
    return result;
}

void CircuitSession::remove_component(ComponentId id)
{
    modify([&]() { controller().remove_component(circuit(), component(id)); });
}

void CircuitSession::set_pin_width(ComponentId id, int width)
{
    modify([&]() { controller().set_pin_width(circuit(), pin(id), width); });
}

void CircuitSession::set_pin_type(ComponentId id, PinType type)
{
    modify([&]() { controller().set_pin_type(circuit(), pin(id), type); });
}

void CircuitSession::set_label(ComponentId id, const std::string & label)
{
    modify([&]() { controller().set_label(circuit(), component(id), label); });
}

void CircuitSession::move_pin(ComponentId id, int index)
{
    modify([&]() { controller().move_pin(circuit(), pin(id), index); });
}

void CircuitSession::clear()
{
    modify([&]() { controller().clear_circuit(circuit()); });
}

std::vector< PortId > CircuitSession::inputs(ComponentId id) const
{
    std::vector< PortId > result;
    for (Port * item : component(id).inputs())
    {
        result.push_back(port_id(*item));
    }
    return result;
}

std::vector< PortId > CircuitSession::outputs(ComponentId id) const
{
    std::vector< PortId > result;
    for (Port * item : component(id).outputs())
    {
        result.push_back(port_id(*item));
    }
    return result;
}

PortId CircuitSession::input_port(ComponentId id, int index) const
{
    return port_id(*component(id).input_port(index));
}

PortId CircuitSession::output_port(ComponentId id, int index) const
{
    return port_id(*component(id).output_port(index));
}

PortId CircuitSession::interface_port(ComponentId placement, ComponentId interface_pin) const
{
    Subcircuit * item = dynamic_cast< Subcircuit * >(&component(placement));
    if (item == nullptr ||
        (interface_pin.is_scoped() && !interface_pin.matches(item->circuit()->id(), lifetime_)) ||
        dynamic_cast< const Pin * >(item->circuit()->find_component(interface_pin.value)) == nullptr)
    {
        throw std::invalid_argument("Expected a placement and one of its blueprint's interface pins");
    }
    Port * result = item->find_port(interface_pin.value);
    if (result == nullptr)
    {
        throw std::invalid_argument("Placement interface port is missing");
    }
    return port_id(*result);
}

int CircuitSession::port_width(PortId id) const
{
    return port(id).width();
}

PortDirection CircuitSession::port_direction(PortId id) const
{
    return port(id).direction();
}

std::string CircuitSession::port_label(PortId id) const
{
    return port(id).label();
}

WireNodeId CircuitSession::attached_node(PortId id) const
{
    const WireNode * attached = port(id).node();
    return attached == nullptr ? WireNodeId() : WireNodeId(attached->id());
}

std::pair< WireNodeId, WireNodeId > CircuitSession::edge_endpoints(WireEdgeId id) const
{
    const WireEdge & item = edge(id);
    return std::make_pair(WireNodeId(item.first()->id()), WireNodeId(item.second()->id()));
}

int CircuitSession::node_width(WireNodeId id) const
{
    return node(id).width();
}

std::string CircuitSession::node_label(WireNodeId id) const
{
    return node(id).label();
}

WireNodeId CircuitSession::add_node(int width, const std::string & label)
{
    WireNodeId result;
    modify([&]() { result = WireNodeId(controller().create_wire_node(circuit(), width, label).id()); });
    return result;
}

WireEdgeId CircuitSession::add_edge(WireNodeId first, WireNodeId second)
{
    WireEdgeId result;
    modify([&]() { result = WireEdgeId(controller().create_wire_edge(circuit(), node(first), node(second)).id()); });
    return result;
}

void CircuitSession::remove_node(WireNodeId id)
{
    modify([&]() { controller().remove_wire_node(circuit(), node(id)); });
}

void CircuitSession::remove_edge(WireEdgeId id)
{
    modify([&]() { controller().remove_wire_edge(circuit(), edge(id)); });
}

void CircuitSession::attach(PortId port_id, WireNodeId node_id)
{
    modify([&]() { controller().connect(circuit(), port(port_id), node(node_id)); });
}

void CircuitSession::disconnect(PortId id)
{
    modify([&]() { controller().disconnect(circuit(), port(id)); });
}

void CircuitSession::connect(PortId first, PortId second)
{
    modify([&]() { controller().connect(circuit(), port(first), port(second)); });
}

void CircuitSession::batch(const std::function< void(CircuitSession &) > & operation)
{
    modify([&]()
    {
        circuit();
        controller().batch([&]() { operation(*this); });
    });
}

RuntimePath CircuitSession::instance() const
{
    Circuit & blueprint = circuit();
    ProjectController & owner = controller();
    owner.require_runtime_idle();
    RuntimePath result = explicit_instance_ ? instance_ : owner.default_instance(circuit_id_);
    if (&owner.resolve_instance(result).circuit() != &blueprint)
    {
        throw std::invalid_argument("Runtime instance does not match this circuit");
    }
    return result;
}

CircuitState & CircuitSession::state() const
{
    return controller().resolve_instance(instance());
}

CircuitSession CircuitSession::child(ComponentId placement) const
{
    Subcircuit * item = dynamic_cast< Subcircuit * >(&component(placement));
    if (item == nullptr)
    {
        throw std::invalid_argument("Component is not a subcircuit placement");
    }
    RuntimePath path = instance();
    path.placements.push_back(component_id(item->id()));
    return controller().open(item->circuit()->id(), path);
}

void CircuitSession::set(ComponentId input, const LogicVector & value)
{
    modify([&]()
    {
        Pin & target = pin(input);
        if (target.type() != INPUT_PIN)
        {
            throw std::logic_error("Cannot set an output pin");
        }
        if (target.width() != value.width())
        {
            throw std::invalid_argument("Pin value width does not match pin width");
        }
        controller().set(state(), target, value);
    });
}

void CircuitSession::set(ComponentId input, LogicValue value)
{
    modify([&]() { set(input, LogicVector(pin(input).width(), value)); });
}

void CircuitSession::toggle(ComponentId input)
{
    modify([&]()
    {
        Pin & target = pin(input);
        if (target.type() != INPUT_PIN)
        {
            throw std::logic_error("Cannot toggle an output pin");
        }
        controller().toggle(state(), target);
    });
}

bool CircuitSession::run()
{
    bool result = false;
    modify([&]()
    {
        state();
        result = controller().run();
    });
    return result;
}

LogicVector CircuitSession::value(ComponentId id) const
{
    return pin(id).value(state());
}

LogicVector CircuitSession::value(PortId id) const
{
    return state().value(port(id));
}

LogicVector CircuitSession::value(WireNodeId id) const
{
    const Net * net = node(id).net();
    if (net == nullptr)
    {
        throw std::logic_error("Wire node has no associated electrical net");
    }
    return state().value(*net);
}

std::vector< SessionDiagnostic > CircuitSession::diagnostics(bool recursive) const
{
    RuntimePath selected = instance();
    CircuitState & selected_state = controller().resolve_instance(selected);
    std::vector< SessionDiagnostic > result;
    for (const CircuitDiagnostic & fault : selected_state.diagnostics(recursive))
    {
        RuntimePath path = selected;
        std::vector< ComponentId > relative_path;
        const CircuitState * current = fault.state;
        while (current != &selected_state)
        {
            relative_path.push_back(ComponentId(current->placement()->id(), current->parent()->circuit().id(), lifetime_));
            current = current->parent();
        }
        path.placements.insert(path.placements.end(), relative_path.rbegin(), relative_path.rend());
        std::vector< WireNodeId > nodes;
        for (const WireNode * node : fault.net->nodes())
        {
            nodes.push_back(WireNodeId(node->id()));
        }
        result.push_back({fault.kind, fault.state->circuit().id(), path, nodes, fault.bits, fault.message});
    }
    return result;
}
