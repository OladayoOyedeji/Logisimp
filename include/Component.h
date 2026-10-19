/**
 * @file Component.h
 * @brief Abstract class for all circuit components.
 * @author Cole Schwandt
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <ostream>
#include <string>
#include <vector>

class Circuit;
class Port;
class Simulator;
class CircuitState;
class ComponentState;
class ProjectEdit;

/**
 * @brief Abstract base class for circuit components with input and output @ref Port "ports".
 */
class Component
{
public:
    /**
     * @brief Constructs a component with an identifier and optional label.
     * @param id Component identifier.
     * @param label Optional descriptive label.
     */
    Component(int id, const std::string & label = "")
        : id_(id), label_(label), next_port_id_(0), edit_(nullptr)
    {}

    /** @brief Components cannot be copied. */
    Component(const Component & other) = delete;

    /** @brief Components cannot be assigned. */
    Component & operator=(const Component & other) = delete;

    /** @brief Components cannot be moved. */
    Component(Component && other) = delete;

    /** @brief Components cannot be move-assigned. */
    Component & operator=(Component && other) = delete;

    /**
     * @brief Destroys the component and its ports.
     */
    virtual ~Component()
    {
        clear_ports();
    }

    /**
     * @brief Gets the component's identifier.
     * @return Copy of the identifier.
     */
    int id() const { return id_; }

    /**
     * @brief Accesses the component's label.
     * @return Reference to the label.
     */
    std::string & label() { return label_; }

    /**
     * @brief Gets the component's label.
     * @return Copy of the label.
     */
    std::string label() const { return label_; }

    /**
     * @brief Gets the component's input ports.
     * @return Const reference to the vector of input port pointers.
     */
    const std::vector< Port * > & inputs() const { return inputs_; }
    
    /**
     * @brief Gets the component's output ports.
     * @return Const reference to the vector of output port pointers.
     */
    const std::vector< Port * > & outputs() const { return outputs_; }

    /**
     * @brief Gets the number of input ports.
     * @return Copy of the input port count.
     */
    int input_count() const;

    /**
     * @brief Gets the number of output ports.
     * @return Copy of the output port count.
     */
    int output_count() const;
    
    /**
     * @brief Accesses an input port at the specified index.
     * @param i Index of the input port.
     * @return Pointer to the input port.
     */
    Port * input_port(int i);

    /**
     * @brief Gets an input port at the specified index.
     * @param i Index of the input port.
     * @return Pointer to the input port.
     */
    Port * input_port(int i) const;

    /**
     * @brief Accesses an output port at the specified index.
     * @param i Index of the output port.
     * @return Pointer to the output port.
     */
    Port * output_port(int i);

    /**
     * @brief Gets an output port at the specified index.
     * @param i Index of the output port.
     * @return Pointer to the output port.
     */
    Port * output_port(int i) const;

    /**
     * @brief Creates component-specific runtime state.
     * @return Pointer to the component state, or nullptr if none is needed.
     * @note The default implementation returns nullptr.
     */
    virtual ComponentState * create_state(CircuitState &) const
    {
        return nullptr;
    }


    /**
     * @brief Evaluates the component during simulation.
     * @param simulator Simulator performing the evaluation.
     * @param state Runtime state of the circuit being simulated.
     */
    virtual void evaluate(Simulator & simulator, CircuitState & state) = 0;


    /**
     * @brief Converts the component to its string representation.
     * @return String representation of the component.
     */
    virtual std::string to_string() const;

protected:
    /** 
     * @brief Adds an input port to the component.
     * @param width Number of bits accepted by the port.
     * @param label Optional descriptive label.
     * @param port_id Port identifier, or -1 for automatic assignment.
     * @return Pointer to the created input port.
     */
    Port * add_input(int width = 1, const std::string & label = "", int port_id = -1);
    
    /**
     * @brief Adds an output port to the component.
     * @param width Number of bits produced by the port.
     * @param label Optional descriptive label.
     * @param port_id Port identifier, or -1 for automatic assignment.
     * @return Pointer to the created output port.
     */
    Port * add_output(int width = 1, const std::string & label = "", int port_id = -1);

private:
    friend class Circuit;
    friend class ProjectEdit;

    /**
     * @brief Determines the ID to assign to a new port.
     * @param port_id Requested ID, or -1 for automatic assignment.
     * @return Copy of the ID to assign.
     */
    int resolve_port_id(int port_id) const;

    /**
     * @brief Removes a port from the component.
     * @param port Port to remove.
     */
    void remove_port(Port & port);

    /**
     * @brief Removes all input and output ports.
     */
    void clear_ports();
    
    int id_; ///< Component identifier.
    std::string label_; ///< Optional descriptive label.
    std::vector< Port * > inputs_; ///< Input ports belonging to this component.
    std::vector< Port * > outputs_; ///< Output ports belonging to this component.
    int next_port_id_; ///< Next identifier for automatic port assignment.

    ProjectEdit * edit_; ///< Borrowed transaction retaining removed ports.

};

/**
 * @brief Writes a component to an output stream.
 * @param cout Output stream.
 * @param component Component to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const Component & component)
{
    cout << component.to_string();
    return cout;
}

#endif // COMPONENT_H
