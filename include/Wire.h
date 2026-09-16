// Wire.h

#ifndef WIRE_H
#define WIRE_H

#include "LogicVector.h"
#include "Port.h"

#include <string>
#include <vector>
#include <iostream>

class Wire
{
public:
    Wire(int id, int width = 1, const std::string & label = "")
        : id_(id), label_(label), value_(width, UNKNOWN), driver_(nullptr)
    { }

    Wire(const Wire & other) = delete;
    Wire & operator=(const Wire & other) = delete;

    int & id() { return id_; }
    int id() const { return id_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    LogicVector & value() { return value_; }
    LogicVector value() const { return value_; }

    Port * & driver() { return driver_; }
    Port * driver() const { return driver_; }

    std::vector< Port * > & listeners() { return listeners_; }
    std::vector< Port * > listeners() const { return listeners_; }

    int width() const
    {
        return value_.width();
    }

private:
    int id_;
    std::string label_;
    LogicVector value_;
    Port * driver_;
    std::vector< Port * > listeners_;
};

inline std::ostream & operator<<(std::ostream & cout, const Wire & wire)
{
    cout << "Wire("
         << "id=" << wire.id()
         << ", width=" << wire.width()
         << ", value=" << wire.value()
         << ", label=\"" << wire.label() << "\""
         << ", driver=";

    if (wire.driver() == nullptr)
    {
        cout << "none";
    }
    else
    {
        cout << "port " << wire.driver()->id();
    }

    cout << ", listeners=" << wire.listeners().size()
         << ')';

    return cout;
}

#endif // Wire.h
