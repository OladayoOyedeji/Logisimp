#ifndef PORT_H
#define PORT_H

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
private:
    Gate *parentGate;

    double offsetX;
    double offsetY;

    PortType type;
};

#endif // PORT_H
