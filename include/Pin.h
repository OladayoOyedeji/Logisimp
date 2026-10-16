/**
 * @file Pin.h
 * @brief Circuit interface component used as an input or output pin.
 * @author Cole Schwandt
 */

#ifndef PIN_H
#define PIN_H

#include "Component.h"
#include "LogicVector.h"

enum PinType
{
    INPUT_PIN,
    OUTPUT_PIN
};

class Pin : public Component
{
public:
    Pin(int id, PinType type, int width = 1);

    PinType type() const { return type_; }
    int width() const { return width_; }

    LogicVector & value(CircuitState & state);
    LogicVector value(const CircuitState & state) const;

    void set(const LogicVector & value, Simulator & simulator, CircuitState & state);
    void toggle(Simulator & simulator, CircuitState & state);

    ComponentState * create_state(CircuitState & state) const override;

    void evaluate(Simulator & simulator, CircuitState & state) override;

    std::string to_string() const override;

private:
    friend class Circuit;

    PinType type_;
    int width_;
};

#endif // PIN_H
