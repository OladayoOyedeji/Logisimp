/**
 * @file Port.h
 * @brief Connection point between a component and a wire node.
 * @author Cole Schwandt
 */

#ifndef PORT_H
#define PORT_H

#include <ostream>
#include <string>

class Circuit;
class Component;
class WireNode;
class Net;

/**
 * @brief Specifies whether a port receives or produces signals.
 */
enum PortDirection
{
    INPUT, ///< Receives signals.
    OUTPUT ///< Produces signals.
};

/**
 * @brief Represents an input or output connection point on a @ref Component.
 */
class Port
{
public:
    /**
     * @brief Constructs a port with the specified properties.
     * @param id Unique port identifier.
     * @param width Number of bits.
     * @param direction Input or output direction.
     * @param label Optional descriptive label.
     */
    Port(int id, int width, PortDirection direction, const std::string & label = "");

    /** @brief Ports cannot be copied */
    Port(const Port & other) = delete;
    /** @brief Ports cannot be assigned. */
    Port & operator=(const Port & other) = delete;

    /** @brief Ports cannot be moved. */
    Port(Port && other) = delete;
    /** @brief Ports cannot be move-assigned */
    Port & operator=(Port && other) = delete;

    /**
     * @brief Gets the port's identifier.
     * @return Copy of Port identifier.
     */
    int id() const { return id_; }

    /** @brief Creation identity within the owning component, stable until deletion. */
    int generation() const { return generation_; }

    /**
     * @brief Gets the port's bit width.
     * @return Number of bits it can output or take in as input.
     */
    int width() const { return width_; }

    /**
     * @brief Gets the port's direction.
     * @return INPUT or OUTPUT.
     */
    PortDirection direction() const { return direction_; }

    /**
     * @brief Gets the component that owns this port.
     * @return Pointer to the owning component.
     */
    Component * owner() const { return owner_; }

    /**
     * @brief Gets the wire node connected to this port.
     * @return Pointer to the connected wire node, or nullptr if disconnected.
     */
    WireNode * node() const { return node_; }

    /**
     * @brief Gets the net (logical signal) associated with this port.
     * @return Pointer to the net, or nullptr if no net is assigned.
     */
    Net * net() const { return net_; }

    /**
     * @brief Accesses the port's label.
     * @return Reference to the label.
     */
    std::string & label() { return label_; }

    /**
     * @brief Gets the port's label.
     * @return Copy of the label.
     */
    std::string label() const { return label_; }

    /**
     * @brief Checks whether the port is connected to a wire node.
     * @return True if connected, false otherwise.
     * @todo Change to be that the wire it's connected to connects to another port. Create a bool attached() const { return node_ != nullptr; } for WireNode connection and connected should instead be connection between ports.
     */
    bool connected() const { return node_ != nullptr; }

    /**
     * @brief Converts the port to its string representation.
     * @return String representation of the port.
     */
    std::string to_string() const;

private:
    friend class Circuit;
    friend class Component;
    friend class ProjectEdit;
    
    int id_; ///< Unique port identifier.
    int generation_; ///< Distinguishes replacements with the same semantic ID.
    int width_; ///< Number of bits this port outputs or inputs.
    PortDirection direction_; ///< Whether the port is an input or output.
    Component * owner_; ///< Component that this port belongs to
    WireNode * node_; ///< Wire node connected to this port, or nullptr if disconnected.
    Net * net_; ///< Net associated with this port, or nullptr if none.
    std::string label_; ///< Optional label for this port.
};

/**
 * @brief Writes a port to an output stream.
 * @param cout Output stream.
 * @param port Port to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const Port & port)
{
    cout << port.to_string();
    return cout;
}

#endif // PORT_H
