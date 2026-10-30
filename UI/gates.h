#pragma once

#include "gate.h"
#include "port.h"
#include "spritesheet.h"

namespace gui
{
    class AndGate : public Gate
    {
    public:
        AndGate()
        {
            numInputs = 2;

            inputPorts.push_back(new Port(this, -width / 2, -12, PortType::INPUT));
            inputPorts.push_back(new Port(this, -width / 2, 12, PortType::INPUT));
            outputPorts.push_back(new Port(this, width / 2, 0, PortType::OUTPUT));
        }

        void draw(QPainter &painter, Spritesheet &sprites, double screenX, double screenY, double zoom) override
        {
            double drawWidth = width * zoom;
            double drawHeight = height * zoom;

            sprites.drawSprite(painter, 0, 0,
                               screenX - drawWidth / 2, screenY - drawHeight / 2,
                               drawWidth, drawHeight);

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
    };

    class OrGate : public Gate
    {
    public:
        OrGate()
        {
            numInputs = 2;

            inputPorts.push_back(new Port(this, -width / 2, -12, PortType::INPUT));
            inputPorts.push_back(new Port(this, -width / 2, 12, PortType::INPUT));
            outputPorts.push_back(new Port(this, width / 2, 0, PortType::OUTPUT));
        }

        void draw(QPainter &painter, Spritesheet &sprites, double screenX, double screenY, double zoom) override
        {
            double drawWidth = width * zoom;
            double drawHeight = height * zoom;

            sprites.drawSprite(painter, 0, 1,
                               screenX - drawWidth / 2, screenY - drawHeight / 2,
                               drawWidth, drawHeight);

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
    };

    class NotGate : public Gate
    {
    public:
        NotGate()
        {
            numInputs = 1;

            inputPorts.push_back(new Port(this, -width / 2, 0, PortType::INPUT));
            outputPorts.push_back(new Port(this, width / 2, 0, PortType::OUTPUT));
        }

        void draw(QPainter &painter, Spritesheet &sprites, double screenX, double screenY, double zoom) override
        {
            double drawWidth = width * zoom;
            double drawHeight = height * zoom;

            sprites.drawSprite(painter, 0, 2,
                               screenX - drawWidth / 2, screenY - drawHeight / 2,
                               drawWidth, drawHeight);

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
    };
}
