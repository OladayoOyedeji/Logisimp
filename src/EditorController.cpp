/** @file EditorController.cpp @brief Event interpretation using ID-based sessions. */

#include "EditorController.h"

#include "ProjectController.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace
{
void validate_position(CanvasPosition position)
{
    if (!std::isfinite(position.x) || !std::isfinite(position.y))
    {
        throw std::invalid_argument("Canvas position must contain finite coordinates");
    }
}
}

EditorController::EditorController(ProjectController & controller, CircuitId circuit, CircuitLayout & layout)
    : controller_(&controller), session_(controller.open(circuit)), layout_(&layout), active_tool_(EditorTool::Select)
{}

EditorController::EditorController(ProjectController & controller, CircuitId circuit, CircuitLayout & layout, const RuntimePath & instance)
    : EditorController(controller, circuit, layout)
{
    bind_instance(instance);
}

ProjectController & EditorController::controller() const
{
    if (!session_.controller_alive())
    {
        throw std::logic_error("Editor controller's project has been destroyed");
    }
    return *controller_;
}

void EditorController::select_tool(EditorTool tool)
{
    switch (tool)
    {
    case EditorTool::Select:
    case EditorTool::Interact:
    case EditorTool::Wire:
    case EditorTool::InputPin:
    case EditorTool::OutputPin:
    case EditorTool::AndGate:
    case EditorTool::OrGate:
    case EditorTool::NotGate:
    case EditorTool::NandGate:
    case EditorTool::NorGate:
    case EditorTool::XorGate:
    case EditorTool::XnorGate:
    case EditorTool::Subcircuit:
        break;
    default:
        throw std::invalid_argument("Unknown editor tool");
    }
    if (active_tool_ != tool)
    {
        cancel_wire();
    }
    active_tool_ = tool;
}

void EditorController::set_subcircuit_blueprint(CircuitId circuit)
{
    controller().open(circuit);
    subcircuit_blueprint_ = circuit;
}

ComponentKind EditorController::placement_kind() const
{
    switch (active_tool_)
    {
    case EditorTool::InputPin: return ComponentKind::InputPin;
    case EditorTool::OutputPin: return ComponentKind::OutputPin;
    case EditorTool::AndGate: return ComponentKind::AndGate;
    case EditorTool::OrGate: return ComponentKind::OrGate;
    case EditorTool::NotGate: return ComponentKind::NotGate;
    case EditorTool::NandGate: return ComponentKind::NandGate;
    case EditorTool::NorGate: return ComponentKind::NorGate;
    case EditorTool::XorGate: return ComponentKind::XorGate;
    case EditorTool::XnorGate: return ComponentKind::XnorGate;
    case EditorTool::Subcircuit: return ComponentKind::Subcircuit;
    default: throw std::invalid_argument("Active tool does not place components");
    }
}

ComponentId EditorController::on_canvas_click(CanvasPosition position)
{
    bool single = active_tool_ == EditorTool::InputPin || active_tool_ == EditorTool::OutputPin || active_tool_ == EditorTool::NotGate;
    return on_canvas_click(position, single ? 1 : 2);
}

ComponentId EditorController::on_canvas_click(CanvasPosition position, int size)
{
    validate_position(position);
    if (active_tool_ == EditorTool::Select)
    {
        clear_selection();
        return ComponentId();
    }
    if (active_tool_ == EditorTool::Interact)
    {
        return ComponentId();
    }
    if (active_tool_ == EditorTool::Wire)
    {
        throw std::invalid_argument("Wire tool requires port endpoints through begin_wire/finish_wire");
    }
    return add_component(placement_kind(), position, size);
}

ComponentId EditorController::add_component(ComponentKind kind, CanvasPosition position)
{
    bool single = kind == ComponentKind::InputPin || kind == ComponentKind::OutputPin || kind == ComponentKind::NotGate;
    return add_component(kind, position, single ? 1 : 2);
}

