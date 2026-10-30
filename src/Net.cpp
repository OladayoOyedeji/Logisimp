/**
 * @file Net.cpp
 * @brief Implements the electrical connections derived from a circuit's wire graph.
 * @author Cole Schwandt
 */

#include "Net.h"
#include "Port.h"
#include "WireNode.h"

#include <algorithm>
#include <stdexcept>

Net::Net(Circuit & circuit, int id, int width)
    : id_(id), circuit_(&circuit), width_(width), width_mismatch_(false)
{
    if (width <= 0)
    {
        throw std::invalid_argument("Net width must be positive");
    }
}

LogicVector Net::resolve(const std::unordered_map< const Port *, LogicVector > & output_values) const
{
    LogicVector resolved(width_, UNKNOWN);
    if (width_mismatch_ || drivers_.empty())
    {
        return resolved;
    }

    for (const Port * driver : drivers_)
    {
        const LogicVector & value = output_values.at(driver);
        if (value.width() != width_)
        {
            throw std::logic_error("Driver cache width does not match its net");
        }
        for (int bit = 0; bit < width_; ++bit)
        {
            // An undetermined contribution does not override another driver's
            // known value. ERROR is absorbing, including an earlier conflict.
            if (value[bit] == UNKNOWN)
            {
                continue;
            }
            if (resolved[bit] == UNKNOWN)
            {
                resolved[bit] = value[bit];
            }
            else if (resolved[bit] != value[bit])
            {
                resolved[bit] = ERROR;
            }
        }
    }
    return resolved;
}

std::string Net::label() const
{
    std::string ret;
    std::vector< std::string > labels;
    for (const WireNode * node : nodes_)
    {
        std::string node_label = node->label();
        if (node_label.empty() || std::find(labels.begin(), labels.end(), node_label) != labels.end())
        {
            continue;
        }
        if (!ret.empty())
        {
            ret += " / ";
        }
        ret += node_label;
        labels.push_back(node_label);
    }
    return ret;
}

std::string Net::to_string() const
{
    std::string ret;

    ret += "Net(id=";
    ret += std::to_string(id_);
    ret += ", width=";
    ret += std::to_string(width_);
    ret += ", label=\"";
    ret += label();
    ret += "\", drivers=";
    ret += std::to_string(drivers_.size());
    ret += ", width_mismatch=";
    ret += width_mismatch_ ? "true" : "false";
    ret += ", driver=";

    if (driver() == nullptr)
    {
        ret += drivers_.empty() ? "none" : "multiple";
    }
    else
    {
        ret += "port ";
        ret += std::to_string(driver()->id());
    }

    ret += ", listeners=";
    ret += std::to_string(listeners_.size());
    ret += ", nodes=";
    ret += std::to_string(nodes_.size());
    ret += ')';

    return ret;
}
