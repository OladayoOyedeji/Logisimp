@page gui_api_reference GUI API quick reference

## Project and circuit management

| User action / GUI event | Method to call |
| --- | --- |
| Create the project and shared layout | `ProjectController controller; CircuitLayout layout;` |
| User clicks Add Circuit with a name | `CircuitId circuit_id = controller.add_circuit("Main");` |
| Add a circuit before choosing its name | `Circuit & circuit = controller.create_circuit();` |
| User renames a circuit | `controller.set_circuit_name(circuit_id, "ALU");` |
| Read the name / populate navigation | `controller.circuit_name(circuit_id)`; `controller.circuit_infos()` |
| Open an editor tab | `EditorController editor(controller, circuit_id, layout);` |
| User switches circuits | `editor.view_circuit(next_id);` |
| Read the viewed circuit | `editor.circuit_id()` |
| Open another tab | `EditorController other(controller, next_id, layout);` |

## Tools, placement and selection

| User action / GUI event | Method to call |
| --- | --- |
| User selects a gate tool | `editor.select_tool(EditorTool::AndGate);` |
| User clicks canvas to place | `ComponentId placed = editor.on_canvas_click({250, 150});` |
| GUI creates a component directly | `ComponentId gate_id = editor.add_component(ComponentKind::NandGate, {200, 150});` |
| Create a configured pin or gate | `editor.add_component(ComponentKind::InputPin, {100, 150}, 8);` |
| User clicks an existing component | `editor.on_component_click(component_id);` |
| GUI explicitly selects a known object | `editor.select_component(component_id);` |
| Keep IDs with drawn objects | Use the returned component/port handles. |
| Read / clear selection | `editor.selected_component()`; `editor.clear_selection();` |
| User clicks empty canvas with Select | `editor.on_canvas_click({x, y});` |

## Component properties, movement and deletion

| User action / GUI event | Method to call |
| --- | --- |
| Populate a properties panel for any component | `ComponentProperties info = editor.properties(component_id);` |
| Read component configuration | `info.kind`; `info.label`; `info.inputs`; `info.outputs`; `info.pin_width`; `info.pin_type`; `info.subcircuit` |
| Read the snapshot's position | `info.has_position`; `info.position` |
| Inspect current selection | `editor.selected_properties()` |
| User changes a label | `editor.set_label(component_id, "My NAND");` |
| User changes pin width | `editor.set_pin_width(input_id, 8);` |
| User changes pin direction | `editor.set_pin_type(input_id, OUTPUT_PIN);` |
| Properties panel edits the selection | `editor.set_selected_label("Data");` |
| User drags a component | `editor.move_component(component_id, {300, 200});` |
| Read a component position | `editor.component_position(component_id, position)` |
| User duplicates a component | `ComponentId copy = editor.duplicate_component(component_id, {400, 200});` |
| User deletes a known component | `editor.delete_component(clicked_id);` |
| User presses Delete for current selection | `ComponentId removed = editor.delete_selected();` |

## Ports, wires and drawing existing objects

| User action / GUI event | Method to call |
| --- | --- |
| Find a component input/output | `editor.input_port(gate_id, 0)`; `editor.output_port(input_id)` |
| Enumerate component ports | `editor.inputs(component_id)`; `editor.outputs(component_id)` |
| User connects two known ports | `editor.connect_ports(first_port, second_port);` |
| User disconnects a port | `editor.disconnect_port(port);` |
| User starts a wire | `editor.select_tool(EditorTool::Wire); editor.begin_wire(first_port);` |
| User finishes on a second port | `editor.finish_wire(second_port);` |
| Inspect / cancel pending wire | `editor.pending_wire()`; `editor.cancel_wire();` |
| User deletes a wire segment | `editor.delete_wire(edge_id);` |
| List existing drawing objects | `editor.components()`; `editor.nodes()`; `editor.edges()` |
| Find a port's attachment | `WireNodeId node = editor.attached_node(port);` |
| Find a segment's endpoints | `auto endpoints = editor.edge_endpoints(edge_id);` |
| Read a wire-node position | `editor.node_position(node_id, position)` |

