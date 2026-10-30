#include "gate.h"
#include "port.h"

namespace gui
{
    double Gate::getX()
    {
        return x;
    }

    double Gate::getY()
    {
        return y;
    }

    const std::vector<Port*> &Gate::getInputPorts() const
    {
        return inputPorts;
    }

    const std::vector<Port*> &Gate::getOutputPorts() const
    {
        return outputPorts;
    }

    void gui::Gate::setLogicComponent(::Component* component)
    {
        logicComponent = component;
    }
    
    ::Component* gui::Gate::getLogicComponent() const
    {
        return logicComponent;
    }
}
 
