@page gui_api_reference GUI API reference

Use one ProjectController as the owner of an open project. Keep it alive while
its editor tabs and simulation views are open. Perform structural edits and
input changes through that controller; query the returned objects to draw them.
The examples below use the current untimed simulator.

## Objects the GUI works with

| Object | Meaning for the GUI | Ownership and values |
| --- | --- | --- |
| ProjectController | Application entry point for creation, editing, and simulation. | Owns the project, simulator, and root runtime hierarchies. |
| Circuit | Reusable blueprint displayed in an editor tab. | Owns components, wire nodes, edges, and derived nets. It contains structure rather than one simulation's values. |
| Component / Gate / Pin | Items placed in a blueprint. A Pin defines the blueprint's external interface. | Circuit owns components; each component owns its ports. Primitive gates are currently single-bit. |
| Subcircuit | One component placing a reusable Circuit inside a parent Circuit. | Borrows its child blueprint. Editing the blueprint updates all its placements. |
| CircuitState | One simulation instance of a blueprint. | The controller owns roots; each SubcircuitState owns its nested child. Different instances have independent values. |
| Port | A component's input or output connection endpoint. | Attach it to a WireNode. A port can have one node attachment. |
| WireNode / WireEdge | An endpoint or junction, and an undirected segment between two nodes. | Circuit owns both. The GUI stores drawing coordinates separately. |
| Net | Electrical region derived from connected nodes and their attached ports. | Describes connectivity; its value belongs to a CircuitState. Reacquire Net pointers after connectivity edits. |

## Place, inspect, and edit components

Table snippets assume an existing `ProjectController controller` and a
`Circuit & circuit` owned by it. Other argument names refer to existing objects
in that circuit unless the row says otherwise. Include `ProjectController.h`,
`CircuitState.h`, and `Gates.h` when using their definitions.

| GUI question | API or code | Behavior to account for |
| --- | --- | --- |
| How do I create an editor's circuit? | `Circuit & circuit = controller.create_circuit();` | Creates an empty blueprint without starting simulation. |
| How do I list circuit tabs or library entries? | `controller.circuits()` | Returns borrowed circuit pointers. The API currently has no circuit-name or persistent circuit-ID field; keep display metadata in the GUI. |
| How do I place a two-input AND gate? | `AndGate & gate = controller.create_and_gate(circuit);` | Creates disconnected ports. The GUI chooses its screen position. |
| How do I choose the number of inputs? | `controller.create_and_gate(circuit, 3)` | Input count must be positive. OR, NAND, NOR, XOR, and XNOR factories take the same optional count. |
| How do I place other primitive gates? | `create_or_gate`, `create_not_gate`, `create_nand_gate`, `create_nor_gate`, `create_xor_gate`, `create_xnor_gate` | Call these on `controller` with `circuit`; NOT always has one input. |
| How do I get connection handles? | `gate.input_port(0)`; `gate.output_port()` | Gate outputs accept omitted index or index `0`; other indices throw. Other components, including Subcircuit, require an explicit output index. |
| How do I draw existing components and handles? | `circuit.components()`; `component.inputs()`; `component.outputs()` | Returned pointers are borrowed. Use `input_count()` / `output_count()` when indexing; invalid indices throw. |
| How do I label an ordinary gate or placement? | `gate.label() = "Enable";` | Descriptive metadata. Use `controller.set_pin_label(...)` for interface pins so placement-port labels synchronize. |
| How do I duplicate a selected component? | `Component & duplicate = controller.copy_component(circuit, component);` | Copies a supported concrete type, label, and configuration with disconnected ports. Copying a Subcircuit shares its referenced blueprint. The source must belong to this controller's project. |
| How do I duplicate a complete blueprint? | `Circuit & duplicate = controller.copy_circuit(circuit);` | Copies components, interface ordering, nodes, edges, and attachments into a new blueprint. Child blueprints remain shared; runtime values are not copied. |
| How do I delete a gate or other component? | `controller.remove_component(circuit, component);` | Detaches and destroys it and its component-specific state. Wire nodes and segments survive. Surviving runtime states remain alive. |
| How do I clear a tab? | `controller.clear_circuit(circuit);` | Retains the empty blueprint and its own live CircuitState objects. Deletes its contents and updates every external placement to the empty interface. |

