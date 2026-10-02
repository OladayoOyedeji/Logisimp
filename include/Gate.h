// Gate.h
/**
 * Base class for primtive single-output logic gates,
 * like NOT, AND, OR, etc.
 */

#ifndef GATE_H
#define GATE_H

#include "Component.h"
#include "LogicValue.h"

class Gate : public Component
{
public:
    Gate(int id, int input_count);

    Port * output();
    Port * output() const;

    std::string to_string() const override;

protected:
    LogicValue input_value(int index) const;
    void drive_output(LogicValue value, Simulator & simulator);
};

#endif // GATE_H
