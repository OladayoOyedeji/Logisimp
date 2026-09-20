// Component.h

#ifndef COMPONENT_H
#define COMPONENT_H

#include "Port.h"

#include <string>
#include <vector>
#include <iostream>

class Simulator;

class Component
{
public:
    Component(int id, const std::string & label = "")
        : id_(id), label_(label)
    {}

    // Don't allow copying.
    // Copying should be managed by daddy Circuit
    Component(const Component & other) = delete;
    Component & operator=(const Component & other) = delete;

    Component(Component && other) = delete;
    Component & operator=(Component && other) = delete;

    virtual ~Component()
    {
        clear_ports();
    }

    int id() const { return id_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    std::vector< Port * > & inputs() { return inputs_; }
    std::vector< Port * > inputs() const { return inputs_; }

    std::vector< Port * > & outputs() { return outputs_; }
    std::vector< Port * > outputs() const { return outputs_; }

    virtual void evaluate(Simulator & simulator) = 0;

    virtual void reset()
    {}

    virtual void print(std::ostream & cout) const
    {
        cout << "Component("
             << "id=" << id_
             << ", label=\"" << label_ << "\""
             << ", inputs=" << inputs_.size()
             << ", outputs=" << outputs_.size()
             << ')';
    }

// protected:
    Port * add_input(int width = 1, const std::string & label = "")
    {
        int port_id = inputs_.size() + outputs_.size();

        Port * port = new Port(port_id, width, INPUT, label);
        port->owner() = this;

        inputs_.push_back(port);

        return port;
    }

    Port * add_output(int width = 1, const std::string & label = "")
    {
        int port_id = inputs_.size() + outputs_.size();

        Port * port = new Port(port_id, width, OUTPUT, label);
        port->owner() = this;

        outputs_.push_back(port);

        return port;
    }

    Port * input_port(int i)
    {
        return inputs_.at(i); // at is [] but does bounds-checking
        // (throws std::out_of_range error if out of bounds)
    }

    Port * input_port(int i) const
    {
        return inputs_.at(i);
    }

    Port * output_port(int i)
    {
        return outputs_.at(i);
    }

    Port * output_port(int i) const
    {
        return outputs_.at(i);
    }

    int input_count() const
    {
        return inputs_.size();
    }

    int output_count() const
    {
        return outputs_.size();
    }

private:
    void clear_ports()
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

    int id_;
    std::string label_;
    std::vector< Port * > inputs_;
    std::vector< Port * > outputs_;
};

inline std::ostream & operator<<(std::ostream & cout, const Component & component)
{
    component.print(cout);

    return cout;
}

#endif // Component.h
