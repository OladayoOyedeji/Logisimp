/**
 * @file WireNode.cpp
 * @brief Implements junctions and endpoints in a circuit's wire graph.
 * @author Cole Schwandt
 */

#include "WireNode.h"

#include <stdexcept>

WireNode::WireNode(Circuit & circuit, int id, int width, const std::string & label)
    : id_(id), circuit_(&circuit), width_(width), label_(label), net_(nullptr)
{
    if (width < 0)
    {
        throw std::invalid_argument("Wire node width must not be negative");
    }
}

std::string WireNode::to_string() const
{
    std::string ret = "WireNode(id=" + std::to_string(id_);
    ret += ", width=" + std::to_string(width_);
    ret += ", label=\"" + label_ + "\", edges=";
    ret += std::to_string(edges_.size());
    ret += ", ports=" + std::to_string(ports_.size()) + ')';
    return ret;
}
