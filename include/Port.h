/**
 * @file Port.h
 * @brief Connection point between a component and a wire node.
 * @author Cole Schwandt
 */

#ifndef PORT_H
#define PORT_H

#include <ostream>
#include <string>

class Circuit;
class Component;
class WireNode;
class Net;

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

    Port(Port && other) = delete;
    Port & operator=(Port && other) = delete;

    int id() const { return id_; }

    int width() const { return width_; }

    PortDirection direction() const { return direction_; }

    Component * owner() const { return owner_; }

    WireNode * node() const { return node_; }

    Net * net() const { return net_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    bool connected() const { return node_ != nullptr; }

    std::string to_string() const;

private:
    friend class Circuit;
    friend class Component;
    
    int id_;
    int width_;
    PortDirection direction_;
    Component * owner_;
    WireNode * node_;
    Net * net_;
    std::string label_;
};

inline std::ostream & operator<<(std::ostream & cout, const Port & port)
{
    cout << port.to_string();
    return cout;
}

#endif // PORT_H
