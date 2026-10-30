/**
 * @file main.cpp
 * @brief Common GUI actions using ProjectController and EditorController.
 * @author Cole Schwandt, Ethan Graham, Robert Oyedeji
 */

#include "ProjectController.h"
#include "EditorController.h"

#include <iostream>
#include <stdexcept>

void create_circuit_example()
{
    ProjectController project;

    // User clicks Add Circuit.
    CircuitId circuit = project.add_circuit("Main");
    // User renames the circuit.
    project.set_circuit_name(circuit, "My circuit");
    std::cout << "Circuit " << circuit.value << ": " << project.circuit_name(circuit) << '\n';
}

void place_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);

    // User selects NAND from the toolbar. Nothing is created yet.
    editor.select_tool(EditorTool::NandGate);
    // User clicks the canvas.
    ComponentId nand = editor.on_canvas_click({200, 150});
    std::cout << "Placed NAND " << nand.value << '\n';
}

void add_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);

    // GUI creates an AND gate directly, without changing the toolbar tool.
    ComponentId gate = editor.add_component(ComponentKind::AndGate, {200, 150});
    std::cout << "Created AND " << gate.value << '\n';
}

void select_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId gate = editor.add_component(ComponentKind::AndGate, {200, 150});
    ComponentId clicked_id = gate; // Reuse the created ID in place of GUI hit-testing.

    // User clicks the gate with the selection tool.
    editor.on_component_click(clicked_id);
    std::cout << "Selected component: " << editor.selected_component().value << '\n';
}

void edit_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId input = editor.add_component(ComponentKind::InputPin, {100, 150});

    // User changes the pin's properties.
    editor.set_pin_width(input, 8);
    editor.set_label(input, "Data");
    // GUI reads the properties without changing selection.
    ComponentProperties info = editor.properties(input);
    std::cout << info.label << ": " << info.pin_width << " bits\n";

    // A properties panel can also operate on the current selection.
    editor.select_component(input);
    editor.set_selected_label("Selected Data");
}

void delete_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId gate = editor.add_component(ComponentKind::AndGate, {200, 150});
    ComponentId clicked_id = gate; // Reuse the created ID in place of GUI hit-testing.

    // User deletes this known component.
    editor.delete_component(clicked_id);
    std::cout << "Selection after deletion: " << editor.selected_component().value << " (-1 means none)\n";

    gate = editor.add_component(ComponentKind::NandGate, {200, 150});
    // User selects a component and presses Delete.
    editor.select_component(gate);
    editor.delete_selected();
}

void connect_ports_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId input = editor.add_component(ComponentKind::InputPin, {100, 150});
    ComponentId gate = editor.add_component(ComponentKind::AndGate, {300, 150});

    // User connects two known ports.
    editor.connect_ports(editor.output_port(input), editor.input_port(gate, 0));
    WireNodeId node = editor.attached_node(editor.output_port(input));
    std::cout << "Connected at node " << node.value << "; explicit edges: " << editor.edges().size() << '\n';

    // User disconnects the gate's input.
    editor.disconnect_port(editor.input_port(gate, 0));
}

void toggle_input_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId input = editor.add_component(ComponentKind::InputPin, {100, 150});

    // User clicks the input during simulation.
    editor.toggle_input(input);
    if (!editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }
    std::cout << "Input after click: " << editor.value(input) << '\n';
}

void read_signal_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId first_input = editor.add_component(ComponentKind::InputPin, {100, 100});
    ComponentId second_input = editor.add_component(ComponentKind::InputPin, {100, 200});
    ComponentId gate = editor.add_component(ComponentKind::AndGate, {300, 150});
    ComponentId output = editor.add_component(ComponentKind::OutputPin, {500, 150});
    editor.connect_ports(editor.output_port(first_input), editor.input_port(gate, 0));
    editor.connect_ports(editor.output_port(second_input), editor.input_port(gate, 1));
    editor.connect_ports(editor.output_port(gate), editor.input_port(output));
    editor.set_input(first_input, HIGH);
    editor.set_input(second_input, HIGH);
    if (!editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }

    // GUI reads an output pin, a port, and a wire node to display their values.
    PortId port = editor.input_port(output);
    WireNodeId wire = editor.attached_node(port);
    std::cout << "AND output: " << editor.value(output)
              << "; port: " << editor.value(port) << "; wire: " << editor.value(wire) << '\n';
}

void move_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId gate = editor.add_component(ComponentKind::NandGate, {200, 150});

    // User drags the gate.
    editor.move_component(gate, {300, 200});
    CanvasPosition position;
    if (editor.component_position(gate, position))
    {
        std::cout << "Gate position: " << position.x << ", " << position.y << '\n';
    }
}

