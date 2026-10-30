/**
 * @file CircuitState.cpp
 * @brief Implements the runtime state of a circuit instance.
 * @author Cole Schwandt
 */

#include "CircuitState.h"

#include "Circuit.h"
#include "Component.h"
#include "ComponentState.h"
#include "Simulator.h"
#include "Subcircuit.h"
#include "SubcircuitState.h"
#include "Net.h"
#include "Pin.h"
#include "Port.h"
#include "WireNode.h"

#include <algorithm>
#include <stdexcept>

void print_diagnostics(const CircuitState & state, std::ostream & out)
{
    const auto diagnostics = state.diagnostics();
    if (diagnostics.empty())
    {
        out << "Diagnostics: none\n";
        return;
    }

    for (const CircuitDiagnostic & diagnostic : diagnostics)
    {
        out << "Diagnostic: ";
        switch (diagnostic.kind)
        {
        case DiagnosticKind::WidthMismatch:
            out << "WidthMismatch";
            break;
        case DiagnosticKind::DriverConflict:
            out << "DriverConflict";
            break;
        case DiagnosticKind::SignalError:
            out << "SignalError";
            break;
        }

        out << ", state=" << (diagnostic.state == &state ? "current instance" : "nested instance")
            << ", net=" << diagnostic.net->id() << ", bits=";
        if (diagnostic.bits.empty())
        {
            out << "whole net";
        }
        else
        {
            for (std::size_t i = 0; i < diagnostic.bits.size(); ++i)
            {
                out << (i == 0 ? "" : ",") << diagnostic.bits[i];
            }
        }
        out << "\n  " << diagnostic.message << '\n';
    }
}

CircuitState::CircuitState(const Circuit & circuit, CircuitState * parent, const Subcircuit * parent_subcircuit)
    : circuit_(&circuit), parent_(parent), parent_subcircuit_(parent_subcircuit)
{
    if ((parent_ == nullptr) != (parent_subcircuit_ == nullptr))
    {
        // Output notifications need both links to find the containing placement;
        // root instances have neither link.
        throw std::invalid_argument("Nested state requires both a parent and a placement");
    }

    if (parent_ != nullptr)
    {
        const std::vector< Component * > & components = parent_->circuit().components();
        if (parent_subcircuit_->circuit() != circuit_ || std::find(components.begin(), components.end(), parent_subcircuit_) == components.end())
        {
            throw std::invalid_argument("Placement does not belong to the supplied parent or blueprint");
        }
    }

    // Derived nets are borrowed by the value map. Register before constructing
    // child states so connectivity edits cannot invalidate any live hierarchy.
    ++circuit_->active_states_;
    try
    {
        reset();
    }
    catch (...)
    {
        --circuit_->active_states_;
        throw;
    }
}

CircuitState::~CircuitState()
{
    clear_component_states();
    --circuit_->active_states_;
}

LogicVector & CircuitState::value(const Net & net)
{
    return net_values_.at(&net);
}

LogicVector CircuitState::value(const Net & net) const
{
    return net_values_.at(&net);
}

LogicVector CircuitState::value(const Port & port) const
{
    const Component * owner = port.owner();
    if (owner == nullptr || circuit_->find_component(owner->id()) != owner || owner->find_port(port.id()) != &port)
    {
        throw std::invalid_argument("Port does not belong to this runtime instance's blueprint");
    }

    const Net * net = port.net();
    if (net == nullptr)
    {
        if (port.direction() == OUTPUT)
        {
            return output_values_.at(&port);
        }
        return LogicVector(port.width(), UNKNOWN);
    }
    if (net->width_mismatch() || net->width() != port.width())
    {
        return LogicVector(port.width(), UNKNOWN);
    }

    const LogicVector & bits = net_values_.at(net);
    if (bits.width() != port.width())
    {
        return LogicVector(port.width(), UNKNOWN);
    }
    return bits;
}