## Custom circuits and shared subcircuits

| GUI question | API or code | Behavior to account for |
| --- | --- | --- |
| How do I create a reusable custom circuit? | `Circuit & custom_circuit = controller.create_circuit();` | Add interface pins, gates, and wiring to this blueprint as for any other circuit. See the runnable examples below and `src/main.cpp`. |
| How do I expose a custom input? | `Pin & circuit_input = controller.create_pin(custom_circuit, INPUT_PIN);` | Inside the blueprint, use `circuit_input.output_port(0)` to drive its internal wiring. A placement exposes this pin as an input port. |
| How do I expose a custom output? | `Pin & circuit_output = controller.create_pin(custom_circuit, OUTPUT_PIN);` | Inside the blueprint, attach internal wiring to `circuit_output.input_port(0)`. A placement exposes this pin as an output port. |
| How do I place the custom circuit? | `Subcircuit & placement = controller.create_subcircuit(parent, custom_circuit);` | Both blueprints must belong to this controller. Direct and indirect recursive placement are rejected. Include `Subcircuit.h` to use placement methods. |
| How do I wire a placement? | `placement.input_port(0)`; `placement.output_port(0)` | Connect those ports to nodes in `parent`. Port order follows the child blueprint's input/output pin lists. |
| How do I reuse it elsewhere? | `controller.create_subcircuit(other_parent, custom_circuit)` | Multiple placements and parent circuits share the structure while each live placement has independent runtime values. |
| How do I edit all uses of a custom circuit? | Edit `custom_circuit` through `controller`. | Updates every live instance, including nested ones. Surviving child CircuitState addresses remain stable. |
| How do I change the interface label or order? | `controller.set_pin_label(custom_circuit, pin, "Data");`; `controller.move_pin(custom_circuit, pin, 0);` | Synchronizes placement ports. Reordering within the same input/output list retains compatible ports and their connections; refresh the GUI's port indices. |
| How do I change width or direction? | `controller.set_pin_width(custom_circuit, pin, 8);`; `controller.set_pin_type(custom_circuit, pin, OUTPUT_PIN);` | Width must be positive. A real width/type change resets that pin's runtime state. Width changes detach incompatible connections; direction changes replace ports and detach old connections, including at placements. Refresh port handles. |
| How do I remove an exposed pin? | `controller.remove_component(custom_circuit, pin);` | Removes the corresponding port from every placement and disconnects that port. Other interface pins survive. |
| How do I delete just one placed subcircuit? | `controller.remove_component(parent, placement);` | Deletes that component from the parent blueprint, and its child runtime subtree from every live instance of that parent. Other placements and the shared child blueprint survive. |
| How do I delete the reusable blueprint itself? | Remove every referencing placement; stop every remaining root of that blueprint; call `controller.remove_circuit(custom_circuit);`. | The controller rejects deletion while the blueprint still has live instances or registered placements. Parent roots can stay running when their placements are removed through the controller. See the second example. |

## Draw wires and make electrical connections

A visual segment is a WireEdge between two WireNodes. A port attaches to a
node; ports and edges do not attach directly to each other. Multiple ports may
attach to the same node. Matching labels and overlapping screen coordinates
do not create electrical connections.

