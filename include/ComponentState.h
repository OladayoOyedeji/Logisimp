// ComponentState.h
/**
 * Abstract class for component-specific runtime state.
 * Components that require memory during simulation, 
 * such as custom user circuits, store that information
 * in derived state classes.
 */

#ifndef COMPONENTSTATE_H
#define COMPONENTSTATE_H

class ComponentState
{
public:
    virtual ~ComponentState()
    {}
};

#endif // COMPONENTSTATE_H
