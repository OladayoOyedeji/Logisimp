#pragma once

#include "circuit.h"
#include <vector>
#include <iostream>

class Component;

namespace gui
{
    class Port;
    
    class Gate : public Circuit
    {
    public:
        virtual ~Gate() = default;

        double getX();
        double getY();

        const std::vector<Port*>& getInputPorts() const;
        const std::vector<Port*>& getOutputPorts() const;

        void setLogicComponent(::Component* component);
        ::Component* getLogicComponent() const;

    protected:
        int numInputs = 0;
        int numOutputs = 1;

        double width = 50.0;
        double height = 50.0;

        std::vector<Port*> inputPorts;
        std::vector<Port*> outputPorts;

    private:
        ::Component* logicComponent = NULL;
    };
}
