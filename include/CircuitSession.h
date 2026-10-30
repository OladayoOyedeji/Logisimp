/**
 * @file CircuitSession.h
 * @brief ID-based editing and runtime access for one project circuit.
 */

#ifndef CIRCUIT_SESSION_H
#define CIRCUIT_SESSION_H

#include "Handles.h"
#include "CircuitState.h"
#include "LogicVector.h"
#include "Pin.h"
#include "Port.h"
#include "RuntimePath.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Circuit;
class CircuitState;
class Component;
class Port;
class ProjectController;
class WireNode;
class WireEdge;

enum class ComponentKind
{
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

/**
 * @brief Value snapshot of a fault, using instance and wire IDs for inspection.
 * @note Requery after edits or simulation. Stored IDs can become stale, but no
 * runtime or derived-net pointers are retained in the snapshot.
 */
struct SessionDiagnostic
{
    DiagnosticKind kind;
    CircuitId circuit;
    RuntimePath instance;
    std::vector< WireNodeId > nodes;
    std::vector< int > bits;
    std::string message;
};

/**
 * @brief Copyable, non-owning view of a blueprint and optionally one instance.
 *
 * Opening/copying/destroying a session never starts or stops simulation. Runtime
 * access lazily uses the controller's shared default root unless explicitly
 * bound. child() binds a particular nested placement. run() drains the project's
 * shared queue. Sessions detect controller destruction; deleted circuits,
 * components, ports and explicit instances throw instead of rebinding.
 *
 * Session-issued component IDs include circuit and controller provenance.
 * Explicit integer component IDs remain contextual legacy lookups; wire IDs
 * remain scoped by the session. Enumerations return value snapshots; GUI
 * selection and layout belong to the caller. Use on the
 * controller's thread. All mutations delegate to its transaction machinery.
 */
class CircuitSession
{
public:
    CircuitId circuit_id() const { return circuit_id_; }
    /** @brief Checks the borrowed controller lifetime without resolving this circuit. */
    bool controller_alive() const noexcept { return !lifetime_.expired(); }
    /** @brief Returns the selected circuit's current descriptive name. */
    std::string circuit_name() const;
    /** @brief Renames this circuit through the controller's metadata transaction. */
    void set_circuit_name(const std::string & name);

    std::vector< ComponentId > components() const;
    std::vector< WireNodeId > nodes() const;
    std::vector< WireEdgeId > edges() const;
    ComponentKind kind(ComponentId id) const;
    bool contains(ComponentId id) const;
    /** @brief Checks that a live component handle was issued for this controller/circuit. */
    bool contains_scoped(ComponentId id) const;
    /** @brief Returns the blueprint referenced by a subcircuit placement. */
    CircuitId subcircuit_blueprint(ComponentId placement) const;
    std::string label(ComponentId id) const;
    int pin_width(ComponentId id) const;
    PinType pin_type(ComponentId id) const;

    /** @brief Creates a pin (size = width) or gate (size = input count). */
    ComponentId create(ComponentKind kind);
    ComponentId create(ComponentKind kind, int size);
    ComponentId create_subcircuit(CircuitId blueprint);
    ComponentId duplicate_component(ComponentId id);
    void remove_component(ComponentId id);
    void set_pin_width(ComponentId id, int width);
    void set_pin_type(ComponentId id, PinType type);
    void set_label(ComponentId id, const std::string & label);
    void move_pin(ComponentId id, int index);
    void clear();

    std::vector< PortId > inputs(ComponentId id) const;
    std::vector< PortId > outputs(ComponentId id) const;
    PortId input_port(ComponentId id, int index = 0) const;
    PortId output_port(ComponentId id, int index = 0) const;
    /** @brief Resolves a placement endpoint by child interface pin ID, not order. */
    PortId interface_port(ComponentId placement, ComponentId interface_pin) const;
    /** @brief Reads endpoint metadata, rejecting stale or foreign port handles. */
    int port_width(PortId id) const;
    PortDirection port_direction(PortId id) const;
    std::string port_label(PortId id) const;
    /** @brief Returns the attachment, or WireNodeId(-1) for a valid detached port. */
    WireNodeId attached_node(PortId id) const;
    /** @brief Returns the two endpoints of a live edge in this session's circuit. */
    std::pair< WireNodeId, WireNodeId > edge_endpoints(WireEdgeId id) const;
    /** @brief Reads the node's configured width constraint; zero is unconstrained. */
    int node_width(WireNodeId id) const;
    std::string node_label(WireNodeId id) const;
    /** @brief Creates a node with an optional width constraint and descriptive label. */
    WireNodeId add_node(int width = 0, const std::string & label = "");
    WireEdgeId add_edge(WireNodeId first, WireNodeId second);
    void remove_node(WireNodeId id);
    void remove_edge(WireEdgeId id);
    void attach(PortId port, WireNodeId node);
    void disconnect(PortId port);
    /** @brief Atomically joins endpoints, retaining their existing branches. */
    void connect(PortId first, PortId second);

    /** @brief Runs related edits as one transaction; any failed mutation aborts it. */
    void batch(const std::function< void(CircuitSession &) > & operation);

    RuntimePath instance() const;
    CircuitSession child(ComponentId placement) const;
    void set(ComponentId input, const LogicVector & value);
    void set(ComponentId input, LogicValue value);
    void toggle(ComponentId input);
    bool run();
    LogicVector value(ComponentId pin) const;
    LogicVector value(PortId port) const;
    /**
     * @brief Reads this node's current net value in the session's runtime instance.
     * @throws std::invalid_argument If the node ID is missing or deleted.
     * @throws std::logic_error If the node has no associated electrical net.
     * @note Resolves the net on every call. Bare geometry has no invented value
     * and does not start a runtime. Wire IDs are contextual to this session.
     */
    LogicVector value(WireNodeId id) const;
    std::vector< SessionDiagnostic > diagnostics(bool recursive = true) const;

private:
    friend class ProjectController;
    CircuitSession(ProjectController & controller, CircuitId circuit);
    CircuitSession(ProjectController & controller, CircuitId circuit, const RuntimePath & instance);
    ProjectController & controller() const;
    Circuit & circuit() const;
    Component & component(ComponentId id) const;
    ComponentId component_id(int value) const;
    PortId port_id(const Port & port) const;
    Pin & pin(ComponentId id) const;
    Port & port(PortId id) const;
    WireNode & node(WireNodeId id) const;
    WireEdge & edge(WireEdgeId id) const;
    CircuitState & state() const;
    void modify(const std::function< void() > & operation);

    ProjectController * controller_;
    std::weak_ptr< int > lifetime_;
    CircuitId circuit_id_;
    bool explicit_instance_;
    RuntimePath instance_;
};

#endif // CIRCUIT_SESSION_H
