/**
 * @file PinState.h
 * @brief Stores the runtime value of a circuit pin.
 * @author Cole Schwandt
 */

#ifndef PINSTATE_H
#define PINSTATE_H

#include "ComponentState.h"
#include "LogicVector.h"

/**
 * @brief Stores the current logic value of a pin during simulation.
 */
class PinState : public ComponentState
{
public:
    /**
     * @brief Constructs a pin state with the specified width and initial value.
     * @param width Number of bits.
     * @param initial_value Initial value of each bit.
     */
    PinState(int width, LogicValue initial_value = UNKNOWN)
        : value_(width, initial_value)
    {}

    /**
     * @brief Accesses the pin's logic value.
     * @return Reference to the logic vector.
     */
    LogicVector & value() { return value_; }

    /**
     * @brief Gets the pin's logic value.
     * @return Copy of the logic vector.
     */
    LogicVector value() const { return value_; }

private:
    LogicVector value_; ///< Current logic value of the pin.
};

#endif // PINSTATE_H