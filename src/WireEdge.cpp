/**
 * @file WireEdge.cpp
 * @brief Implements connections between two wire nodes.
 * @author Cole Schwandt
 */

#include "WireEdge.h"
#include "WireNode.h"

WireEdge::WireEdge(Circuit & circuit, int id, WireNode & first, WireNode & second)
    : id_(id), circuit_(&circuit), first_(&first), second_(&second)
{}

std::string WireEdge::to_string() const
{
    std::string ret = "WireEdge(id=" + std::to_string(id_);
    ret += ", first=" + std::to_string(first_->id());
    ret += ", second=" + std::to_string(second_->id()) + ')';
    return ret;
}
