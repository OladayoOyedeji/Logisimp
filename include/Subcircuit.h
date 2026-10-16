/**
 * @file Subcircuit.h
 * @brief Represents an instance of a custom circuit within another circuit.
 * @author Cole Schwandt
 */

#ifndef SUBCIRCUIT_H
#define SUBCIRCUIT_H

#include "Component.h"

class Circuit;

class Subcircuit : public Component
{
public:
    Subcircuit(int id, Circuit & circuit);

    Circuit * circuit() const { return circuit_; }

    ComponentState * create_state(CircuitState & state) const override;

    void evaluate(Simulator & simulator, CircuitState & state) override;

    std::string to_string() const override;

private:
    Circuit * circuit_;
};

#endif // SUBCIRCUIT_H