| GUI question | API or code | Behavior to account for |
| --- | --- | --- |
| How do I create an endpoint or junction? | `WireNode & node = controller.create_wire_node(circuit);` | Creates a node with width constraint `0`, meaning unconstrained. This does not start a wire from a selected port automatically. |
| How do I create an explicitly sized bus node? | `controller.create_wire_node(circuit, 8, "Data")` | Nonzero node widths must agree throughout a connected region and match attached port widths. `node.width()` is the stored constraint; `node.net()->width()` is the resolved net width when a net exists. |
| How do I draw a segment? | `WireEdge & edge = controller.create_wire_edge(circuit, first, second);` | Both endpoints must be distinct nodes of this circuit. Duplicate undirected edges are rejected. Endpoint order does not specify signal direction. |
| How do I attach a gate's output? | `controller.connect(circuit, *gate.output_port(), node);` | Requires an unattached port. Creates no nodes or edges. Explicit `gate.output_port(0)` is equivalent. |
| How do I attach an input to the same wire? | `controller.connect(circuit, *listener_gate.input_port(0), node);` | Use the appropriate listener gate and node. Ports may share one node or use separate nodes connected by edges. |
| How do I connect two existing wires? | `controller.create_wire_edge(circuit, first_wire_node, second_wire_node);` | Joins their electrical regions. Rejects conflicting widths or more than one OUTPUT driver, even when both drivers currently have the same value. |
| How do I make a branch? | Create another node, then `controller.create_wire_edge(circuit, junction, branch_node);`. | Reuse an existing junction node. Add a port attachment at either endpoint as needed. |
| How do I branch from the middle of an existing segment? | Save `edge.first()` and `edge.second()`; remove the old edge; create a junction; connect both saved endpoints and the new branch to it. | There is no automatic segment-splitting API. These are separate edit transactions; run simulation after completing the sequence. A failure in a later call does not undo earlier successful calls. |
| Do crossing segments connect automatically? | No model call is made for a purely visual crossing. | To create an electrical junction, explicitly use a shared node and edges. Coordinate-based hit testing belongs to the GUI. |
| How do I move a port to another wire? | `controller.disconnect(circuit, port);`, then `controller.connect(circuit, port, other_node);` | Connecting an already attached port, including to the same node, throws. The two calls are separate transactions; if the second fails, the port remains detached. |
| How do I delete a segment? | `controller.remove_wire_edge(circuit, edge);` | Endpoints survive. An alternate graph path may keep them electrically connected. Fragments without a driver become UNKNOWN. |
| How do I delete a junction? | `controller.remove_wire_node(circuit, node);` | Deletes incident edges and detaches attached ports. Neighboring nodes are not automatically joined; components survive. |
| How do I duplicate wire geometry? | `controller.copy_wire_node(circuit, source_node)` | Copies one node's width constraint and label without edges or ports. Recreate selected edges explicitly between the new endpoints. |
| How do I redraw existing geometry? | `circuit.wire_nodes()`; `circuit.wire_edges()`; `edge.first()`; `edge.second()` | Keep positions and routing metadata in GUI storage. Component/node/edge IDs are local to their circuit and object category, rather than global project identifiers. |
| How do I color a wire using its signal? | Check `node.net() != nullptr`, then read `state.value(*node.net())`. | Use the CircuitState for the displayed instance of that circuit. A region without attached ports has no Net; never dereference a null net pointer. |

## Run simulation and inspect nested instances

