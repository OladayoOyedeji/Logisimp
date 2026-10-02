// Wire.h
/**
 * Connects component ports and carries a logic value.
 * Ports are like nodes and wires are like (hyper)edges.
 * (you can also think of components as nodes and ports as "border checkpoints"
 * on the "roads" (arcs) to other countries)
 */

#ifndef WIRE_H
#define WIRE_H

#include "LogicVector.h"
#include "Port.h"

#include <ostream>
#include <string>
#include <vector>

class Port;

class Wire
{
public:
    Wire(int id, int width = 1, const std::string & label = "")
        : id_(id), label_(label), value_(width, UNKNOWN), driver_(nullptr)
    {}

    // prevent lvalue copy
    // ex:
    // Wire a(0);
    // Wire b(a); <---- ILLEGAL, lvalue -> copy constructor
    // b = a;     <---- ILLEGAL, lvalue -> copy assignment
    Wire(const Wire & other) = delete;
    Wire & operator=(const Wire & other) = delete;
    
    // prevent rvalue move
    // ex:
    // Wire a(Wire(0)) <---- ILLEGAL, rvalue -> move constructor
    // Wire a(0)
    // a = Wire(1)     <---- ILLEGAL, rvalue -> move assignment 
    Wire(Wire && other) = delete;
    Wire & operator=(Wire && other) = delete;

    // int & id() { return id_; } // wire's id should remain stable
    int id() const { return id_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    LogicVector & value() { return value_; }
    LogicVector value() const { return value_; }

    Port *& driver() { return driver_; }
    Port * driver() const { return driver_; }

    std::vector< Port * > & listeners() { return listeners_; }
    std::vector< Port * > listeners() const { return listeners_; }

    int width() const { return value_.width(); }

    std::string to_string() const;

private:
    int id_;
    std::string label_;
    LogicVector value_;
    Port * driver_;
    std::vector< Port * > listeners_;
};

inline
std::ostream & operator<<(std::ostream & cout, const Wire & wire)
{
    cout << wire.to_string();
    return cout;
}

#endif // Wire.h
