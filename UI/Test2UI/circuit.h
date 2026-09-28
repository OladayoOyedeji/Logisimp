#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <QPainter>

class Circuit
{
public:
    //Circuit();
    virtual ~Circuit() = default;

    virtual void draw(QPainter &painter, double screenX, double screenY, double zoom) = 0;
    virtual bool contains(double worldX, double worldY) const = 0;
    virtual void get_gate() = 0;

    double getX() const;
    double getY() const;

    void setPosition(double newX, double newY);

protected:
    double x = 0;
    double y = 0;
};

#endif // CIRCUIT_H
