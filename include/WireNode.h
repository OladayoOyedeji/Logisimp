/**
 * @file WireNode.h
 * @brief Connection point for wire edges and ports in a circuit.
 * @details Vertex in the wire graph
 * @see Net, WireEdge
 * @author Cole Schwandt
 */

#ifndef WIRE_NODE_H
#define WIRE_NODE_H

#include <ostream>
#include <string>
#include <vector>

class Circuit;
class Net;
class Port;
class WireEdge;

/**
 * @brief Connection point (vertex) for @ref WireEdge "wire edges" 
 * and @ref Port "ports" in a circuit's wire graph.
 */
class WireNode
{
public:
    /** @brief Wire nodes cannot be copied. */
    WireNode(const WireNode & other) = delete;

    /** @brief Wire nodes cannot be assigned. */
    WireNode & operator=(const WireNode & other) = delete;

    /** @brief Wire nodes cannot be moved. */
    WireNode(WireNode && other) = delete;

    /** @brief Wire nodes cannot be move-assigned */
    WireNode & operator=(WireNode && other) = delete;

    /** @brief Destroys the wire node. */
    ~WireNode() {}

    /**
     * @brief Gets the wire node's identifier.
     * @return Copy of the identifier.
     */
    int id() const { return id_; }

    /**
     * @brief Gets the circuit containing this wire node.
     * @return Pointer to the circuit containing this wire node.
     */
    Circuit * circuit() const { return circuit_; }

    /**
     * @brief Gets the wire node's bit width.
     * @return Number of bits.
     */
    int width() const { return width_; }

    /**
     * @brief Accesses the wire node's label.
     * @return Reference to the label.
     */
    std::string & label() { return label_; }

    /**
     * @brief Gets the wire node's label.
     * @return Copy of the label.
     */
    std::string label() const { return label_; }

    /**
     * @brief Gets the wire edges connected to this node.
     * @return Const reference to the vector of wire edge pointers.
     */
    const std::vector< WireEdge * > & edges() const { return edges_; }

    /**
     * @brief Gets the ports attached to this node.
     * @return Const reference to the vector of port pointers.
     */
    const std::vector< Port * > & ports() const { return ports_; }

    /**
     * @brief Gets the net associated with this wire node.
     * @return Pointer to the net, or nullptr if none is assigned.
     */
    Net * net() const { return net_; }

    /**
     * @brief Converts the wire node to its string representation.
     * @return String representation of the wire node.
     */
    std::string to_string() const;

private:
    friend class Circuit;
    friend class ProjectEdit;

    /**
     * @brief Constructs a wire node belonging to a circuit.
     * @param circuit Circuit containing the wire node.
     * @param id Unique wire node identifier.
     * @param width Bit-width constraint; 0 means unconstrained, allowing any
     * otherwise compatible signal width. The constraint stays unchanged on
     * connection; Net::width() reports the resolved width of the connected region.
     * @param label Optional descriptive label.
     */
    WireNode(Circuit & circuit, int id, int width = 0, const std::string & label = "");

    int id_; ///< Unique wire node identifier.
    Circuit * circuit_; ///< Circuit containing this wire node.
    int width_; ///< Bit width, or 0 if unconstrained.
    std::string label_; ///< Optional descriptive label.
    std::vector< WireEdge * > edges_; ///< Wire edges connected to this node.
    std::vector< Port * > ports_; ///< Ports attached to this node.
    Net * net_; ///< Associated net, or nullptr if none.
};

/**
 * @brief Writes a wire node to an output stream.
 * @param cout Output stream.
 * @param node Wire node to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const WireNode & node)
{
    cout << node.to_string();
    return cout;
}

#endif // WIRE_NODE_H
