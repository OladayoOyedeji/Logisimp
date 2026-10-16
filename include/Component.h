/**
 * @file Component.h
 * @brief Abstract class for all circuit components.
 * @author Cole Schwandt
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <ostream>
#include <string>
#include <vector>

class Circuit;
class Port;
class Simulator;
class CircuitState;
class ComponentState;

class Component
{
public:
    Component(int id, const std::string & label = "")
        : id_(id), label_(label), next_port_id_(0)
    {}

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
    const std::vector< Port * > & inputs() const { return inputs_; }
    const std::vector< Port * > & outputs() const { return outputs_; }

    int input_count() const;
    int output_count() const;
    
    Port * input_port(int i);
    Port * input_port(int i) const;
    Port * output_port(int i);
    Port * output_port(int i) const;

    virtual ComponentState * create_state(CircuitState &) const
    {
        return nullptr;
    }

    virtual void evaluate(Simulator & simulator, CircuitState & state) = 0;

    virtual std::string to_string() const;

protected:
    Port * add_input(int width = 1, const std::string & label = "", int port_id = -1);
    Port * add_output(int width = 1, const std::string & label = "", int port_id = -1);

private:
    friend class Circuit;

    int resolve_port_id(int port_id) const;
    void remove_port(Port & port);
    void clear_ports();
    
    int id_;
    std::string label_;
    std::vector< Port * > inputs_;
    std::vector< Port * > outputs_;
    int next_port_id_;
};

inline std::ostream & operator<<(std::ostream & cout, const Component & component)
{
    cout << component.to_string();
    return cout;
}

#endif // COMPONENT_H
