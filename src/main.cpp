/**
 * @file main.cpp
 * @brief Examples of circuit construction, simulation, and live editing through ProjectController.
 * @author Cole Schwandt, Ethan Graham, Robert Oyedeji
 */

#include "ProjectController.h"
#include "CircuitState.h"
#include "Gates.h"
#include "Subcircuit.h"

#include <iostream>
#include <stdexcept>

//=============================================================
// EXAMPLE 1: Basic AND Gate
//=============================================================
void and_gate_example()
{
    // Create the project controller and circuit.
    ProjectController controller;
    Circuit & circuit = controller.create_circuit();

    // Create two inputs and one output.
    Pin & a = controller.create_pin(circuit, INPUT_PIN);
    Pin & b = controller.create_pin(circuit, INPUT_PIN);
    Pin & result = controller.create_pin(circuit, OUTPUT_PIN);

    // Create an AND gate.
    AndGate & gate = controller.create_and_gate(circuit);

    // Create three wire nodes.
    WireNode & a_node = controller.create_wire_node(circuit);
    WireNode & b_node = controller.create_wire_node(circuit);
    WireNode & result_node = controller.create_wire_node(circuit);

    // Connect input A to the first gate input.
    controller.connect(circuit, *a.output_port(0), a_node);
    controller.connect(circuit, *gate.input_port(0), a_node);

    // Connect input B to the second gate input.
    controller.connect(circuit, *b.output_port(0), b_node);
    controller.connect(circuit, *gate.input_port(1), b_node);

    // Connect the gate output to the circuit output.
    controller.connect(circuit, *gate.output_port(), result_node);
    controller.connect(circuit, *result.input_port(0), result_node);

    // Create the simulation.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    // Supply pin input values.
    controller.set(state, a, LogicVector(1, HIGH));
    controller.set(state, b, LogicVector(1, HIGH));

    // Run the simulator.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // Read output.
    std::cout << "AND(1, 1) = " << result.value(state)[0] << '\n';

    // End simulation.
    controller.stop(state);
    // The controller also stops remaining states automatically on destruction.
}


//=============================================================
// EXAMPLE 2: Creating a Custom NAND Circuit
//=============================================================
// NAND = NOT(AND(A, B))
//
// This function builds a reusable circuit blueprint.
// It does not create a simulation.
Circuit & create_custom_nand(ProjectController & controller)
{
    // Create a new circuit.
    Circuit & circuit = controller.create_circuit();

    // Create the circuit's external inputs and output.
    Pin & a = controller.create_pin(circuit, INPUT_PIN);
    Pin & b = controller.create_pin(circuit, INPUT_PIN);
    Pin & result = controller.create_pin(circuit, OUTPUT_PIN);

    // Create the internal gates.
    AndGate & and_gate = controller.create_and_gate(circuit);
    NotGate & not_gate = controller.create_not_gate(circuit);

    // Create the wire nodes.
    WireNode & a_node = controller.create_wire_node(circuit);
    WireNode & b_node = controller.create_wire_node(circuit);
    WireNode & middle_node = controller.create_wire_node(circuit);
    WireNode & result_node = controller.create_wire_node(circuit);

    // Connect the circuit inputs to the AND gate.
    controller.connect(circuit, *a.output_port(0), a_node);
    controller.connect(circuit, *and_gate.input_port(0), a_node);

    controller.connect(circuit, *b.output_port(0), b_node);
    controller.connect(circuit, *and_gate.input_port(1), b_node);

    // Connect the AND gate output to the NOT gate input.
    controller.connect(circuit, *and_gate.output_port(), middle_node);
    controller.connect(circuit, *not_gate.input_port(0), middle_node);

    // Connect the NOT gate to the circuit output.
    controller.connect(circuit, *not_gate.output_port(), result_node);
    controller.connect(circuit, *result.input_port(0), result_node);

    // Return the reusable circuit.
    return circuit;
}

