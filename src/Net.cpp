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
    : id_(id), circuit_(&circuit), width_(width), driver_(nullptr)
{
    if (width <= 0)
    {
        throw std::invalid_argument("Net width must be positive");
    }
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
    ret += ", nodes=";
    ret += std::to_string(nodes_.size());
    ret += ')';

    return ret;
}
