// Pin.h
/**
 * Circuit interface component used as an input or output pin.
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

    LogicVector & value() { return value_; }
    LogicVector value() const { return value_; }

    void set(const LogicVector & value, Simulator & simulator);
    void toggle(Simulator & simulator);

    void evaluate(Simulator & simulator) override;
    void reset() override;

    std::string to_string() const override;

private:
    PinType type_;
    LogicVector value_;
};

#endif // PIN_H
