/**
 * @file ComponentState.h
 * @brief Abstract class for component-specific runtime state.
 * @author Cole Schwandt
 */

#ifndef COMPONENTSTATE_H
#define COMPONENTSTATE_H

/**
 * @brief Base class for storing component-specific simulation state.
 */
class ComponentState
{
public:
    /**
     * @brief Destroys component state.
     * @note Virtual to ensure derived classes are properly destroyed.
     */
    virtual ~ComponentState()
    {}
};

#endif // COMPONENTSTATE_H
