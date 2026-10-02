// LogicVector.cpp

#include "LogicVector.h"

#include <stdexcept>

LogicVector::LogicVector(int width, LogicValue initial_value)
{
    if (width <= 0)
    {
        throw std::invalid_argument("LogicVector width must be positive");
    }

    bits_.resize(width, initial_value);
}

LogicValue & LogicVector::operator[](int index)
{
    return bits_.at(index);
}

LogicValue LogicVector::operator[](int index) const
{
    return bits_.at(index);
}

bool LogicVector::operator==(const LogicVector & other) const
{
    return bits_ == other.bits_;
}

bool LogicVector::operator!=(const LogicVector & other) const
{
    return bits_ != other.bits_;
}

bool LogicVector::fully_known() const
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

std::string LogicVector::to_string() const
{
    std::string ret;

    ret += '[';

    for (int i = 0; i < width(); i++)
    {
        if (i != 0)
        {
            ret += ' ';
        }

        ret += ::to_string(bits_[i]);
    }

    ret += ']';

    return ret;
}