ComponentId EditorController::add_component(ComponentKind kind, CanvasPosition position, int size)
{
    if (kind == ComponentKind::Subcircuit && subcircuit_blueprint_.value < 0)
    {
        throw std::invalid_argument("Choose a subcircuit blueprint before placing it");
    }
    return create_positioned_component(position, [&](CircuitSession & session)
    {
        return kind == ComponentKind::Subcircuit ? session.create_subcircuit(subcircuit_blueprint_) : session.create(kind, size);
    });
}

ComponentId EditorController::create_positioned_component(CanvasPosition position, const std::function< ComponentId(CircuitSession &) > & create)
{
    validate_position(position);
    // Allocation or runtime synchronization can fail. Publish layout and
    // selection only after the complete circuit transaction succeeds.
    CircuitLayout staged_layout = *layout_;
    ComponentId created;
    session_.batch([&](CircuitSession & session)
    {
        created = create(session);
        staged_layout.set_component_position(circuit_id(), created, position);
    });
    layout_->swap(staged_layout);
    selected_component_ = created;
    return created;
}

ComponentId EditorController::duplicate_component(ComponentId component, CanvasPosition position)
{
    require_component(component);
    return create_positioned_component(position, [&](CircuitSession & session) { return session.duplicate_component(component); });
}

void EditorController::move_component(ComponentId component, CanvasPosition position)
{
    if (controller().operation_active())
    {
        throw std::logic_error("Move components after the current project operation has committed");
    }
    require_component(component);
    layout_->set_component_position(circuit_id(), component, position);
}

bool EditorController::component_position(ComponentId component, CanvasPosition & position) const
{
    require_component(component);
    return layout_->component_position(circuit_id(), component, position);
}

void EditorController::require_component(ComponentId component) const
{
    if (!session_.contains_scoped(component))
    {
        throw std::invalid_argument("Expected a live session-issued component ID for this circuit and project");
    }
}

void EditorController::select_component(ComponentId component)
{
    require_component(component);
    selected_component_ = component;
}

ComponentId EditorController::selected_component()
{
    if (!session_.controller_alive())
    {
        clear_selection();
    }
    else if (selected_component_.value >= 0)
    {
        try
        {
            if (!session_.contains_scoped(selected_component_))
            {
                clear_selection();
            }
        }
        catch (const std::invalid_argument &)
        {
            // Deleting the entire viewed blueprint invalidates its selection.
            clear_selection();
        }
    }
    return selected_component_;
}

ComponentId EditorController::require_selection()
{
    ComponentId selected = selected_component();
    if (selected.value < 0)
    {
        throw std::invalid_argument("No live component is selected in this editor");
    }
    return selected;
}

ComponentProperties EditorController::selected_properties()
{
    return properties(require_selection());
}

ComponentProperties EditorController::properties(ComponentId component) const
{
    require_component(component);
    ComponentProperties result;
    result.component = component;
    result.kind = session_.kind(component);
    result.label = session_.label(component);
    result.inputs = session_.inputs(component);
    result.outputs = session_.outputs(component);
    result.has_position = layout_->component_position(circuit_id(), component, result.position);
    if (result.kind == ComponentKind::InputPin || result.kind == ComponentKind::OutputPin)
    {
        result.pin_width = session_.pin_width(component);
        result.pin_type = session_.pin_type(component);
    }
    if (result.kind == ComponentKind::Subcircuit)
    {
        result.subcircuit = session_.subcircuit_blueprint(component);
    }
    return result;
}

void EditorController::set_selected_pin_width(int width)
{
    set_pin_width(require_selection(), width);
}

void EditorController::set_selected_pin_type(PinType type)
{
    set_pin_type(require_selection(), type);
}

void EditorController::set_selected_label(const std::string & label)
{
    set_label(require_selection(), label);
}

void EditorController::set_label(ComponentId component, const std::string & label)
{
    require_component(component);
    session_.set_label(component, label);
}

void EditorController::set_pin_width(ComponentId component, int width)
{
    require_component(component);
    session_.set_pin_width(component, width);
}