| GUI question | API or code | Behavior to account for |
| --- | --- | --- |
| How do I start simulation? | `CircuitState & state = controller.start(circuit);` | Creates a root hierarchy, schedules it and other roots, and returns a borrowed reference. Call `controller.run()` to evaluate. |
| How do I simulate the same circuit twice? | Call `controller.start(circuit)` again. | Creates another root with independent values. Choose which CircuitState each simulation view displays. |
| How do I set a circuit input? | `controller.set(state, circuit_input, LogicVector(1, HIGH));` | Pin must be an INPUT_PIN of `state.circuit()` and the value width must match. Schedules propagation but does not run the queue. For a bus, construct the desired-width LogicVector and assign its bits. |
| How do I toggle an input? | `controller.toggle(state, circuit_input);` | Toggles all bits. Use `set()` for a chosen multi-bit pattern. |
| How do I update the displayed outputs? | Check `controller.run()`, then read `circuit_output.value(state)`. | `run()` evaluates the shared queue for all roots. `false` means the current feedback did not settle within the work limit; do not report it as a settled result. |
| How do I open a placement's internal simulation view? | Obtain `SubcircuitState` from `parent_state.component_state(placement)`, then use `child_state()`. | Include `SubcircuitState.h`; see the cast in the example below. This selects one placement in one parent instance. It does not start a separate root. |
| Should I set a nested input directly? | Normally set the containing circuit's input and run. | Parent wiring supplies a placement's inputs. A directly set child input can be overwritten when the parent placement next evaluates. |
| Can I edit while simulation exists? | Call the usual controller edit method, then `controller.run()`. | Existing states survive compatible edits. Each successful edit reschedules all roots; use one thread and do not edit reentrantly during evaluation. |
| How do I pause processing? | Defer calls to `controller.run()`. | There is no background simulation thread or pause API. `stop()` destroys a root rather than pausing it. |
| How do I close a simulation view? | `controller.stop(state);` | Only pass an owned root, not a nested child. It removes that hierarchy's events and destroys it while preserving other roots' queued work. |
| How do I close the whole project? | Let its ProjectController be destroyed. | It clears events, destroys all remaining runtime hierarchies, and then releases blueprints. Do not delete returned objects yourself. |

## Handle edits, selection, and errors

| Situation | GUI responsibility |
| --- | --- |
| A controller edit throws. | Report the error. That individual edit restores the previous structure, values, and event queue. Most invalid ownership/connections throw `std::invalid_argument`; invalid indices throw `std::out_of_range`; lifetime/reentrancy violations throw `std::logic_error`. Allocation and ID-exhaustion failures can also propagate. |
| One UI action makes several controller calls. | Each call commits separately. There is no public batch transaction or undo stack. Record the GUI's intended operation and handle a partial sequence explicitly. |
| A selected object is deleted. | Clear its borrowed pointer/reference. Deleting a placement also invalidates every runtime child under that placement; deleting a circuit invalidates all its contents. |
| A connectivity edit succeeds. | Reacquire derived nets through surviving ports or nodes. Do not store Net pointers as permanent wire selections or retain references to their old runtime values. |
| A pin changes width or role. | Refresh its runtime-value references and affected placement port handles. Resized/retyped pins get fresh state: LOW for an input, UNKNOWN for an output. |
| A blueprint changes while several views show it. | Refresh every view of that blueprint and affected placement interfaces. Unchanged components and CircuitState objects retain their addresses, but vector iterators and interface indices may change. |
| The GUI needs runtime storage. | Use `controller.start()` and the existing nested states. Do not construct independent CircuitState objects for these blueprints, reset/delete owned states, or write directly through mutable value accessors. Use controller input methods so propagation and ownership checks occur. |
| The GUI needs coordinates, persistence, or undo. | Store those at the application layer; this API currently provides no layout, save/load, cross-project import, or undo methods. |

## Complete example: place and wire an AND gate

This callable example checks a HIGH output, toggles an input, and then removes
the gate while retaining the same root CircuitState. Link it with the core
simulator sources, excluding the existing `src/main.cpp` entry point.

