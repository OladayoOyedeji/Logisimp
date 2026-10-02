// Port.cpp

#include "Port.h"

#include <stdexcept>

Port::Port(int id, int width, PortDirection direction, const std::string & label)
    : id_(id),
      width_(width),
      direction_(direction),
      owner_(nullptr),
      wire_(nullptr),
      label_(label)
{
    if (width <= 0)
    {
        throw std::invalid_argument("Port width must be positive");
    }
}

std::string Port::to_string() const
{
    std::string ret;

    ret += "Port(id=";
    ret += std::to_string(id_);
    ret += ", width=";
    ret += std::to_string(width_);
    ret += ", direction=";

    if (direction_ == INPUT)
    {
        ret += "INPUT";
    }
    else
    {
        ret += "OUTPUT";
    }

    ret += ", label=\"";
    ret += label_;
    ret += "\", connected=";

    if (connected())
    {
        ret += "true";
    }
    else
    {
        ret += "false";
    }

    ret += ')';

    return ret;
}
