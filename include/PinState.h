/**
 * @file PinState.h
 * @brief Stores the runtime value of a circuit pin.
 * @author Cole Schwandt
 */

#ifndef PINSTATE_H
#define PINSTATE_H

#include "ComponentState.h"
#include "LogicVector.h"

class PinState : public ComponentState
{
public:
    PinState(int width, LogicValue initial_value = UNKNOWN)
        : value_(width, initial_value)
    {}

    LogicVector & value() { return value_; }
    LogicVector value() const { return value_; }

private:
    LogicVector value_;
};

#endif // PINSTATE_H
