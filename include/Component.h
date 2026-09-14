// Component.h

#ifndef COMPONENT_H
#define COMPONENT_H

#include "Port.h"

#include <stdexcept>
#include <string>
#include <vector>

class Simulator;

class Component
{
public:
    Component(int id)
        : id_(id)
    {
    }

    virtual ~Component()
    {
    }

    Component(const Component & other) = delete;
    Component & operator=(const Component & other) = delete;

    int & id() { return id_; }
    int id() const { return id_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    std::vector< Port * > & ports() { return ports_; }
    std::vector< Port * > ports() const { return ports_; }

    Port * port_by_id(int id) const
    {
        for (Port * port : ports_)
        {
            if (port->id() == id)
            {
                return port;
            }
        }

        return nullptr;
    }

    virtual void evaluate(Simulator & simulator) = 0;

    virtual void reset()
    {
    }

protected:
    void add_port(Port & port)
    {
        if (port.owner() != nullptr)
        {
            throw std::runtime_error("Port already belongs to a component");
        }

        for (Port * existing_port : ports_)
        {
            if (existing_port->id() == port.id())
            {
                throw std::runtime_error("Component contains duplicate port IDs");
            }
        }

        port.owner() = this;
        ports_.push_back(&port);
    }

private:
    int id_;
    std::string label_;
    std::vector< Port * > ports_;
};

#endif // Component.h
