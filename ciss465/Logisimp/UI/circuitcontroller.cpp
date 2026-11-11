#include "circuitcontroller.h"

AndGate& CircuitController::addAndGate()
{
    return project.create_and_gate(circuit);
}

OrGate& CircuitController::addOrGate()
{
    return project.create_or_gate(circuit);
}

NotGate& CircuitController::addNotGate()
{
    return project.create_not_gate(circuit);
}

void CircuitController::connectPorts(Port& output, Port& input)
{
    WireNode& node = project.create_wire_node(circuit);

    project.connect(circuit, output, node);
    project.connect(circuit, input, node);
}

void CircuitController::bindPorts(gui::Gate& visualGate,
                                  ::Component& logicGate)
{
    const auto& visualInputs = visualGate.getInputPorts();
    const auto& visualOutputs = visualGate.getOutputPorts();

    // Obtain logical input and output ports from LogicGate
    // Use the actual accessors provided by the Component/Gate API

    // For each matching input:
    // visualInputs[i]->setLogicPort(&logicalInput);

    // For each matching Ouput:
    // visualOutputs[i]->setLogicPort(&logicalOutput);
}
