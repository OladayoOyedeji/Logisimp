/**
 * @file Net.h
 * @brief Describes the electrical connections derived from a circuit's wire graph.
 * @author Cole Schwandt
 */

#ifndef NET_H
#define NET_H

#include "LogicVector.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class Circuit;
class Port;
class WireNode;

/**
 * @brief Represents a logical connection between electrically connected ports and wire nodes.
 *
 * @details
 *
 * @par Purpose
 * During simulation, the simulator needs to determine which input ports are affected
 * whenever a component changes its output. Without nets, the simulator would need to traverse the wire graph each
 * time a signal changes to discover all connected input ports.
 *
 * A Net stores this connectivity information in advance. It groups
 * electrically connected wire nodes and ports, identifies the output ports
 * driving the signal, and stores the input ports receiving it. This allows
 * the simulator to find affected components directly through listeners(),
 * without repeatedly traversing the wire graph.
 *
 * @par Example
 * Consider an output port A connected to two input ports B and C through
 * several wire nodes and edges:
 *
 * @dot "One electrical net"
 * graph net_example
 * {
 *     graph [rankdir=LR, bgcolor="transparent", nodesep=0.5, ranksep=0.7];
 *     node [fontname="Helvetica", fontsize=11];
 *     edge [color="#2563eb", penwidth=2];
 *
 *     output_a [label="Output A\n8 bits", shape=box, style=rounded];
 *     input_b [label="Input B\n8 bits", shape=box, style=rounded];
 *     input_c [label="Input C\n8 bits", shape=box, style=rounded];
 *
 *     node [shape=circle, width=0.5, fixedsize=true];
 *     n1 [label="N1"];
 *     n2 [label="N2"];
 *     n3 [label="N3"];
 *     n4 [label="N4"];
 *
 *     output_a -- n1;
 *     n1 -- n2;
 *     n2 -- n3;
 *     n2 -- n4;
 *     n3 -- input_b;
 *     n4 -- input_c;
 * }
 * @enddot
 *
 * Although this graph contains four wire nodes and several wire edges,
 * all three ports belong to one Net because they are electrically connected.
 *
 * The resulting net has:
 * - Driver: Output A.
 * - Listeners: Input B and Input C.
 * - Width: 8 bits.
 * - Nodes: N1, N2, N3, and N4.
 *
 * If Output A changes its value, the simulator can access the affected
 * input ports directly:
 *
 * @code{.cpp}
 * for (Port * port : net.listeners())
 * {
 *     Component * component = port->owner();
 *     // Schedule the component for evaluation.
 * }
 * @endcode
 *
 * This avoids traversing N1, N2, N3, and N4 each time the output changes.
 * The benefit is especially significant when signals change frequently
 * but the circuit's wiring remains unchanged.
 *
 * @par Net Construction
 * Nets are derived from the circuit's wire graph by Circuit::rebuild_nets().
 * This function identifies connected groups of wire nodes, collects their
 * attached ports, determines the net width, and identifies drivers and
 * listeners.
 *
 * Each connected group containing at least one port produces a Net.
 * Bare geometry also produces a Net when positive node width constraints
 * conflict, so the fault can be inspected before attaching any ports.
 *
 * Connections are retained when their declared widths disagree or several
 * outputs drive the same net. width_mismatch() reports incompatible positive
 * widths; width() is then the maximum declared width used for runtime storage.
 * Driver values are resolved independently for each runtime circuit instance.
 * A net with no output drivers resolves to UNKNOWN.
 *
 * @par Runtime State
 * A Net describes electrical connectivity, not the current signal value.
 * Runtime logic values are managed separately by the simulation state.
 *
 * @note Nets are rebuilt when wire connectivity changes. Existing Net
 * pointers may become invalid after a rebuild.
 */
class Net
{
public:
    /** @brief Nets cannot be copied. */
    Net(const Net & other) = delete;

    /** @brief Nets cannot be assigned. */
    Net & operator=(const Net & other) = delete;

    /** @brief Nets cannot be moved. */
    Net(Net && other) = delete;

    /** @brief Nets cannot be move-assigned. */
    Net & operator=(Net && other) = delete;

    /** @brief Destroys the net. */
    ~Net() {}

    /**
     * @brief Gets the net's identifier.
     * @return Copy of the identifier.
     */
    int id() const { return id_; }

    /**
     * @brief Gets the circuit containing this net.
     * @return Pointer to the circuit.
     */
    Circuit * circuit() const { return circuit_; }

    /**
     * @brief Gets the net's label.
     * @return Copy of the label.
     * @todo Add label support.
     */
    std::string label() const;

    /**
     * @brief Gets the sole port driving this net.
     * @return Pointer to the driver, or nullptr when there are zero or multiple drivers.
     * @note Use drivers() to distinguish an undriven net from a shared net.
     */
    Port * driver() const { return drivers_.size() == 1 ? drivers_[0] : nullptr; }

    /**
     * @brief Gets every output port attached to this net.
     * @return Borrowed driver ports in deterministic wire traversal order.
     */
    const std::vector< Port * > & drivers() const { return drivers_; }

    /** @brief Reports conflicting positive node or port width declarations. */
    bool width_mismatch() const { return width_mismatch_; }

    /**
     * @brief Resolves the current per-output values of one circuit instance.
     * @param output_values Last driven values indexed by output port.
     * @return UNKNOWN for incompatible widths or undriven bits; ERROR for conflicting known drives.
     * @note UNKNOWN driver bits are neutral: a known drive resolves an otherwise unknown bit.
     * @note Does not modify individual driver values or schedule evaluations.
     */
    LogicVector resolve(const std::unordered_map< const Port *, LogicVector > & output_values) const;

    /**
     * @brief Gets the input ports that receive signals from this net.
     * @return Const reference to the vector of listener port pointers.
     */
    const std::vector< Port * > & listeners() const { return listeners_; }

    /**
     * @brief Gets all ports connected to this net.
     * @return Const reference to the vector of port pointers.
     */
    const std::vector< Port * > & ports() const { return ports_; }

    /**
     * @brief Gets all wire nodes belonging to this net.
     * @return Const reference to the vector of wire node pointers.
     */
    const std::vector< WireNode * > & nodes() const { return nodes_; }

    /**
     * @brief Gets the net's bit width.
     * @return Copy of the number of bits.
     */
    int width() const { return width_; }

    /**
     * @brief Converts the net to its string representation.
     * @return String representation of the net.
     */
    std::string to_string() const;

private:
    friend class Circuit;

    /**
     * @brief Constructs a net belonging to a circuit.
     * @param circuit Circuit containing the net.
     * @param id Unique net identifier.
     * @param width Number of bits carried by the net.
     */
    Net(Circuit & circuit, int id, int width);

    int id_; ///< Unique net identifier.
    Circuit * circuit_; ///< Circuit containing this net.
    int width_; ///< Runtime width, using the largest declared width when incompatible.
    bool width_mismatch_; ///< Conflicting positive width declarations in this connected region.
    std::vector< Port * > drivers_; ///< Output ports driving this net.
    std::vector< Port * > listeners_; ///< Input ports receiving signals from this net.
    std::vector< Port * > ports_; ///< All ports connected to this net.
    std::vector< WireNode * > nodes_; ///< Wire nodes belonging to this net.
};

/**
 * @brief Writes a net to an output stream.
 * @param cout Output stream.
 * @param net Net to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const Net & net)
{
    cout << net.to_string();
    return cout;
}

#endif // NET_H