## Simulation, values and diagnostics

| User action / GUI event | Method to call |
| --- | --- |
| User runs simulation | `bool settled = editor.run_simulation();` |
| Interpret simulation completion | Check `settled`. |
| User clicks an input with Interact | `editor.select_tool(EditorTool::Interact); editor.on_component_click(input_id);` |
| Toggle a known input directly | `editor.toggle_input(input_id);` |
| Set an input | `editor.set_input(input_id, HIGH);` |
| GUI reads a pin value | `editor.value(output_id)` |
| GUI reads a port value | `editor.value(port)` |
| GUI reads a wire signal | `editor.value(node_id)` |
| GUI displays electrical faults | `auto faults = editor.diagnostics();` |
| Read fault details for display | `fault.kind`; `fault.circuit`; `fault.instance`; `fault.nodes`; `fault.bits`; `fault.message` |
| Handle a rejected GUI event | Catch `const std::exception & error`; display `error.what()`. |
| Read bits for drawing | `value.width()`; `value[bit]` |
| Pause processing | Defer calls to `run_simulation()`. |

## Advanced: reusable circuits and runtime instances

| Workflow | API |
| --- | --- |
| Create a reusable circuit | `CircuitId child_id = controller.add_circuit("NAND");` |
| Choose and place a reusable circuit | `editor.set_subcircuit_blueprint(child_id); editor.select_tool(EditorTool::Subcircuit);` |
| Resolve stable placement interface ports | `CircuitSession parent = controller.open(editor.circuit_id());` then `parent.interface_port(placement_id, child_pin_id)` |
| Inspect a placement's referenced circuit | `editor.properties(placement_id).subcircuit` |
| List live occurrences | `controller.instances(circuit_id)` |
| Choose a runtime for an editor | `editor.bind_instance(path);` |
| Open a chosen occurrence | `editor.view_circuit(circuit_id, path);` |
| Restore automatic runtime choice | `editor.unbind_instance();` |
| Start another independent root | `RuntimePath root = controller.start_instance(circuit_id);` |
| Stop a root | `controller.stop(root);` |
| Inspect a nested placement through a session | `controller.open(parent_id, parent_path).child(placement_id)` |
| Use an unbound session for simulation | `controller.open(circuit_id)` |
| Change a nested input | Set the containing circuit's input through its editor. |

## Advanced: wire geometry, multi-object copying and transactions

| Workflow | API |
| --- | --- |
| Open an advanced editing session | `CircuitSession session = controller.open(editor.circuit_id());` |
| Add a junction | `WireNodeId node = session.add_node(0, "Branch");` |
| Add a branch segment | `WireEdgeId edge = session.add_edge(junction, branch);` |
| Attach a port to geometry | `session.attach(port, node);` |
| Remove a node | `session.remove_node(node);` |
| Inspect metadata for copying | `session.node_width(node)`; `session.node_label(node)`; `session.port_width(port)`; `session.port_direction(port)`; `session.port_label(port)` |
| Copy several components | `session.duplicate_component(id)` inside `session.batch(operation)` |
| Resolve connection structure | `session.attached_node(port)`; `session.edge_endpoints(edge)` |
| Store advanced node layout | `layout.set_node_position(circuit_id, node, position);` |
| Stage layout for a grouped action | `CircuitLayout staged_layout = layout;` |
| Group structural edits | `session.batch(operation);` |
| Publish the grouped layout | `layout.swap(staged_layout);` |
| Choose APIs inside a batch | Use session edits and staged layout. |
| Refresh views after external edits | `editor.refresh();`; `controller.revision()` |
| Copy and paste a selection | Keep selected IDs and source-to-copy mappings in the GUI. |
| Copy/remove blueprints | `controller.copy_circuit(circuit)`; `controller.remove_circuit(circuit)` |
