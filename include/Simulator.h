// Simulator.h
/**
 * Evaluates components whose input values have changed.
 * Manages a scheduler/work queue for evaluation of components
 * TODO: rename to Scheduler(?) and do time based execution
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <ostream>
#include <queue>
#include <string>
#include <unordered_set>

class Component;
class LogicVector;
class Wire;

class Simulator
{
public:
    Simulator()
    {}

    std::queue< Component * > & work_queue() { return work_queue_; }
    std::queue< Component * > work_queue() const { return work_queue_; }

    std::unordered_set< Component * > & queued_components() { return queued_components_; }
    std::unordered_set< Component * > queued_components() const { return queued_components_; }

    void enqueue(Component * component);
    void drive(Wire & wire, const LogicVector & value);
    bool run();
    void clear();

    std::string to_string() const;

private:
    std::queue< Component * > work_queue_;
    std::unordered_set< Component * > queued_components_;
};

inline
std::ostream & operator<<(std::ostream & cout, const Simulator & simulator)
{
    cout << simulator.to_string();
    return cout;
}

#endif // Simulator.h
