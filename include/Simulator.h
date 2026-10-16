/**
 * @file Simulator.h
 * @brief Evaluates components whose input values have changed.
 * @author Cole Schwandt
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <cstddef>
#include <functional>
#include <ostream>
#include <queue>
#include <string>
#include <unordered_set>

class Component;
class CircuitState;
class LogicVector;
class Net;

class Simulator
{
public:
    struct Event
    {
        Event(CircuitState & state, Component & component)
            : state(&state), component(&component)
        {}

        bool operator==(const Event & other) const
        {
            return state == other.state && component == other.component;
        }

        CircuitState * state;
        Component * component;
    };

    struct EventHash
    {
        std::size_t operator()(const Event & event) const
        {
            std::size_t state_hash = std::hash< CircuitState * >()(event.state);
            std::size_t component_hash = std::hash< Component * >()(event.component);
            return state_hash ^ (component_hash + (state_hash << 6) + (state_hash >> 2));
        }
    };

    Simulator()
    {}

    Simulator(const Simulator & other) = delete;
    Simulator & operator=(const Simulator & other) = delete;
    Simulator(Simulator && other) = delete;
    Simulator & operator=(Simulator && other) = delete;

    const std::queue< Event > & work_queue() const { return work_queue_; }
    const std::unordered_set< Event, EventHash > & queued_components() const { return queued_components_; }

    void enqueue(CircuitState & state, Component * component);
    void schedule(CircuitState & state);
    void notify_parent(CircuitState & state);
    void drive(CircuitState & state, Net & net, const LogicVector & value);
    bool run();
    void clear();

    std::string to_string() const;

private:
    std::queue< Event > work_queue_;
    std::unordered_set< Event, EventHash > queued_components_;
};

inline std::ostream & operator<<(std::ostream & cout, const Simulator & simulator)
{
    cout << simulator.to_string();
    return cout;
}

#endif // SIMULATOR_H
