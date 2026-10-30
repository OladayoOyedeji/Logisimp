/**
 * @file Gate.h
 * @brief Base class for primitive single-output logic gates.
 * @author Cole Schwandt
 */

#ifndef GATE_H
#define GATE_H

#include "Component.h"
#include "LogicValue.h"

/**
 * @brief Abstract base class for primitive logic gates with a single output.
 */
class Gate : public Component
{
public:
    /**
     * @brief Constructs a gate with the specified number of inputs.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    Gate(int id, int input_count);

    /**
     * @brief Accesses the gate's output port.
     * @param index Index of the output port, defaults to 0.
     * @return Pointer to the output port.
     */
    Port * output_port(int index = 0);

    /**
     * @brief Gets the gate's output port.
     * @param index Index of the output port, defaults to 0.
     * @return Pointer to the output port.
     */
    Port * output_port(int index = 0) const;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the gate.
     */
    std::string to_string() const override;

protected:
    /**
     * @brief Gets the current logic value of an input port.
     * @param index Index of the input port.
     * @param state Runtime state of the circuit.
     * @return Copy of the input's logic value.
     */
    LogicValue input_value(int index, CircuitState & state) const;
    
    /**
     * @brief Drives the gate's output with the specified logic value.
     * @param value Logic value to output.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void drive_output(LogicValue value, Simulator & simulator, CircuitState & state);
};

#endif // GATE_H