void EditorController::set_pin_type(ComponentId component, PinType type)
{
    require_component(component);
    session_.set_pin_type(component, type);
}

ComponentId EditorController::delete_selected()
{
    ComponentId selected = require_selection();
    delete_component(selected);
    return selected;
}

void EditorController::delete_component(ComponentId component)
{
    require_component(component);
    CircuitLayout staged_layout = *layout_;
    staged_layout.remove_component(circuit_id(), component);
    session_.batch([&](CircuitSession & session) { session.remove_component(component); });
    layout_->swap(staged_layout);
    if (selected_component_ == component)
    {
        clear_selection();
    }
    if (pending_wire_.value >= 0 && pending_wire_.component == component)
    {
        cancel_wire();
    }
}

std::vector< ComponentId > EditorController::components() const
{
    return session_.components();
}

std::vector< WireNodeId > EditorController::nodes() const
{
    return session_.nodes();
}

std::vector< WireEdgeId > EditorController::edges() const
{
    return session_.edges();
}

std::vector< PortId > EditorController::inputs(ComponentId component) const
{
    require_component(component);
    return session_.inputs(component);
}

std::vector< PortId > EditorController::outputs(ComponentId component) const
{
    require_component(component);
    return session_.outputs(component);
}

PortId EditorController::input_port(ComponentId component, int index) const
{
    require_component(component);
    return session_.input_port(component, index);
}

PortId EditorController::output_port(ComponentId component, int index) const
{
    require_component(component);
    return session_.output_port(component, index);
}

WireNodeId EditorController::attached_node(PortId port) const
{
    require_port(port);
    return session_.attached_node(port);
}

std::pair< WireNodeId, WireNodeId > EditorController::edge_endpoints(WireEdgeId edge) const
{
    return session_.edge_endpoints(edge);
}

bool EditorController::node_position(WireNodeId node, CanvasPosition & position) const
{
    session_.node_width(node);
    return layout_->node_position(circuit_id(), node, position);
}

void EditorController::connect_ports(PortId first, PortId second)
{
    require_port(first);
    require_port(second);
    session_.connect(first, second);
}

void EditorController::disconnect_port(PortId port)
{
    require_port(port);
    session_.disconnect(port);
}

void EditorController::delete_wire(WireEdgeId edge)
{
    session_.remove_edge(edge);
}

bool EditorController::valid_port(PortId port) const
{
    if (!session_.controller_alive())
    {
        return false;
    }
    try
    {
        if (!session_.contains_scoped(port.component))
        {
            return false;
        }
        auto inputs = session_.inputs(port.component);
        auto outputs = session_.outputs(port.component);
        return std::find(inputs.begin(), inputs.end(), port) != inputs.end() || std::find(outputs.begin(), outputs.end(), port) != outputs.end();
    }
    catch (const std::invalid_argument &)
    {
        return false;
    }
}

void EditorController::require_port(PortId port) const
{
    if (!valid_port(port))
    {
        throw std::invalid_argument("Expected a live session-issued port ID for this circuit and project");
    }
}

void EditorController::begin_wire(PortId port)
{
    if (active_tool_ != EditorTool::Wire)
    {
        throw std::invalid_argument("Activate the Wire tool before starting a wire");
    }
    require_port(port);
    pending_wire_ = port;
}

PortId EditorController::pending_wire()
{
    if (pending_wire_.value >= 0 && !valid_port(pending_wire_))
    {
        cancel_wire();
    }
    return pending_wire_;
}

void EditorController::finish_wire(PortId port)
{
    if (active_tool_ != EditorTool::Wire || pending_wire().value < 0)
    {
        throw std::invalid_argument("Start a wire from a live port before finishing it");
    }
    connect_ports(pending_wire_, port);
    cancel_wire();
}

