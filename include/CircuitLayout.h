/**
 * @file CircuitLayout.h
 * @brief Project layout data kept separate from electrical simulation.
 */

#ifndef CIRCUIT_LAYOUT_H
#define CIRCUIT_LAYOUT_H

#include "Handles.h"

#include <vector>

struct CanvasPosition
{
    double x = 0;
    double y = 0;
    bool operator==(CanvasPosition other) const { return x == other.x && y == other.y; }
    bool operator!=(CanvasPosition other) const { return !(*this == other); }
};

struct ComponentLayout
{
    CircuitId circuit;
    ComponentId component;
    CanvasPosition position;
};

struct WireNodeLayout
{
    CircuitId circuit;
    WireNodeId node;
    CanvasPosition position;
};

/**
 * @brief Copyable layout model shared by the editor tabs of one project.
 *
 * Coordinates have no electrical meaning. Setters reject nonfinite positions
 * and invalid IDs; callers validate that objects belong to their circuit. This
 * model does not retain references to circuits and does not observe mutations.
 * EditorController removes its deleted components and refreshes stale component
 * and node records. Other layout callers manage their own stale records.
 *
 * Inspection returns value snapshots suitable for a future project serializer.
 * No persistence format, hit-testing, orientation or wire routing is imposed.
 * Component IDs carry session provenance; circuit and wire-node IDs remain
 * contextual, so a layout must not be shared between unrelated projects.
 */
class CircuitLayout
{
public:
    void set_component_position(CircuitId circuit, ComponentId component, CanvasPosition position);
    /** @brief Copies a recorded position; returns false without changing position when absent. */
    bool component_position(CircuitId circuit, ComponentId component, CanvasPosition & position) const;
    void remove_component(CircuitId circuit, ComponentId component) noexcept;
    std::vector< ComponentLayout > components(CircuitId circuit) const;

    void set_node_position(CircuitId circuit, WireNodeId node, CanvasPosition position);
    /** @brief Copies a recorded position; returns false without changing position when absent. */
    bool node_position(CircuitId circuit, WireNodeId node, CanvasPosition & position) const;
    void remove_node(CircuitId circuit, WireNodeId node) noexcept;
    std::vector< WireNodeLayout > nodes(CircuitId circuit) const;

    void clear(CircuitId circuit) noexcept;
    void swap(CircuitLayout & other) noexcept;

private:
    std::vector< ComponentLayout > components_;
    std::vector< WireNodeLayout > nodes_;
};

#endif // CIRCUIT_LAYOUT_H
