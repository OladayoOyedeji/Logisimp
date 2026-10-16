/**
 * @file circuit.cpp
 * @brief Position accessors for the Qt prototype drawables.
 * @author Cole Schwandt
 */

#include "circuit.h"


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
