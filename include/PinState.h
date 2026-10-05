// PinState.h
/**
 * Stores the runtime value of a circuit pin.
 */

#ifndef PINSTATE_H
#define PINSTATE_H

#include "ComponentState.h"
#include "LogicVector.h"

class PinState : public ComponentState
{
public:
    PinState(int width, LogicValue initial_value)
        : value_(width, initial_value)
    {}

    LogicVector & value() { return value_; }
    LogicVector value() const { return value_; }

private:
    LogicVector value_;
};

#endif // PINSTATE.h
