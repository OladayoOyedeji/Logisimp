// Simulator.h

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Component.h"
#include "LogicVector.h"
#include "Wire.h"

#include <queue>
#include <stdexcept>
#include <unordered_set>
#include <iostream>

class Simulator
{
public:
    Simulator()
    {}

    std::queue< Component * > & work_queue() { return work_queue_; }
    std::queue< Component * > work_queue() const { return work_queue_; }

    std::unordered_set< Component * > & queued_components() { return queued_components_; }
    std::unordered_set< Component * > queued_components() const { return queued_components_; }

    void enqueue(Component * component)
    {
        if (component == nullptr)
        {
            return;
        }

        if (queued_components_.find(component) != queued_components_.end())
        {
            return;
        }

        queued_components_.insert(component);
        work_queue_.push(component);
    }

    void drive(Wire & wire, const LogicVector & value)
    {
        if (wire.width() != value.width())
        {
            throw std::runtime_error("Cannot drive a wire with a value of a different width");
        }

        // only enqueue if value changed
        if (wire.value() == value)
        {
            return;
        }
 
        wire.value() = value;

        for (Port * listener : wire.listeners())
        {
            enqueue(listener->owner());
        }
    }

    bool run()
    {
        const int MAX_EVALS = 100000;
        int evaluation_count = 0;

        while (!work_queue_.empty())
        {
            // returns false if circuit oscillate
            // (need a better way to indicate this.
            // Currently the issue is evaluation count is global,
            // but it should be localized to each component)
            if (evaluation_count >= MAX_EVALS)
            {
                clear();
                return false;
                // the issue with this is it aborts the whole circuit
                // if one part is oscillating, but you can have
                // two disconnected "circuits" in one circuit project.
                // if one fails, we don't want that to make the other fail.
                // Need a better way to do this. Probably by having a
                // hashtable that maps Component * to evaluation_counts,
                // then marking the output of components that exceed MAX_EVALS
                // as UNKNOWN
            }

            Component * component = work_queue_.front();

            work_queue_.pop();
            queued_components_.erase(component);

            evaluation_count++;

            component->evaluate(*this);
        }

        return true;
    }

    void clear()
    {
        while (!work_queue_.empty())
        {
            work_queue_.pop();
        }

        queued_components_.clear();
    }

private:
    std::queue< Component * > work_queue_;
    std::unordered_set< Component * > queued_components_;
};

inline std::ostream & operator<<(std::ostream & cout, const Simulator & simulator)
{
    std::queue< Component * > queue = simulator.work_queue();

    cout << "Simulator("
         << "queued=" << queue.size()
         << ')';

    if (!queue.empty())
    {
        cout << "\nQueue:";

        while (!queue.empty())
        {
            cout << "\n  ";

            if (queue.front() == nullptr)
            {
                cout << "nullptr";
            }
            else
            {
                cout << *queue.front();
            }

            queue.pop();
        }
    }

    return cout;
}

#endif // Simulator.h
