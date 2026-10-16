/**
 * @file Component.cpp
 * @brief Implements the structure shared by circuit components.
 * @author Cole Schwandt
 */

#include "Component.h"
#include "Port.h"

#include <algorithm>
#include <limits>
#include <ostream>
#include <stdexcept>

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

int Component::resolve_port_id(int port_id) const
{
    if (port_id == -1)
    {
        port_id = next_port_id_;
    }

    if (port_id < 0 || port_id == std::numeric_limits< int >::max())
    {
        throw std::invalid_argument("Invalid port ID");
    }

    // Both directions share one ID namespace: a blueprint Pin ID must identify
    // exactly one placement port even when that pin changes direction.
    for (Port * port : inputs_)
    {
        if (port->id() == port_id)
        {
            throw std::invalid_argument("Port ID is already in use");
        }
    }

    for (Port * port : outputs_)
    {
        if (port->id() == port_id)
        {
            throw std::invalid_argument("Port ID is already in use");
        }
    }

    return port_id;
}

Port * Component::add_input(int width, const std::string & label, int port_id)
{
    port_id = resolve_port_id(port_id);

    Port * port = new Port(port_id, width, INPUT, label);
    port->owner_ = this;

    try
    {
        inputs_.push_back(port);
    }
    catch (...)
    {
        delete port;
        throw;
    }

    // Explicit IDs may come from blueprint pins; keep automatic IDs above them
    // so later ordinary ports cannot collide with the synchronized interface.
    if (port_id >= next_port_id_)
    {
        next_port_id_ = port_id + 1;
    }

    return port;
}

Port * Component::add_output(int width, const std::string & label, int port_id)
{
    port_id = resolve_port_id(port_id);

    Port * port = new Port(port_id, width, OUTPUT, label);
    port->owner_ = this;

    try
    {
        outputs_.push_back(port);
    }
    catch (...)
    {
        delete port;
        throw;
    }

    // Explicit blueprint Pin IDs also reserve the automatic allocation range.
    if (port_id >= next_port_id_)
    {
        next_port_id_ = port_id + 1;
    }

    return port;
}

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

void Component::remove_port(Port & port)
{
    std::vector< Port * >::iterator position =
        std::find(inputs_.begin(), inputs_.end(), &port);

    if (position != inputs_.end())
    {
        // The owning Circuit must detach first; deleting a connected port would
        // leave a dangling attachment pointer in its node.
        if (port.node() != nullptr)
        {
            throw std::logic_error("Disconnect port before removing it");
        }

        inputs_.erase(position);
        delete &port;
        return;
    }

    position = std::find(outputs_.begin(), outputs_.end(), &port);

    if (position != outputs_.end())
    {
        if (port.node() != nullptr)
        {
            throw std::logic_error("Disconnect port before removing it");
        }

        outputs_.erase(position);
        delete &port;
        return;
    }

    throw std::invalid_argument("Port does not belong to this component");
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
