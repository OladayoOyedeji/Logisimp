/**
 * @file Handles.h
 * @brief Value identifiers for project and circuit editing APIs.
 */

#ifndef HANDLES_H
#define HANDLES_H

#include <memory>

/** @brief A typed identifier; -1 denotes an invalid identifier. */
template< typename Tag > struct ObjectId
{
    explicit ObjectId(int value = -1) : value(value) {}
    bool operator==(ObjectId other) const { return value == other.value; }
    bool operator!=(ObjectId other) const { return value != other.value; }
    int value;
};

struct CircuitIdTag;
struct ComponentIdTag;
struct WireNodeIdTag;
struct WireEdgeIdTag;
struct RuntimeIdTag;

using CircuitId = ObjectId< CircuitIdTag >;

/**
 * @brief A component identity, optionally qualified by its circuit and controller.
 * @note Session-issued handles carry provenance. Explicit integer construction
 * remains available for legacy circuit-local lookups; it produces an unscoped
 * handle and does not compare equal to a qualified handle with the same value.
 */
template< > struct ObjectId< ComponentIdTag >
{
    explicit ObjectId(int value = -1) : value(value) {}

    bool operator==(const ObjectId & other) const
    {
        return value == other.value && circuit_ == other.circuit_ &&
               !owner_.owner_before(other.owner_) && !other.owner_.owner_before(owner_);
    }
    bool operator!=(const ObjectId & other) const { return !(*this == other); }

    /** @brief Reports whether this handle includes circuit/controller provenance. */
    bool is_scoped() const { return circuit_.value >= 0; }

    /** @brief Checks provenance against a live controller and circuit. */
    bool matches(CircuitId circuit, const std::weak_ptr< int > & owner) const
    {
        return is_scoped() && circuit_ == circuit && !owner_.expired() &&
               !owner_.owner_before(owner) && !owner.owner_before(owner_);
    }

    int value;

private:
    friend class CircuitSession;
    friend class ProjectController;
    ObjectId(int value, CircuitId circuit, const std::weak_ptr< int > & owner)
        : value(value), circuit_(circuit), owner_(owner)
    {}

    CircuitId circuit_;
    std::weak_ptr< int > owner_;
};

using ComponentId = ObjectId< ComponentIdTag >;
using WireNodeId = ObjectId< WireNodeIdTag >;
using WireEdgeId = ObjectId< WireEdgeIdTag >;
using RuntimeId = ObjectId< RuntimeIdTag >;

/**
 * @brief A component-local port identity, including its creation generation.
 * @note Reordering preserves the handle. Replacing a port invalidates it even
 * when a placement reuses the same blueprint interface pin ID.
 */
struct PortId
{
    PortId(ComponentId component = ComponentId(), int value = -1, int generation = -1) : component(component), value(value), generation(generation) {}
    bool operator==(PortId other) const { return component == other.component && value == other.value && generation == other.generation; }
    bool operator!=(PortId other) const { return !(*this == other); }
    ComponentId component;
    int value;
    int generation;
};

#endif // HANDLES_H
