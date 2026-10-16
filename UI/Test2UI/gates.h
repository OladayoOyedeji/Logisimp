#ifndef GATES_H
#define GATES_H

#include "gate.h"

class AndGate : public Gate
{
public:
    AndGate()
    {
        numInputs = 2;
    }

    void draw(QPainter &painter, double screenX, double screenY, double zoom) override
    {
        QPen gridPen(QColor(0, 0, 250));

        painter.setPen(gridPen);

        double screenWidth = width * zoom;
        double screenHeight = height * zoom;

        painter.drawRect(screenX - screenWidth / 2.0,
                         screenY - screenHeight / 2.0,
                         screenWidth, screenHeight);
    }

    bool contains(double worldX, double worldY) const
    {
        return worldX >= x - width / 2.0 &&
               worldX <= x + width / 2.0 &&
               worldY >= y - height / 2.0 &&
               worldY <= y + height / 2.0;
    }

    void get_gate()
    {
        std::cout << "And Gate" << '\n';
    }
};

class OrGate : public Gate
{
public:
    OrGate()
    {
        numInputs = 2;
    }

    void draw(QPainter &painter, double screenX, double screenY, double zoom) override
    {
        QPen gridPen(QColor(0, 250, 0));

        painter.setPen(gridPen);

        double screenWidth = width * zoom;
        double screenHeight = height * zoom;

        painter.drawRect(screenX - screenWidth / 2.0,
                         screenY - screenHeight / 2.0,
                         screenWidth, screenHeight);
    }

    bool contains(double worldX, double worldY) const
    {
        return worldX >= x - width / 2.0 &&
               worldX <= x + width / 2.0 &&
               worldY >= y - height / 2.0 &&
               worldY <= y + height / 2.0;
    };

    void get_gate()
    {
        std::cout << "Or Gate" << '\n';
    }
};

class NotGate : public Gate
{
public:
    NotGate()
    {
        numInputs = 1;
    }

    void draw(QPainter &painter, double screenX, double screenY, double zoom) override
    {
        QPen gridPen(QColor(250, 0, 0));

        painter.setPen(gridPen);

        double screenWidth = width * zoom;
        double screenHeight = height * zoom;

        painter.drawRect(screenX - screenWidth / 2.0,
                         screenY - screenHeight / 2.0,
                         screenWidth, screenHeight);
    }

    bool contains(double worldX, double worldY) const
    {
        return worldX >= x - width / 2.0 &&
               worldX <= x + width / 2.0 &&
               worldY >= y - height / 2.0 &&
               worldY <= y + height / 2.0;
    }

    void get_gate()
    {
        std::cout << "Not Gate" << '\n';
    }
};

#endif // GATES_H
