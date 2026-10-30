/**
 * @file ControllerConnections.cpp
 * @brief Atomic convenience connections that retain existing wire branches.
 */

#include "ProjectController.h"

#include "Port.h"

#include <stdexcept>

void ProjectController::connect(Circuit & circuit, Port & first, Port & second)
{
    edit([&]()
    {
        project_.require_circuit(circuit);
        for (Port * port : { &first, &second })
        {
            Component * owner = port->owner();
            if (owner == nullptr || circuit.find_component(owner->id()) != owner || owner->find_port(port->id()) != port)
            {
                throw std::invalid_argument("Ports must belong to the circuit being connected");
            }
        }

        if (&first == &second || (first.net() != nullptr && first.net() == second.net()))
        {
            return;
        }

        WireNode * first_node = first.node();
        WireNode * second_node = second.node();
        if (first_node == nullptr && second_node == nullptr)
        {
            WireNode & node = project_.create_wire_node(circuit);
            project_.connect(circuit, first, node);
            project_.connect(circuit, second, node);
        }
        else if (first_node == nullptr)
        {
            project_.connect(circuit, first, *second_node);
        }
        else if (second_node == nullptr)
        {
            project_.connect(circuit, second, *first_node);
        }
        else
        {
            project_.create_wire_edge(circuit, *first_node, *second_node);
        }
    });
}
