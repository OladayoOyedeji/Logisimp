/**
 * @file CircuitLayout.cpp
 * @brief Stores circuit-local drawing coordinates without simulation state.
 */

#include "CircuitLayout.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
void validate_position(CanvasPosition position)
{
    if (!std::isfinite(position.x) || !std::isfinite(position.y))
    {
        throw std::invalid_argument("Layout coordinates must be finite");
    }
}
}

void CircuitLayout::set_component_position(CircuitId circuit, ComponentId component, CanvasPosition position)
{
    validate_position(position);
    if (circuit.value < 0 || component.value < 0 || !component.is_scoped())
    {
        throw std::invalid_argument("Component layout requires a circuit and a session-issued component ID");
    }
    for (ComponentLayout & entry : components_)
    {
        if (entry.circuit == circuit && entry.component == component)
        {
            entry.position = position;
            return;
        }
    }
    components_.push_back(ComponentLayout{circuit, component, position});
}

bool CircuitLayout::component_position(CircuitId circuit, ComponentId component, CanvasPosition & position) const
{
    for (const ComponentLayout & entry : components_)
    {
        if (entry.circuit == circuit && entry.component == component)
        {
            position = entry.position;
            return true;
        }
    }
    return false;
}

void CircuitLayout::remove_component(CircuitId circuit, ComponentId component) noexcept
{
    components_.erase(std::remove_if(components_.begin(), components_.end(), [&](const ComponentLayout & entry)
    {
        return entry.circuit == circuit && entry.component == component;
    }), components_.end());
}

std::vector< ComponentLayout > CircuitLayout::components(CircuitId circuit) const
{
    std::vector< ComponentLayout > result;
    for (const ComponentLayout & entry : components_)
    {
        if (entry.circuit == circuit)
        {
            result.push_back(entry);
        }
    }
    return result;
}

void CircuitLayout::set_node_position(CircuitId circuit, WireNodeId node, CanvasPosition position)
{
    validate_position(position);
    if (circuit.value < 0 || node.value < 0)
    {
        throw std::invalid_argument("Wire-node layout requires valid circuit and node IDs");
    }
    for (WireNodeLayout & entry : nodes_)
    {
        if (entry.circuit == circuit && entry.node == node)
        {
            entry.position = position;
            return;
        }
    }
    nodes_.push_back(WireNodeLayout{circuit, node, position});
}

bool CircuitLayout::node_position(CircuitId circuit, WireNodeId node, CanvasPosition & position) const
{
    for (const WireNodeLayout & entry : nodes_)
    {
        if (entry.circuit == circuit && entry.node == node)
        {
            position = entry.position;
            return true;
        }
    }
    return false;
}

void CircuitLayout::remove_node(CircuitId circuit, WireNodeId node) noexcept
{
    nodes_.erase(std::remove_if(nodes_.begin(), nodes_.end(), [&](const WireNodeLayout & entry)
    {
        return entry.circuit == circuit && entry.node == node;
    }), nodes_.end());
}

std::vector< WireNodeLayout > CircuitLayout::nodes(CircuitId circuit) const
{
    std::vector< WireNodeLayout > result;
    for (const WireNodeLayout & entry : nodes_)
    {
        if (entry.circuit == circuit)
        {
            result.push_back(entry);
        }
    }
    return result;
}

void CircuitLayout::clear(CircuitId circuit) noexcept
{
    components_.erase(std::remove_if(components_.begin(), components_.end(), [&](const ComponentLayout & entry)
    {
        return entry.circuit == circuit;
    }), components_.end());
    nodes_.erase(std::remove_if(nodes_.begin(), nodes_.end(), [&](const WireNodeLayout & entry)
    {
        return entry.circuit == circuit;
    }), nodes_.end());
}

void CircuitLayout::swap(CircuitLayout & other) noexcept
{
    components_.swap(other.components_);
    nodes_.swap(other.nodes_);
}