void duplicate_component_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId gate = editor.add_component(ComponentKind::AndGate, {200, 150}, 3);
    editor.set_label(gate, "Three-input AND");

    // User duplicates the gate. External wiring is not copied.
    ComponentId copy = editor.duplicate_component(gate, {400, 200});
    ComponentProperties info = editor.properties(copy);
    std::cout << "Copy " << copy.value << ": " << info.label << ", " << info.inputs.size() << " inputs\n";
}

void place_subcircuit_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId nand_circuit = project.add_circuit("NAND");
    EditorController nand_editor(project, nand_circuit, layout);
    ComponentId first_input = nand_editor.add_component(ComponentKind::InputPin, {100, 100});
    ComponentId second_input = nand_editor.add_component(ComponentKind::InputPin, {100, 200});
    ComponentId gate = nand_editor.add_component(ComponentKind::NandGate, {300, 150});
    ComponentId output = nand_editor.add_component(ComponentKind::OutputPin, {500, 150});
    nand_editor.connect_ports(nand_editor.output_port(first_input), nand_editor.input_port(gate, 0));
    nand_editor.connect_ports(nand_editor.output_port(second_input), nand_editor.input_port(gate, 1));
    nand_editor.connect_ports(nand_editor.output_port(gate), nand_editor.input_port(output));
    nand_editor.set_input(first_input, HIGH);
    nand_editor.set_input(second_input, HIGH);
    if (!nand_editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }
    std::cout << "Reusable NAND(1, 1): " << nand_editor.value(output) << '\n';

    CircuitId main_circuit = project.add_circuit("Main");
    EditorController editor(project, main_circuit, layout);
    // User chooses NAND from the reusable-circuit list.
    editor.set_subcircuit_blueprint(nand_circuit);
    editor.select_tool(EditorTool::Subcircuit);
    // User clicks the Main canvas.
    ComponentId placement = editor.on_canvas_click({300, 150});
    std::cout << "Placed NAND: " << placement.value << ", inputs: " << editor.inputs(placement).size() << '\n';

    ComponentId clicked_id = placement; // Reuse the placed ID in place of GUI hit-testing.
    // User clicks this NAND placement and presses Delete.
    editor.select_component(clicked_id);
    editor.delete_selected();
}

void delete_wire_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId input = editor.add_component(ComponentKind::InputPin, {100, 100});
    ComponentId first_output = editor.add_component(ComponentKind::OutputPin, {400, 100});
    ComponentId second_output = editor.add_component(ComponentKind::OutputPin, {400, 200});
    editor.connect_ports(editor.output_port(input), editor.input_port(first_output));

    // Advanced: add an explicit branch with a session.
    CircuitSession session = project.open(circuit);
    WireNodeId junction = editor.attached_node(editor.output_port(input));
    WireNodeId branch = session.add_node();
    WireEdgeId edge = session.add_edge(junction, branch);
    session.attach(editor.input_port(second_output), branch);
    layout.set_node_position(circuit, junction, {250, 100});
    layout.set_node_position(circuit, branch, {250, 200});

    WireEdgeId clicked_edge = edge; // Reuse the segment ID in place of GUI hit-testing.
    // User clicks and deletes this segment; the other branch stays connected.
    editor.delete_wire(clicked_edge);
    std::cout << "Remaining edges: " << editor.edges().size() << '\n';
}

void inspect_objects_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId input = editor.add_component(ComponentKind::InputPin, {100, 100});
    ComponentId output = editor.add_component(ComponentKind::OutputPin, {400, 100});
    editor.connect_ports(editor.output_port(input), editor.input_port(output));

    // GUI associates each returned ID with the component it draws.
    for (ComponentId id : editor.components())
    {
        ComponentProperties info = editor.properties(id);
        std::cout << "Component " << id.value << ", type " << int(info.kind) << ", label " << info.label << '\n';
        if (info.has_position)
        {
            std::cout << "  Position: " << info.position.x << ", " << info.position.y << '\n';
        }
        for (PortId port : info.inputs)
        {
            std::cout << "  Input " << port.value << " attaches to node " << editor.attached_node(port).value << '\n';
        }
        for (PortId port : info.outputs)
        {
            std::cout << "  Output " << port.value << " attaches to node " << editor.attached_node(port).value << '\n';
        }
    }
    for (WireNodeId id : editor.nodes())
    {
        CanvasPosition position;
        std::cout << "Node " << id.value;
        if (editor.node_position(id, position))
        {
            std::cout << " at " << position.x << ", " << position.y;
        }
        std::cout << '\n';
    }
    for (WireEdgeId id : editor.edges())
    {
        auto endpoints = editor.edge_endpoints(id);
        std::cout << "Edge " << id.value << ": nodes " << endpoints.first.value << ", " << endpoints.second.value << '\n';
    }
}

