// Signal.h

#ifndef SIGNAL_H
#define SIGNAL_H

#include "LogicVector.h"
#include "Port.h"

#include <algorithm>
#include <string>
#include <vector>

class Signal
{
public:
    Signal(int id, int width = 1)
        : id_(id),
          value_(width),
          driver_(nullptr)
    {
    }

    Signal(const Signal & other) = delete;
    Signal & operator=(const Signal & other) = delete;

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

    void add_listener(Port * port)
    {
        if (std::find(listeners_.begin(), listeners_.end(), port) == listeners_.end())
        {
            listeners_.push_back(port);
        }
    }

    void remove_listener(Port * port)
    {
        std::vector< Port * >::iterator position = std::find(listeners_.begin(), listeners_.end(), port);

        if (position != listeners_.end())
        {
            listeners_.erase(position);
        }
    }

private:
    int id_;
    std::string label_;
    LogicVector value_;
    Port * driver_;
    std::vector< Port * > listeners_;
};

#endif // Signal.h
