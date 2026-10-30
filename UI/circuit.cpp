#include "circuit.h"

namespace gui
{
    double Circuit::getX() const
    {
        return x;
    }

    double Circuit::getY() const
    {
        return y;
    }

    void Circuit::setPosition(double newX, double newY)
    {
        x = newX;
        y = newY;
    }
}
