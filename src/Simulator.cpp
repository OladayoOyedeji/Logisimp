// Simulator.cpp

#include "Simulator.h"
#include "Component.h"
#include "LogicVector.h"
#include "Port.h"
#include "Wire.h"

#include <stdexcept>

void Simulator::enqueue(Component * component)
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

void Simulator::drive(Wire & wire, const LogicVector & value)
{
    if (wire.width() != value.width())
    {
        throw std::runtime_error("Cannot drive a wire with a value of a different width");
    }

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

bool Simulator::run()
{
    const int MAX_EVALS = 100000;
    int evaluation_count = 0;

    while (!work_queue_.empty())
    {
        if (evaluation_count >= MAX_EVALS)
        {
            clear();
            return false;
        }

        Component * component = work_queue_.front();

        work_queue_.pop();
        queued_components_.erase(component);

        evaluation_count++;

        component->evaluate(*this);
    }

    return true;
}

void Simulator::clear()
{
    while (!work_queue_.empty())
    {
        work_queue_.pop();
    }

    queued_components_.clear();
}

std::string Simulator::to_string() const
{
    std::queue< Component * > queue = work_queue_;
    std::string ret;

    ret += "Simulator(queued=";
    ret += std::to_string(queue.size());
    ret += ')';

    if (!queue.empty())
    {
        ret += "\nQueue:";

        while (!queue.empty())
        {
            ret += "\n  ";

            if (queue.front() == nullptr)
            {
                ret += "nullptr";
            }
            else
            {
                ret += queue.front()->to_string();
            }

            queue.pop();
        }
    }

    return ret;
}
