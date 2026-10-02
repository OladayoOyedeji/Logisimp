// Port.h
/**
 * Connection point between a component and a wire.
 * Ports are like nodes and wires are like (hyper)edges.
 * (you can also think of components as nodes and ports as "border checkpoints"
 * on the "roads" (arcs) to other countries (components))
 */

#ifndef PORT_H
#define PORT_H

#include <ostream>
#include <string>

class Component;
class Wire;

enum PortDirection
{
    INPUT,
    OUTPUT
};

class Port
{
public:
    Port(int id, int width, PortDirection direction, const std::string & label = "");

    Port(const Port & other) = delete;
    Port & operator=(const Port & other) = delete;

    int & id() { return id_; }
    int id() const { return id_; }

    int & width() { return width_; }
    int width() const { return width_; }

    PortDirection & direction() { return direction_; }
    PortDirection direction() const { return direction_; }

    Component * & owner() { return owner_; }
    Component * owner() const { return owner_; }

    Wire * & wire() { return wire_; }
    Wire * wire() const { return wire_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    bool connected() const { return wire_ != nullptr; }

    std::string to_string() const;

private:
    int id_;
    int width_;
    PortDirection direction_;
    Component * owner_;
    Wire * wire_;
    std::string label_;
};

inline std::ostream & operator<<(std::ostream & cout, const Port & port)
{
    cout << port.to_string();
    return cout;
}

#endif // PORT_H
