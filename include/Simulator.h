// Simulator.h

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Component.h"
#include "LogicVector.h"
#include "Signal.h"

#include <queue>
#include <stdexcept>

enum SimulationResult
{
    SETTLED,
    EVALUATION_LIMIT_REACHED
};

class Simulator
{
public:
    Simulator(int max_evaluations = 100000)
        : max_evaluations_(max_evaluations)
    {
        if (max_evaluations <= 0)
        {
            throw std::invalid_argument("Maximum evaluations must be positive");
        }
    }

    std::queue<Component *> & work_queue() { return work_queue_; }
    std::queue<Component *> work_queue() const { return work_queue_; }

    int & max_evaluations() { return max_evaluations_; }
    int max_evaluations() const { return max_evaluations_; }

    void enqueue(Component * component)
    {
        if (component == nullptr)
        {
            return;
        }

        if (component->queued())
        {
            return;
        }

        component->queued() = true;
        work_queue_.push(component);
    }

    void drive(Signal & signal, const LogicVector & value)
    {
        if (signal.width() != value.width())
        {
            throw std::runtime_error("Cannot drive signal with value of different width");
        }

        if (signal.value() == value)
        {
            return;
        }

        signal.value() = value;

        for (Port * listener : signal.listeners())
        {
            enqueue(listener->owner());
        }
    }

    SimulationResult run()
    {
        int evaluation_count = 0;

        while (!work_queue_.empty())
        {
            evaluation_count++;

            if (evaluation_count > max_evaluations_)
            {
                clear_queue();
                return EVALUATION_LIMIT_REACHED;
            }

            Component * component = work_queue_.front();

            work_queue_.pop();

            component->queued() = false;
            component->evaluate(*this);
        }

        return SETTLED;
    }

    void clear_queue()
    {
        while (!work_queue_.empty())
        {
            Component * component = work_queue_.front();

            work_queue_.pop();

            component->queued() = false;
        }
    }

private:
    std::queue<Component *> work_queue_;
    int max_evaluations_;
};

#endif // Simulator.h
