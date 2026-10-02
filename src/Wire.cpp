// Wire.cpp

#include "Wire.h"

std::string Wire::to_string() const
{
    std::string ret;

    ret += "Wire(id=";
    ret += std::to_string(id_);
    ret += ", width=";
    ret += std::to_string(width());
    ret += ", value=";
    ret += value_.to_string();
    ret += ", label=\"";
    ret += label_;
    ret += "\", driver=";

    if (driver_ == nullptr)
    {
        ret += "none";
    }
    else
    {
        ret += "port ";
        ret += std::to_string(driver_->id());
    }

    ret += ", listeners=";
    ret += std::to_string(listeners_.size());
    ret += ')';

    return ret;
}
