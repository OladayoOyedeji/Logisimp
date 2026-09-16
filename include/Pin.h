// Pin.h

#ifndef PIN_H
#define PIN_H

#include "Component.h"
#include "Simulator.h"

#include <stdexcept>

enum PinType
{
    INPUT_PIN,
    OUTPUT_PIN
};

class Pin : public Component
{
public:
    Pin(int id, PinType type, int width = 1)
        : Component(id), type_(type), value_(width, type == INPUT_PIN ? LOW : UNKNOWN)
    {
        if (type == INPUT_PIN)
        {
            add_output(width, "Y");
        }
        else
        {
            add_input(width, "A");
        }
    }

    PinType & type() { return type_; }
    PinType type() const { return type_; }

    LogicVector & value() { return value_; }
    LogicVector value() const { return value_; }

    void set(const LogicVector & value, Simulator & simulator)
    {
        if (type_ != INPUT_PIN)
        {
            throw std::logic_error("Cannot set an output pin");
        }

        if (value.width() != value_.width())
        {
            throw std::invalid_argument("Pin value width does not match pin width");
        }

        value_ = value;

        evaluate(simulator);
    }

    void toggle(Simulator & simulator)
    {
        if (type_ != INPUT_PIN)
        {
            throw std::logic_error("Cannot toggle an output pin");
        }

        for (int i = 0; i < value_.width(); i++)
        {
            if (value_[i] == LOW)
            {
                value_[i] = HIGH;
            }
            else if (value_[i] == HIGH)
            {
                value_[i] = LOW;
            }
        }

        evaluate(simulator);
    }

    void evaluate(Simulator & simulator)
    {
        if (type_ == INPUT_PIN)
        {
            Port * port = output_port(0);

            if (port->wire() != nullptr)
            {
                simulator.drive(*port->wire(), value_);
            }

            return;
        }

        Port * port = input_port(0);

        if (port->wire() == nullptr)
        {
            value_ = LogicVector(value_.width(), UNKNOWN);
            return;
        }

        value_ = port->wire()->value();
    }

    void reset()
    {
        if (type_ == INPUT_PIN)
        {
            value_ = LogicVector(value_.width(), LOW);
        }
        else
        {
            value_ = LogicVector(value_.width(), UNKNOWN);
        }
    }

    void print(std::ostream & cout) const
    {
        cout << "Pin("
             << "id=" << id()
             << ", type=";

        if (type_ == INPUT_PIN)
        {
            cout << "INPUT_PIN";
        }
        else
        {
            cout << "OUTPUT_PIN";
        }

        cout << ", width=" << value_.width()
             << ", value=" << value_
             << ')';
    }

private:
    PinType type_;
    LogicVector value_;
};

#endif // Pin.h
