/**
 * @file Pin.h
 * @brief Circuit interface component used as an input or output pin.
 * @author Cole Schwandt
 */

#ifndef PIN_H
#define PIN_H

#include "Component.h"
#include "LogicVector.h"

/**
 * @brief Specifies whether a pin is a circuit input or output.
 */
enum PinType
{
    INPUT_PIN,  ///< Provides input to the circuit.
    OUTPUT_PIN  ///< Receives output from the circuit.
};

/**
 * @brief Represents an input or output pin of a circuit.
 */
class Pin : public Component
{
public:
    /**
     * @brief Constructs a pin with the specified type and width.
     * @param id Pin identifier.
     * @param type Input or output pin type.
     * @param width Number of bits carried by the pin.
     */
    Pin(int id, PinType type, int width = 1);

    /**
     * @brief Gets the pin's type.
     * @return Copy of the pin type.
     */
    PinType type() const { return type_; }

    /**
     * @brief Gets the pin's bit width.
     * @return Copy of the number of bits.
     */
    int width() const { return width_; }

    /**
     * @brief Accesses the pin's current logic value.
     * @param state Runtime state of the circuit.
     * @return Reference to the logic vector.
     */
    LogicVector & value(CircuitState & state);

    /**
     * @brief Gets the pin's current logic value.
     * @param state Runtime state of the circuit.
     * @return Copy of the logic vector.
     */
    LogicVector value(const CircuitState & state) const;

    /**
     * @brief Sets the pin's logic value during simulation.
     * @param value Logic vector to assign to the pin.
     * @param simulator Simulator performing the update.
     * @param state Runtime state of the circuit.
     */
    void set(const LogicVector & value, Simulator & simulator, CircuitState & state);

    /**
     * @brief Toggles the pin's logic value.
     * @param simulator Simulator performing the update.
     * @param state Runtime state of the circuit.
     */
    void toggle(Simulator & simulator, CircuitState & state);

    /**
     * @brief Creates the pin's runtime state.
     * @param state Runtime state of the circuit.
     * @return Pointer to the created component state.
     */
    ComponentState * create_state(CircuitState & state) const override;

    /**
     * @brief Evaluates the pin during simulation.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit.
     */
    void evaluate(Simulator & simulator, CircuitState & state) override;

    /**
     * @brief Converts the pin to its string representation.
     * @return String representation of the pin.
     */
    std::string to_string() const override;

private:
    friend class Circuit;
    friend class ProjectEdit;

    PinType type_; ///< Whether this is an input or output pin.
    int width_;    ///< Number of bits carried by the pin.
};

#endif // PIN_H
