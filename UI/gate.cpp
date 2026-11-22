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

    ComponentId gui::Gate::getComponentId() const
    {
        return componentId;
    }

    void gui::Gate::setComponentId(ComponentId component)
    {
        componentId = component;
    }
}
 
