// LogicVector.h

#ifndef LOGIC_VECTOR_H
#define LOGIC_VECTOR_H

#include "LogicValue.h"

#include <stdexcept>
#include <vector>

class LogicVector
{
  public:
    LogicVector(int width = 1, LogicValue initial_value = UNKNOWN)
    {
        if (width <= 0)
        {
            throw std::invalid_argument("LogicVector width must be positive");
        }

        bits_.resize(width, initial_value);
    }

    std::vector<LogicValue> & bits() { return bits_; }
    std::vector<LogicValue> bits() const { return bits_; }

    int width() const
    {
        return (int)bits_.size();
    }

    LogicValue operator[](int index) const
    {
        return bits_.at(index);
    }

    LogicValue & operator[](int index)
    {
        return bits_.at(index);
    }

    bool operator==(const LogicVector & other) const
        {
            return bits_ == other.bits_;
        }

    bool operator!=(const LogicVector & other) const
    {
        return bits_ != other.bits_;
    }

    bool fully_known() const
    {
        for (LogicValue bit : bits_)
        {
            if (bit == UNKNOWN)
            {
                return false;
            }
        }

        return true;
    }

  private:
    std::vector<LogicValue> bits_;
};

#endif // LogicVector.h