void custom_nand_example()
{
    ProjectController controller;

    // Build the custom NAND circuit.
    Circuit & circuit = create_custom_nand(controller);

    // Get its input and output pins.
    Pin & a = circuit.input_pin(0);
    Pin & b = circuit.input_pin(1);
    Pin & result = circuit.output_pin(0);

    // Create the simulation.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    // Supply input values.
    controller.set(state, a, LogicVector(1, HIGH));
    controller.set(state, b, LogicVector(1, LOW));

    // Run the simulator.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // Read the output.
    std::cout << "Custom NAND(1, 0) = "
              << result.value(state)[0] << '\n';
}


//=============================================================
// EXAMPLE 3: Using a Custom Circuit Inside Another Circuit
//=============================================================
void subcircuit_example()
{
    // Create the project controller.
    ProjectController controller;

    // Create custom NAND circuit.
    Circuit & nand_circuit = create_custom_nand(controller);

    // Create a second circuit.
    Circuit & circuit = controller.create_circuit();

    // Create the second circuit's inputs and output.
    Pin & a = controller.create_pin(circuit, INPUT_PIN);
    Pin & b = controller.create_pin(circuit, INPUT_PIN);
    Pin & result = controller.create_pin(circuit, OUTPUT_PIN);

    // Place the custom NAND inside the second circuit.
    Subcircuit & nand_gate = controller.create_subcircuit(circuit, nand_circuit);

    // Create three wire nodes.
    WireNode & a_node = controller.create_wire_node(circuit);
    WireNode & b_node = controller.create_wire_node(circuit);
    WireNode & result_node = controller.create_wire_node(circuit);

    // Connect the external inputs to the custom NAND.
    controller.connect(circuit, *a.output_port(0), a_node);
    controller.connect(circuit, *nand_gate.input_port(0), a_node);

    controller.connect(circuit, *b.output_port(0), b_node);
    controller.connect(circuit, *nand_gate.input_port(1), b_node);

    // Connect the custom NAND output to the circuit output.
    controller.connect(circuit, *nand_gate.output_port(0), result_node);
    controller.connect(circuit, *result.input_port(0), result_node);

    // Create the simulation for the outer circuit.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    // Supply input values.
    controller.set(state, a, LogicVector(1, HIGH));
    controller.set(state, b, LogicVector(1, HIGH));

    // Run the simulator.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // Read the output.
    std::cout << "Subcircuit NAND(1, 1) = "
              << result.value(state)[0] << '\n';
}


//=============================================================
// EXAMPLE 4: Connecting Wire Edges and Branching
//=============================================================
// One input feeds two output pins:
//
//              +---- Output 1
//              |
// Input A -----+
//              |
//              +---- Output 2
//
// Unlike the earlier examples, the ports are attached to
// different nodes. WireEdge objects connect those nodes.
void wire_branch_example()
{
    // Create the project controller and circuit.
    ProjectController controller;
    Circuit & circuit = controller.create_circuit();

    // Create one input and two outputs.
    Pin & a = controller.create_pin(circuit, INPUT_PIN);
    Pin & output1 = controller.create_pin(circuit, OUTPUT_PIN);
    Pin & output2 = controller.create_pin(circuit, OUTPUT_PIN);

    // Create three wire nodes.
    WireNode & source_node = controller.create_wire_node(circuit);
    WireNode & first_node = controller.create_wire_node(circuit);
    WireNode & second_node = controller.create_wire_node(circuit);

    // Connect the nodes using wire edges.
    controller.create_wire_edge(circuit, source_node, first_node);
    controller.create_wire_edge(circuit, source_node, second_node);

    // Attach the input and outputs to their nodes.
    controller.connect(circuit, *a.output_port(0), source_node);
    controller.connect(circuit, *output1.input_port(0), first_node);
    controller.connect(circuit, *output2.input_port(0), second_node);

    // All three nodes now belong to the same logical Net.

    // Create the simulation.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    // Supply an input value.
    controller.set(state, a, LogicVector(1, HIGH));

    // Run the simulator.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // Read outputs.
    std::cout << "Branch output 1 = "
              << output1.value(state)[0] << '\n';

    std::cout << "Branch output 2 = "
              << output2.value(state)[0] << '\n';
}