void EditorController::refresh()
{
    if (!session_.controller_alive())
    {
        clear_selection();
        cancel_wire();
        return;
    }
    if (controller().operation_active())
    {
        throw std::logic_error("Refresh layout after the current project operation has committed");
    }
    selected_component();
    pending_wire();
    std::vector< ComponentId > components;
    std::vector< WireNodeId > nodes;
    try
    {
        components = session_.components();
        nodes = session_.nodes();
    }
    catch (const std::invalid_argument &)
    {
        layout_->clear(circuit_id());
        return;
    }
    for (const ComponentLayout & record : layout_->components(circuit_id()))
    {
        if (std::find(components.begin(), components.end(), record.component) == components.end())
        {
            layout_->remove_component(circuit_id(), record.component);
        }
    }
    for (const WireNodeLayout & record : layout_->nodes(circuit_id()))
    {
        if (std::find(nodes.begin(), nodes.end(), record.node) == nodes.end())
        {
            layout_->remove_node(circuit_id(), record.node);
        }
    }
}

void EditorController::require_input(ComponentId component) const
{
    require_component(component);
    if (session_.kind(component) != ComponentKind::InputPin)
    {
        throw std::invalid_argument("Simulation input interaction requires an input pin");
    }
}

void EditorController::on_component_click(ComponentId component)
{
    if (active_tool_ == EditorTool::Select)
    {
        select_component(component);
    }
    else if (active_tool_ == EditorTool::Interact)
    {
        toggle_input(component);
    }
    else
    {
        throw std::invalid_argument("Use a canvas placement event or port wire event for this tool");
    }
}

CircuitSession EditorController::runtime_session() const
{
    ProjectController & project = controller();
    if (bound_instance_.root.value >= 0)
    {
        return project.open(circuit_id(), bound_instance_);
    }
    auto instances = project.instances(circuit_id());
    if (instances.size() > 1)
    {
        throw std::logic_error("This circuit has multiple runtime instances; bind an explicit RuntimePath before simulation interaction");
    }
    if (instances.size() == 1)
    {
        return project.open(circuit_id(), instances.front());
    }
    return session_;
}

RuntimePath EditorController::runtime_instance() const
{
    return runtime_session().instance();
}

void EditorController::toggle_input(ComponentId component)
{
    require_input(component);
    runtime_session().toggle(component);
}

void EditorController::set_input(ComponentId component, const LogicVector & value)
{
    require_input(component);
    runtime_session().set(component, value);
}

void EditorController::set_input(ComponentId component, LogicValue value)
{
    require_input(component);
    runtime_session().set(component, value);
}

bool EditorController::run_simulation()
{
    return runtime_session().run();
}

LogicVector EditorController::value(ComponentId component) const
{
    require_component(component);
    ComponentKind kind = session_.kind(component);
    if (kind != ComponentKind::InputPin && kind != ComponentKind::OutputPin)
    {
        throw std::invalid_argument("Read a pin or a component port to inspect runtime values");
    }
    return runtime_session().value(component);
}

LogicVector EditorController::value(PortId port) const
{
    require_port(port);
    return runtime_session().value(port);
}

LogicVector EditorController::value(WireNodeId node) const
{
    session_.node_width(node);
    return runtime_session().value(node);
}

std::vector< SessionDiagnostic > EditorController::diagnostics(bool recursive) const
{
    return runtime_session().diagnostics(recursive);
}

void EditorController::view_circuit(CircuitId circuit)
{
    CircuitSession next = controller().open(circuit);
    session_ = std::move(next);
    unbind_instance();
    clear_selection();
    cancel_wire();
}

void EditorController::view_circuit(CircuitId circuit, const RuntimePath & instance)
{
    // Validate and allocate the new path before changing the viewed blueprint.
    ProjectController & project = controller();
    project.open(circuit, instance);
    CircuitSession next = project.open(circuit);
    RuntimePath binding(instance);
    session_ = std::move(next);
    bound_instance_ = std::move(binding);
    clear_selection();
    cancel_wire();
}

void EditorController::bind_instance(const RuntimePath & instance)
{
    controller().open(circuit_id(), instance);
    RuntimePath binding(instance);
    bound_instance_ = std::move(binding);
}