std::vector< CircuitDiagnostic > CircuitState::diagnostics(bool recursive) const
{
    std::vector< CircuitDiagnostic > result;
    for (const Net * net : circuit_->nets())
    {
        if (net->width_mismatch())
        {
            std::vector< int > widths;
            auto add_width = [&](int width)
            {
                if (width > 0 && std::find(widths.begin(), widths.end(), width) == widths.end())
                {
                    widths.push_back(width);
                }
            };
            for (const WireNode * node : net->nodes())
            {
                add_width(node->width());
            }
            for (const Port * port : net->ports())
            {
                add_width(port->width());
            }
            std::sort(widths.begin(), widths.end());
            std::string message = "Connected wires and ports have incompatible bit widths: ";
            for (std::size_t i = 0; i < widths.size(); ++i)
            {
                message += (i == 0 ? "" : ", ") + std::to_string(widths[i]);
            }
            result.push_back({DiagnosticKind::WidthMismatch, this, net, {},
                              message});
            // There is no common bus shape on which to classify individual bits.
            continue;
        }

        std::vector< int > conflict_bits;
        std::vector< int > error_bits;
        const LogicVector & resolved = net_values_.at(net);
        for (int bit = 0; bit < net->width(); ++bit)
        {
            bool low = false;
            bool high = false;
            for (const Port * driver : net->drivers())
            {
                auto intent = output_values_.find(driver);
                if (intent == output_values_.end() || intent->second.width() != net->width())
                {
                    continue;
                }
                low = low || intent->second[bit] == LOW;
                high = high || intent->second[bit] == HIGH;
            }
            if (low && high)
            {
                conflict_bits.push_back(bit);
            }
            else if (bit < resolved.width() && resolved[bit] == ERROR)
            {
                error_bits.push_back(bit);
            }
        }
        if (!conflict_bits.empty())
        {
            result.push_back({DiagnosticKind::DriverConflict, this, net, conflict_bits,
                              "Opposing LOW and HIGH output drivers on net " + std::to_string(net->id())});
        }
        if (!error_bits.empty())
        {
            result.push_back({DiagnosticKind::SignalError, this, net, error_bits,
                              "ERROR signal on net " + std::to_string(net->id())});
        }
    }

    if (recursive)
    {
        for (const Component * component : circuit_->components())
        {
            const Subcircuit * placement = dynamic_cast< const Subcircuit * >(component);
            if (placement == nullptr)
            {
                continue;
            }
            const SubcircuitState & child = dynamic_cast< const SubcircuitState & >(component_state(*placement));
            std::vector< CircuitDiagnostic > nested = child.child_state().diagnostics(true);
            result.insert(result.end(), nested.begin(), nested.end());
        }
    }
    return result;
}

ComponentState & CircuitState::component_state(const Component & component)
{
    return *component_states_.at(&component);
}

const ComponentState & CircuitState::component_state(const Component & component) const
{
    return *component_states_.at(&component);
}

void CircuitState::clear_component_states()
{
    // Virtual destruction also releases child CircuitStates held by placements.
    // Pending simulator events borrow these objects and must already be cleared.
    for (const std::pair< const Component * const, ComponentState * > & entry : component_states_)
    {
        delete entry.second;
    }

    component_states_.clear();
}

void CircuitState::reset()
{
    clear_component_states();
    net_values_.clear();
    output_values_.clear();
    pin_shapes_.clear();

    try
    {
        // create runtime states for components that need a state
        // Stateless gates share only their blueprint; pins and subcircuits need
        // fresh storage so repeated placements have independent runtime values.
        for (Component * component : circuit_->components())
        {
            // Output values belong to this instance even while no wire is
            // attached. Live connectivity edits can seed replacement nets from
            // the last driven value without replacing the component's state.
            for (Port * output_port : component->outputs())
            {
                output_values_.emplace(output_port, LogicVector(output_port->width(), UNKNOWN));
            }

            const Pin * pin = dynamic_cast< const Pin * >(component);
            if (pin != nullptr)
            {
                pin_shapes_.emplace(component, std::make_pair(int(pin->type()), pin->width()));
            }

            ComponentState * component_state = component->create_state(*this);

            if (component_state != nullptr)
            {
                try
                {
                    component_states_.emplace(component, component_state);
                }
                catch (...) // std::bad_alloc prob the only possible exception that could happen here
                {
                    // The factory returned an owned allocation, but the map has
                    // not adopted it if inserting the entry fails.
                    delete component_state;
                    throw;
                }
            }
        }

        // Resolve only after every output has its own initialized intent. No
        // driver has evaluated yet, so valid undriven values remain UNKNOWN.
        for (const Net * net : circuit_->nets())
        {
            net_values_.emplace(net, net->resolve(output_values_));
        }
    }
    catch (...)
    {
        // Reset discarded the previous storage; release any partially rebuilt
        // hierarchy rather than expose a mixture of initialized and absent state.
        clear_component_states();
        net_values_.clear();
        output_values_.clear();
        pin_shapes_.clear();
        throw;
    }
}

bool CircuitState::reset(Simulator & simulator)
{
    // Queued events can point at children that reset() is about to destroy.
    simulator.clear();
    reset();
    return initialize(simulator);
}

bool CircuitState::initialize(Simulator & simulator)
{
    // Initialization replaces all pending work in the shared simulator, even
    // for other roots; the current signal values themselves are preserved.
    simulator.clear();
    try
    {
        simulator.schedule(*this);
        return simulator.run();
    }
    catch (...)
    {
        simulator.clear();
        throw;
    }
}

std::string CircuitState::to_string() const
{
    std::string ret = "CircuitState(";
    const std::vector< Net * > & nets = circuit_->nets();
    for (int i = 0; i < int(nets.size()); ++i)
    {
        if (i > 0)
        {
            ret += ", ";
        }

        const Net * net = nets[i];
        ret += "net ";
        ret += std::to_string(net->id());
        ret += "=";
        ret += value(*net).to_string();
    }
    ret += ")";

    return ret;
}
