// Component.h
/**
 * Base class for all circuit components.
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <iosfwd>
#include <string>
#include <vector>

class Port;
class Simulator;

class Component
{
public:
    Component(int id, const std::string & label = "")
        : id_(id), label_(label)
    {}

    // Don't allow copying.
    // Copying should be managed by daddy Circuit
    //----------------------------------------------
    // prevent lvalue copy
    // ex:
    // Component a(0);
    // Component b(a); <---- ILLEGAL, lvalue -> copy constructor
    // b = a;          <---- ILLEGAL, lvalue -> copy assignment
    Component(const Component & other) = delete;
    Component & operator=(const Component & other) = delete;

    // prevent rvalue move
    // ex:
    // Component a(Component(0)) <---- ILLEGAL, rvalue -> move constructor
    // Component a(0)
    // a = Component(1)          <---- ILLEGAL, rvalue -> move assignment
    Component(Component && other) = delete;
    Component & operator=(Component && other) = delete;

    virtual ~Component()
    {
        clear_ports();
    }

    int & id() { return id_; }
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

    virtual std::string to_string() const;

    Port * input_port(int i);
    Port * input_port(int i) const;
    Port * output_port(int i);
    Port * output_port(int i) const;

protected:
    Port * add_input(int width = 1, const std::string & label= "");
    Port * add_output(int width = 1, const std::string & label = "");

private:
    void clear_ports();
    
    int id_;
    std::string label_;
    std::vector< Port * > inputs_;
    std::vector< Port * > outputs_;
};

inline std::ostream & operator<<(std::ostream & cout, const Component & component)
{
    cout << component.to_string();
    return cout;
}

#endif // Component.h