//=============================================================
// EXAMPLE 5: Deleting a Wire Edge
//=============================================================
// Before:
//
// Input A -------- Output
//
// After deleting the edge:
//
// Input A          Output
//
// The output becomes UNKNOWN because its net has no driver.
void delete_wire_example()
{
    // Create the project controller and circuit.
    ProjectController controller;
    Circuit & circuit = controller.create_circuit();

    // Create an input and output.
    Pin & a = controller.create_pin(circuit, INPUT_PIN);
    Pin & result = controller.create_pin(circuit, OUTPUT_PIN);

    // Create two nodes.
    WireNode & first_node = controller.create_wire_node(circuit);
    WireNode & second_node = controller.create_wire_node(circuit);

    // Connect the nodes with an edge.
    WireEdge & wire_edge = controller.create_wire_edge(circuit, first_node, second_node);

    // Attach the input and output.
    controller.connect(circuit, *a.output_port(0), first_node);
    controller.connect(circuit, *result.input_port(0), second_node);

    // Simulate the connected circuit.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    controller.set(state, a, LogicVector(1, HIGH));

    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    std::cout << "Before deleting wire = "
              << result.value(state)[0] << '\n';

    // Delete the wire edge.
    // The controller coordinates queued events and updates this existing state.
    controller.remove_wire_edge(circuit, wire_edge);

    // Continue the simulation using the modified circuit and the same state.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // The output is now disconnected from A.
    std::cout << "After deleting wire = "
              << result.value(state)[0] << '\n';
}


//=============================================================
// EXAMPLE 6: Deleting a Component
//=============================================================
// Before:
//
// Input A ---- NOT ---- Output
//
// After deleting the NOT gate:
//
// Input A ----     ---- Output
void delete_component_example()
{
    // Create the project controller and circuit.
    ProjectController controller;
    Circuit & circuit = controller.create_circuit();

    // Create an input, output, and NOT gate.
    Pin & a = controller.create_pin(circuit, INPUT_PIN);
    Pin & result = controller.create_pin(circuit, OUTPUT_PIN);
    NotGate & gate = controller.create_not_gate(circuit);

    // Create two wire nodes.
    WireNode & input_node = controller.create_wire_node(circuit);
    WireNode & output_node = controller.create_wire_node(circuit);

    // Connect the input to the NOT gate.
    controller.connect(circuit, *a.output_port(0), input_node);
    controller.connect(circuit, *gate.input_port(0), input_node);

    // Connect the NOT gate to the output.
    controller.connect(circuit, *gate.output_port(), output_node);
    controller.connect(circuit, *result.input_port(0), output_node);

    // Simulate.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    controller.set(state, a, LogicVector(1, HIGH));

    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    std::cout << "Before deleting NOT gate = "
              << result.value(state)[0] << '\n';

    // Delete the NOT gate.
    // (Its ports are detached from the wire nodes automatically)
    controller.remove_component(circuit, gate);

    // Continue the same simulation after deleting the component.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // Nothing drives the output anymore.
    std::cout << "After deleting NOT gate = "
              << result.value(state)[0] << '\n';
}


