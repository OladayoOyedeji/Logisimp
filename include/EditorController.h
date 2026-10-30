/** @file EditorController.h @brief Framework-independent interaction for one editor view. */

#ifndef EDITOR_CONTROLLER_H
#define EDITOR_CONTROLLER_H

#include "CircuitLayout.h"
#include "CircuitSession.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

class ProjectController;

enum class EditorTool
{
    Select,
    Interact,
    Wire,
    InputPin,
    OutputPin,
    AndGate,
    OrGate,
    NotGate,
    NandGate,
    NorGate,
    XorGate,
    XnorGate,
    Subcircuit
};

/** @brief Fresh blueprint information for a properties panel, with no borrowed objects. */
struct ComponentProperties
{
    ComponentId component;
    ComponentKind kind;
    std::string label;
    std::vector< PortId > inputs;
    std::vector< PortId > outputs;
    int pin_width = -1;
    PinType pin_type = INVALID_PIN_TYPE;
    CircuitId subcircuit;
    bool has_position = false;
    CanvasPosition position;
};

/**
 * @brief Interprets forwarded events without polling, hit testing or rendering.
 *
 * The project and its shared layout must outlive this view. All calls use the
 * controller's thread. Session-issued component handles are required: raw local
 * integer IDs cannot safely identify objects selected by another view.
 *
 * Opening, switching or closing a view never starts/stops simulation. Runtime
 * operations use an explicit binding, the only live occurrence, or a shared
 * default root when none exists. Multiple occurrences require bind_instance().
 * run_simulation() drains the project's shared queue, including other circuits.
 *
 * Creation, duplication and deletion own a complete backend transaction before
 * publishing layout/selection changes; they cannot be nested in another batch.
 * Movement changes layout only and requires the project operation to be idle.
 */
class EditorController
{
public:
    EditorController(ProjectController & controller, CircuitId circuit, CircuitLayout & layout);
    EditorController(ProjectController & controller, CircuitId circuit, CircuitLayout & layout, const RuntimePath & instance);

    CircuitId circuit_id() const { return session_.circuit_id(); }
    EditorTool active_tool() const { return active_tool_; }
    void select_tool(EditorTool tool);
    /** @brief Chooses the blueprint for the Subcircuit tool; creates nothing. */
    void set_subcircuit_blueprint(CircuitId circuit);

    /** @brief Places with default size; returns ComponentId(-1) when no object is created. */
    ComponentId on_canvas_click(CanvasPosition position);
    /** @brief Size is pin width or gate input count; ignored only for Subcircuit. */
    ComponentId on_canvas_click(CanvasPosition position, int size);
    /** @brief Dispatches a hit-tested click for Select or Interact. */
    void on_component_click(ComponentId component);

    /** @brief Creates and selects a positioned component without changing the tool. */
    ComponentId add_component(ComponentKind kind, CanvasPosition position);
    /** @brief Size is pin width or gate input count; Subcircuit uses the chosen blueprint. */
    ComponentId add_component(ComponentKind kind, CanvasPosition position, int size);
    /** @brief Deletes a known component and its position, clearing only affected interaction state. */
    void delete_component(ComponentId component);
    /** @brief Updates layout only; electrical connections and simulation values are retained. */
    void move_component(ComponentId component, CanvasPosition position);
    /** @brief Returns false without changing position when a live component has no position. */
    bool component_position(ComponentId component, CanvasPosition & position) const;
    /** @brief Creates and selects a positioned copy with disconnected ports; preserves the tool. */
    ComponentId duplicate_component(ComponentId component, CanvasPosition position);
    void set_label(ComponentId component, const std::string & label);
    void set_pin_width(ComponentId component, int width);
    void set_pin_type(ComponentId component, PinType type);
    ComponentProperties properties(ComponentId component) const;

    void select_component(ComponentId component);
    void clear_selection() { selected_component_ = ComponentId(); }
    /** @brief Clears stale selections; ComponentId(-1) means no selected object. */
    ComponentId selected_component();
    ComponentProperties selected_properties();
    void set_selected_pin_width(int width);
    void set_selected_pin_type(PinType type);
    void set_selected_label(const std::string & label);
    /** @brief Returns the removed ID; no selection is an invalid_argument error. */
    ComponentId delete_selected();

    /** @brief Wire events accept hit-tested endpoints; no graphical routing occurs. */
    void connect_ports(PortId first, PortId second);
    void disconnect_port(PortId port);
    std::vector< PortId > inputs(ComponentId component) const;
    std::vector< PortId > outputs(ComponentId component) const;
    PortId input_port(ComponentId component, int index = 0) const;
    PortId output_port(ComponentId component, int index = 0) const;
    /** @brief Removes one electrical edge while retaining its endpoints and other branches. */
    void delete_wire(WireEdgeId edge);
    std::vector< ComponentId > components() const;
    std::vector< WireNodeId > nodes() const;
    std::vector< WireEdgeId > edges() const;
    WireNodeId attached_node(PortId port) const;
    std::pair< WireNodeId, WireNodeId > edge_endpoints(WireEdgeId edge) const;
    /** @brief Returns false without changing position when a live node has no position. */
    bool node_position(WireNodeId node, CanvasPosition & position) const;

    void begin_wire(PortId port);
    void finish_wire(PortId port);
    void cancel_wire() { pending_wire_ = PortId(); }
    /** @brief Returns a validated endpoint, or an invalid PortId when no wire is pending. */
    PortId pending_wire();
    /** @brief Prunes stale interaction/layout records; rejected during a project operation. */
    void refresh();

    void toggle_input(ComponentId component);
    void set_input(ComponentId component, const LogicVector & value);
    void set_input(ComponentId component, LogicValue value);
    bool run_simulation();
    LogicVector value(ComponentId component) const;
    LogicVector value(PortId port) const;
    /** @brief Reads the node's current derived net; a node without a net throws. */
    LogicVector value(WireNodeId node) const;
    std::vector< SessionDiagnostic > diagnostics(bool recursive = true) const;

    /** @brief Switching clears selection/wire gesture, retaining the active tool and layout. */
    void view_circuit(CircuitId circuit);
    void view_circuit(CircuitId circuit, const RuntimePath & instance);
    void bind_instance(const RuntimePath & instance);
    void unbind_instance() { bound_instance_ = RuntimePath(); }
    /** @brief A root ID of -1 means no explicit runtime binding. */
    RuntimePath bound_instance() const { return bound_instance_; }
    /** @brief Resolves runtime choice; may lazily start the first shared default root. */
    RuntimePath runtime_instance() const;

private:
    ProjectController & controller() const;
    CircuitSession runtime_session() const;
    void require_component(ComponentId component) const;
    void require_input(ComponentId component) const;
    bool valid_port(PortId port) const;
    void require_port(PortId port) const;
    ComponentId require_selection();
    ComponentKind placement_kind() const;
    ComponentId create_positioned_component(CanvasPosition position, const std::function< ComponentId(CircuitSession &) > & create);

    ProjectController * controller_;
    CircuitSession session_;
    CircuitLayout * layout_;
    EditorTool active_tool_;
    ComponentId selected_component_;
    CircuitId subcircuit_blueprint_;
    PortId pending_wire_;
    RuntimePath bound_instance_;
};

#endif // EDITOR_CONTROLLER_H
