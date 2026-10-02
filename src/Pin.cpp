// Pin.cpp

#include "Pin.h"
#include "Port.h"
#include "Simulator.h"
#include "Wire.h"

#include <stdexcept>

Pin::Pin(int id, PinType type, int width)
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

void Pin::set(const LogicVector & value, Simulator & simulator)
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

void Pin::toggle(Simulator & simulator)
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

void Pin::evaluate(Simulator & simulator)
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

void Pin::reset()
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

std::string Pin::to_string() const
{
    std::string ret;

    ret += "Pin(id=";
    ret += std::to_string(id());
    ret += ", type=";

    if (type_ == INPUT_PIN)
    {
        ret += "INPUT_PIN";
    }
    else
    {
        ret += "OUTPUT_PIN";
    }

    ret += ", width=";
    ret += std::to_string(value_.width());
    ret += ", value=";
    ret += value_.to_string();
    ret += ')';

    return ret;
}
