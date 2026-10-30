/**
 * @brief intermediate circuit controller that
 *        connects the visual gates to logical
 *        components and visual wires to nets
 */


#pragma once

#include "Project.h"
#include "CircuitState.h"
#include "Simulator.h"

#include <memory>

class CircuitController
{
  public:
    CircuitController()
        : circuit(project.create_circuit())
    {}

    AndGate& addAndGate();
    OrGate& addOrGate();
    NotGate& addNotGate();

    void connectPorts(Port&, Port&);

    void bindPorts(gui::Gate&, ::Component&)  
  private:
    Project project;
    Circuit& circuit;
};
