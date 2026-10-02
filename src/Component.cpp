// Component.cpp

#include "Component.h"
#include "Port.h"

#include <ostream>

std::string Component::to_string() const
{
    std::string ret;

    ret += "Component(id=";
    ret += std::to_string(id_);
    ret += ", label=\"";
    ret += label_;
    ret += "\", inputs=";
    ret += std::to_string(inputs_.size());
    ret += ", outputs=";
    ret += std::to_string(outputs_.size());
    ret += ')';

    return ret;
}

Port * Component::add_input(int width, const std::string & label)
{
    int port_id = int(inputs_.size() + outputs_.size());

    Port * port = new Port(port_id, width, INPUT, label);
    port->owner() = this;

    inputs_.push_back(port);

    return port;
}

Port * Component::add_output(int width, const std::string & label)
{
    int port_id = int(inputs_.size() + outputs_.size());

    Port * port = new Port(port_id, width, OUTPUT, label);
    port->owner() = this;

    outputs_.push_back(port);

    return port;
}

// maybe replace at with [] once project is finished
// at is slow, but helpful for debugging
Port * Component::input_port(int i)
{
    return inputs_.at(i);
}

Port * Component::input_port(int i) const
{
    return inputs_.at(i);
}

Port * Component::output_port(int i)
{
    return outputs_.at(i);
}

Port * Component::output_port(int i) const
{
    return outputs_.at(i);
}

int Component::input_count() const
{
    return int(inputs_.size());
}

int Component::output_count() const
{
    return int(outputs_.size());
}

void Component::clear_ports()
{
    for (Port * port : inputs_)
    {
        delete port;
    }

    for (Port * port : outputs_)
    {
        delete port;
    }

    inputs_.clear();
    outputs_.clear();
}