void switch_circuit_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId main_circuit = project.add_circuit("Main");
    CircuitId other_circuit = project.add_circuit("Other");
    EditorController editor(project, main_circuit, layout);
    ComponentId input = editor.add_component(ComponentKind::InputPin, {100, 100});
    editor.set_input(input, HIGH);
    if (!editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }

    // User opens another circuit, then returns to Main.
    editor.view_circuit(other_circuit);
    editor.view_circuit(main_circuit);
    std::cout << "Main input still holds " << editor.value(input) << '\n';
}

void grouped_edits_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    CircuitSession session = project.open(circuit);
    CircuitLayout staged_layout = layout;

    // Advanced: group related edits and positions into one user action.
    session.batch([&](CircuitSession & edit)
    {
        ComponentId input = edit.create(ComponentKind::InputPin);
        ComponentId output = edit.create(ComponentKind::OutputPin);
        edit.connect(edit.output_port(input), edit.input_port(output));
        staged_layout.set_component_position(circuit, input, {-40, 100});
        staged_layout.set_component_position(circuit, output, {200, 100});
    });
    layout.swap(staged_layout);
    std::cout << "Grouped action created " << session.components().size() << " components\n";
}

void display_diagnostics_example()
{
    ProjectController project;
    CircuitLayout layout;
    CircuitId circuit = project.add_circuit("Main");
    EditorController editor(project, circuit, layout);
    ComponentId first_input = editor.add_component(ComponentKind::InputPin, {100, 100});
    ComponentId second_input = editor.add_component(ComponentKind::InputPin, {100, 200});
    ComponentId inverter = editor.add_component(ComponentKind::NotGate, {300, 150});
    ComponentId output = editor.add_component(ComponentKind::OutputPin, {500, 150});
    editor.connect_ports(editor.output_port(first_input), editor.input_port(inverter));
    editor.connect_ports(editor.output_port(second_input), editor.input_port(inverter));
    editor.connect_ports(editor.output_port(inverter), editor.input_port(output));
    editor.set_input(first_input, HIGH);
    editor.set_input(second_input, LOW);
    if (!editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }

    // GUI displays electrical errors without removing the user's wires.
    for (const SessionDiagnostic & diagnostic : editor.diagnostics())
    {
        std::cout << diagnostic.message << '\n';
    }
    // User repairs the conflicting inputs.
    editor.set_input(second_input, HIGH);
    if (!editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }
    std::cout << "Repaired output: " << editor.value(output) << '\n';

    // User changes one connected pin to an incompatible width.
    editor.set_pin_width(first_input, 8);
    if (!editor.run_simulation())
    {
        throw std::runtime_error("Simulation did not settle");
    }
    for (const SessionDiagnostic & diagnostic : editor.diagnostics())
    {
        std::cout << diagnostic.message << '\n';
    }
}

int main()
{
    try
    {
        std::cout << "\n--- Create and rename a circuit ---\n";
        create_circuit_example();
        std::cout << "\n--- Place with a toolbar tool ---\n";
        place_component_example();
        std::cout << "\n--- Create directly ---\n";
        add_component_example();
        std::cout << "\n--- Select an existing component ---\n";
        select_component_example();
        std::cout << "\n--- Edit component properties ---\n";
        edit_component_example();
        std::cout << "\n--- Delete a component ---\n";
        delete_component_example();
        std::cout << "\n--- Connect and disconnect ports ---\n";
        connect_ports_example();
        std::cout << "\n--- Toggle an input ---\n";
        toggle_input_example();
        std::cout << "\n--- Read pin, port, and wire values ---\n";
        read_signal_example();
        std::cout << "\n--- Move a component ---\n";
        move_component_example();
        std::cout << "\n--- Duplicate a component ---\n";
        duplicate_component_example();
        std::cout << "\n--- Place and delete a reusable NAND ---\n";
        place_subcircuit_example();
        std::cout << "\n--- Delete an explicit wire segment ---\n";
        delete_wire_example();
        std::cout << "\n--- Inspect objects for drawing ---\n";
        inspect_objects_example();
        std::cout << "\n--- Switch circuits ---\n";
        switch_circuit_example();
        std::cout << "\n--- Group advanced edits ---\n";
        grouped_edits_example();
        std::cout << "\n--- Display diagnostics ---\n";
        display_diagnostics_example();
    }
    catch (const std::exception & error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
