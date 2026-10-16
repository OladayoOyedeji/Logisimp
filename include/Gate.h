/**
 * @file Gate.h
 * @brief Base class for primitive single-output logic gates.
 * @author Cole Schwandt
 */

#ifndef GATE_H
#define GATE_H

#include "Component.h"
#include "LogicValue.h"

class Gate : public Component
{
public:
    Gate(int id, int input_count);

    Port * output_port(int index = 0);
    Port * output_port(int index = 0) const;

    std::string to_string() const override;

protected:
    LogicValue input_value(int index, CircuitState & state) const;
    void drive_output(LogicValue value, Simulator & simulator, CircuitState & state);
};

#endif // GATE_H