//=============================================================
// EXAMPLE 7: Using a Custom Circuit Twice, Then Deleting One
//=============================================================
void delete_subcircuit_example()
{
    // Create the project controller and reusable NAND circuit.
    ProjectController controller;
    Circuit & nand_circuit = create_custom_nand(controller);

    // Create a new circuit with two pairs of inputs and two outputs.
    Circuit & circuit = controller.create_circuit();
    Pin & a1 = controller.create_pin(circuit, INPUT_PIN);
    Pin & b1 = controller.create_pin(circuit, INPUT_PIN);
    Pin & a2 = controller.create_pin(circuit, INPUT_PIN);
    Pin & b2 = controller.create_pin(circuit, INPUT_PIN);
    Pin & first_result = controller.create_pin(circuit, OUTPUT_PIN);
    Pin & second_result = controller.create_pin(circuit, OUTPUT_PIN);

    // Place the same custom NAND circuit twice.
    Subcircuit & first_nand = controller.create_subcircuit(circuit, nand_circuit);
    Subcircuit & second_nand = controller.create_subcircuit(circuit, nand_circuit);

    // Connect the first NAND's inputs and output.
    WireNode & a1_node = controller.create_wire_node(circuit);
    WireNode & b1_node = controller.create_wire_node(circuit);
    WireNode & first_result_node = controller.create_wire_node(circuit);

    controller.connect(circuit, *a1.output_port(0), a1_node);
    controller.connect(circuit, *first_nand.input_port(0), a1_node);
    controller.connect(circuit, *b1.output_port(0), b1_node);
    controller.connect(circuit, *first_nand.input_port(1), b1_node);
    controller.connect(circuit, *first_nand.output_port(0), first_result_node);
    controller.connect(circuit, *first_result.input_port(0), first_result_node);

    // Connect the second NAND's inputs and output.
    WireNode & a2_node = controller.create_wire_node(circuit);
    WireNode & b2_node = controller.create_wire_node(circuit);
    WireNode & second_result_node = controller.create_wire_node(circuit);

    controller.connect(circuit, *a2.output_port(0), a2_node);
    controller.connect(circuit, *second_nand.input_port(0), a2_node);
    controller.connect(circuit, *b2.output_port(0), b2_node);
    controller.connect(circuit, *second_nand.input_port(1), b2_node);
    controller.connect(circuit, *second_nand.output_port(0), second_result_node);
    controller.connect(circuit, *second_result.input_port(0), second_result_node);

    // Simulate both instances before deleting either one.
    CircuitState & state = controller.start(circuit);

    if (!controller.run())
    {
        throw std::runtime_error("Initialization failed");
    }

    controller.set(state, a1, LogicVector(1, HIGH));
    controller.set(state, b1, LogicVector(1, HIGH));
    controller.set(state, a2, LogicVector(1, HIGH));
    controller.set(state, b2, LogicVector(1, LOW));

    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    std::cout << "First NAND(1, 1) = "
              << first_result.value(state)[0] << '\n';
    std::cout << "Second NAND(1, 0) = "
              << second_result.value(state)[0] << '\n';

    // Delete the first NAND instance.
    // The controller keeps this runtime state and the surviving child alive.
    controller.remove_component(circuit, first_nand);

    // Simulate the modified circuit to show the second NAND still works.
    // Its input values are preserved without setting them again.
    if (!controller.run())
    {
        throw std::runtime_error("Simulation failed");
    }

    // The first output is undriven; the second NAND is unaffected.
    std::cout << "First output after deletion = "
              << first_result.value(state)[0] << '\n';
    std::cout << "Second NAND after deletion = "
              << second_result.value(state)[0] << '\n';
}


//=============================================================
// MAIN
//=============================================================
int main()
{
    try
    {
        std::cout << "\n--- Example 1: AND Gate ---\n";
        and_gate_example();

        std::cout << "\n--- Example 2: Custom NAND ---\n";
        custom_nand_example();

        std::cout << "\n--- Example 3: Subcircuit ---\n";
        subcircuit_example();

        std::cout << "\n--- Example 4: Wire Branching ---\n";
        wire_branch_example();

        std::cout << "\n--- Example 5: Delete Wire ---\n";
        delete_wire_example();

        std::cout << "\n--- Example 6: Delete Component ---\n";
        delete_component_example();

        std::cout << "\n--- Example 7: Delete One Custom Circuit Instance ---\n";
        delete_subcircuit_example();
    }
    catch (const std::exception & error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