```cpp
#include "ProjectController.h"
#include "CircuitState.h"
#include "Gates.h"

bool gui_and_gate_reference()
{
    ProjectController controller;
    Circuit & circuit = controller.create_circuit();
    Pin & input_a = controller.create_pin(circuit, INPUT_PIN);
    Pin & input_b = controller.create_pin(circuit, INPUT_PIN);
    Pin & circuit_output = controller.create_pin(circuit, OUTPUT_PIN);
    AndGate & gate = controller.create_and_gate(circuit);

    WireNode & a_node = controller.create_wire_node(circuit);
    WireNode & b_node = controller.create_wire_node(circuit);
    WireNode & gate_output_node = controller.create_wire_node(circuit);
    WireNode & circuit_output_node = controller.create_wire_node(circuit);
    controller.connect(circuit, *input_a.output_port(0), a_node);
    controller.connect(circuit, *gate.input_port(0), a_node);
    controller.connect(circuit, *input_b.output_port(0), b_node);
    controller.connect(circuit, *gate.input_port(1), b_node);
    controller.connect(circuit, *gate.output_port(), gate_output_node);
    controller.create_wire_edge(circuit, gate_output_node, circuit_output_node);
    controller.connect(circuit, *circuit_output.input_port(0), circuit_output_node);

    CircuitState & state = controller.start(circuit);
    controller.set(state, input_a, LogicVector(1, HIGH));
    controller.set(state, input_b, LogicVector(1, HIGH));
    if (!controller.run() || circuit_output.value(state)[0] != HIGH)
    {
        return false;
    }

    controller.toggle(state, input_a);
    if (!controller.run() || circuit_output.value(state)[0] != LOW)
    {
        return false;
    }

    controller.remove_component(circuit, gate);
    // gate is now invalid. The root state and the other components survive.
    if (!controller.run() || circuit_output.value(state)[0] != UNKNOWN)
    {
        return false;
    }

    controller.stop(state);
    // state is now invalid. Remaining project objects belong to controller.
    return true;
}
```

## Complete example: reuse and delete a custom circuit

This custom circuit passes an input through to an output. It is placed in two
different parent blueprints and also simulated as its own root. Removing one
placement leaves the other placement and its runtime state alive. Removing the
blueprint requires removing both placements and stopping its independent root.

```cpp
#include "ProjectController.h"
#include "CircuitState.h"
#include "Subcircuit.h"
#include "SubcircuitState.h"

#include <stdexcept>

bool gui_shared_circuit_reference()
{
    ProjectController controller;
    Circuit & custom_circuit = controller.create_circuit();
    Pin & custom_input = controller.create_pin(custom_circuit, INPUT_PIN);
    Pin & custom_output = controller.create_pin(custom_circuit, OUTPUT_PIN);
    WireNode & node = controller.create_wire_node(custom_circuit);
    controller.connect(custom_circuit, *custom_input.output_port(0), node);
    controller.connect(custom_circuit, *custom_output.input_port(0), node);

    Circuit & first_parent = controller.create_circuit();
    Circuit & second_parent = controller.create_circuit();
    Subcircuit & first_placement = controller.create_subcircuit(first_parent, custom_circuit);
    Subcircuit & second_placement = controller.create_subcircuit(second_parent, custom_circuit);
    CircuitState & first_root = controller.start(first_parent);
    CircuitState & second_root = controller.start(second_parent);
    CircuitState & independent_root = controller.start(custom_circuit);
    if (!controller.run())
    {
        return false;
    }

    CircuitState & first_child = dynamic_cast< SubcircuitState & >(first_root.component_state(first_placement)).child_state();
    CircuitState & second_child = dynamic_cast< SubcircuitState & >(second_root.component_state(second_placement)).child_state();
    if (&first_child == &second_child)
    {
        return false;
    }

    bool deletion_rejected = false;
    try
    {
        controller.remove_circuit(custom_circuit);
    }
    catch (const std::logic_error &)
    {
        deletion_rejected = true; // Still used by placements and a root.
    }
    if (!deletion_rejected)
    {
        return false;
    }

    controller.remove_component(first_parent, first_placement);
    // first_placement and first_child are invalid; first_root survives.
    CircuitState & surviving_child = dynamic_cast< SubcircuitState & >(second_root.component_state(second_placement)).child_state();
    if (&surviving_child != &second_child || !controller.run())
    {
        return false;
    }

    controller.remove_component(second_parent, second_placement);
    // second_placement and second_child are now invalid too.
    controller.stop(independent_root);
    controller.remove_circuit(custom_circuit);
    // custom_circuit, its pins, and node are now invalid. Parent roots survive.
    return controller.run() && controller.circuits().size() == 2;
}
```
