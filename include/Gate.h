// Gate.h

#ifndef GATE_H
#define GATE_H

#include "Component.h"
#include "LogicValue.h"
#include "Simulator.h"

#include <stdexcept>

class Gate : public Component
{
public:
    Gate(int id, int input_count)
        : Component(id)
    {
        if (input_count <= 0)
        {
            throw std::invalid_argument("Gate input count must be positive");
        }

        for (int i = 0; i < input_count; i++)
        {
            add_input(1);
        }

        add_output(1, "Y");
    }

    Port * output()
    {
        return output_port(0);
    }

    Port * output() const
    {
        return output_port(0);
    }

    void print(std::ostream & cout) const
    {
        cout << "Gate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
    
protected:
    LogicValue input_value(int index) const
    {
        Port * port = input_port(index);

        if (port->wire() == nullptr)
        {
            return UNKNOWN;
        }

        return port->wire()->value()[0];
    }

    void drive_output(LogicValue value, Simulator & simulator)
    {
        Port * port = output_port(0);

        if (port->wire() == nullptr)
        {
            return;
        }

        LogicVector output_value(1);

        output_value[0] = value;

        simulator.drive(*port->wire(), output_value);
    }
};

#endif
