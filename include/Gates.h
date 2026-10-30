/**
 * @file Gates.h
 * @brief Primitive logic gate components, such as AND, OR, and NOT.
 * @author Cole Schwandt
 */

#ifndef GATES_H
#define GATES_H

#include "Gate.h"

/**
 * @brief Logic gate that performs the AND operation on its inputs.
 */
class AndGate : public Gate
{
public:
    /**
     * @brief Constructs an AND gate.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    AndGate(int id, int input_count = 2);

    /**
     * @brief Evaluates the AND operation on the gate's inputs.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the AND gate.
     */
    std::string to_string() const override;
};

/**
 * @brief Logic gate that performs the OR operation on its inputs.
 */
class OrGate : public Gate
{
public:
    /**
     * @brief Constructs an OR gate.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    OrGate(int id, int input_count = 2);

    /**
     * @brief Evaluates the OR operation on the gate's inputs.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the OR gate.
     */
    std::string to_string() const override;
};

/**
 * @brief Logic gate that performs the NOT operation on its input.
 */
class NotGate : public Gate
{
public:
    /**
     * @brief Constructs a NOT gate with one input.
     * @param id Gate identifier.
     */
    NotGate(int id);

    /**
     * @brief Evaluates the NOT operation on the gate's input.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the NOT gate.
     */
    std::string to_string() const override;
};

/**
 * @brief Logic gate that performs the NAND operation on its inputs.
 */
class NandGate : public Gate
{
public:
    /**
     * @brief Constructs a NAND gate.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    NandGate(int id, int input_count = 2);

    /**
     * @brief Evaluates the NAND operation on the gate's inputs.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the NAND gate.
     */
    std::string to_string() const override;
};

/**
 * @brief Logic gate that performs the NOR operation on its inputs.
 */
class NorGate : public Gate
{
public:
    /**
     * @brief Constructs a NOR gate.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    NorGate(int id, int input_count = 2);

    /**
     * @brief Evaluates the NOR operation on the gate's inputs.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the NOR gate.
     */
    std::string to_string() const override;
};

/**
 * @brief Logic gate that performs the XOR operation on its inputs.
 */
class XorGate : public Gate
{
public:
    /**
     * @brief Constructs an XOR gate.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    XorGate(int id, int input_count = 2);

    /**
     * @brief Evaluates the XOR operation on the gate's inputs.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the XOR gate.
     */
    std::string to_string() const override;
};

/**
 * @brief Logic gate that performs the XNOR operation on its inputs.
 */
class XnorGate : public Gate
{
public:
    /**
     * @brief Constructs an XNOR gate.
     * @param id Gate identifier.
     * @param input_count Number of input ports.
     */
    XnorGate(int id, int input_count = 2);

    /**
     * @brief Evaluates the XNOR operation on the gate's inputs.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the gate to its string representation.
     * @return String representation of the XNOR gate.
     */
    std::string to_string() const override;
};

#endif // GATES_H