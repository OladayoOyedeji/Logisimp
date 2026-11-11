#pragma once

// This is the Logic port class
// I should change the name of this header
// to avoid confusion
#include "Port.h"
#include "Handles.h"

namespace gui
{
    class Gate;

    enum class PortType
    {
        INPUT,
        OUTPUT
    };

    class Port
    {
    public:
        Port(Gate*, double, double, PortType);

        double getX() const;
        double getY() const;

        PortType getType();

        bool contains(double, double) const;

        void setPortId(PortId);
        PortId getPortId() const;

        Gate* getParentGate() const;
    private:
        Gate *parentGate;

        double offsetX;
        double offsetY;

        PortType type;

        PortId portId;
    };
}
