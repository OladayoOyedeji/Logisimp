/**
 * @file WireEdge.h
 * @brief Connects two wire nodes in a circuit. Represents an edge between nodes in the wire graph.
 * @author Cole Schwandt
 */

#ifndef WIRE_EDGE_H
#define WIRE_EDGE_H

#include <ostream>
#include <string>

class Circuit;
class WireNode;

/**
 * @brief Represents a connection between two @ref WireNode "wire nodes".
 * @note Wire edges are undirected.
 */
class WireEdge
{
public:
    /** @brief Wire edges cannot be copied. */
    WireEdge(const WireEdge & other) = delete;

    /** @brief Wire edges cannot be assigned. */
    WireEdge & operator=(const WireEdge & other) = delete;

    /** @brief Wire edges cannot be moved. */
    WireEdge(WireEdge && other) = delete;

    /** @brief Wire edges cannot be move-assigned. */
    WireEdge & operator=(WireEdge && other) = delete;

    /** @brief Destroys the wire edge. */
    ~WireEdge() {}

    /**
     * @brief Gets the wire edge's identifier.
     * @return Copy of the identifier.
     */
    int id() const { return id_; }

    /**
     * @brief Gets the circuit containing this wire edge.
     * @return Pointer to the circuit.
     */
    Circuit * circuit() const { return circuit_; }

    /**
     * @brief Gets the first endpoint of the wire edge.
     * @return Pointer to the first wire node.
     */
    WireNode * first() const { return first_; }

    /**
     * @brief Gets the second endpoint of the wire edge.
     * @return Pointer to the second wire node.
     */
    WireNode * second() const { return second_; }

    /**
     * @brief Converts the wire edge to its string representation.
     * @return String representation of the wire edge.
     */
    std::string to_string() const;

private:
    friend class Circuit;

    /**
     * @brief Constructs a wire edge connecting two wire nodes.
     * @param circuit Circuit containing the wire edge.
     * @param id Unique wire edge identifier.
     * @param first First endpoint of the wire edge.
     * @param second Second endpoint of the wire edge.
     */
    WireEdge(Circuit & circuit, int id, WireNode & first, WireNode & second);

    int id_; ///< Unique wire edge identifier.
    Circuit * circuit_; ///< Circuit containing this wire edge.
    WireNode * first_; ///< First endpoint of the wire edge.
    WireNode * second_; ///< Second endpoint of the wire edge.
};

/**
 * @brief Writes a wire edge to an output stream.
 * @param cout Output stream.
 * @param edge Wire edge to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const WireEdge & edge)
{
    cout << edge.to_string();
    return cout;
}

#endif // WIRE_EDGE_H
