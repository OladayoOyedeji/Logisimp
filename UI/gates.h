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

            sprites.drawSprite(painter, 0, 2,
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

            sprites.drawSprite(painter, 1, 2,
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

    class InputPin : public Gate
    {
    public:
        InputPin()
        {
            numInputs = 0;
            numOutputs = 1;

            width = 25;
            height = 25;

            outputPorts.push_back(new Port(this, width / 2, 0, PortType::OUTPUT));
        }

        void draw(QPainter &painter, Spritesheet &sprites, double screenX, double screenY, double zoom) override
        {
            // The sprites parameter can remain unused for now.

            double drawWidth = width * zoom;
            double drawHeight = height * zoom;
            
            painter.setPen(QPen(Qt::black, 2));
            painter.setBrush(QColor(80, 80, 80));
            
            double screenWidth = width * zoom;
            double screenHeight = height * zoom;


            painter.drawRect(screenX - screenWidth / 2.0,
                             screenY - screenHeight / 2.0,
                             screenWidth, screenHeight);

            painter.setPen(Qt::white);
            
            painter.drawText(
                QRectF(
                    screenX - screenWidth / 2.0,
                    screenY - screenHeight / 2.0,
                    screenWidth,
                    screenHeight
                    ),
                Qt::AlignCenter,
                "IN"
                );
        }
        
        bool contains(double worldX, double worldY) const override
        {
            return worldX >= x - width / 2.0 &&
                worldX <= x + width / 2.0 &&
                worldY >= y - height / 2.0 &&
                worldY <= y + height / 2.0;
        }
    };

    class OutputPin : public Gate
    {
    public:
        OutputPin()
        {
            numInputs = 1;
            numOutputs = 0;

            width = 25;
            height = 25;

            inputPorts.push_back(new Port(this, -width / 2, 0, PortType::INPUT));
        }

        void draw(QPainter &painter, Spritesheet &sprites, double screenX, double screenY, double zoom) override
        {
            // The sprites parameter can remain unused for now.

            double drawWidth = width * zoom;
            double drawHeight = height * zoom;
            
            painter.setPen(QPen(Qt::black, 2));
            painter.setBrush(displayColor);
            
            double screenWidth = width * zoom;
            double screenHeight = height * zoom;

            painter.drawRect(screenX - screenWidth / 2.0,
                             screenY - screenHeight / 2.0,
                             screenWidth, screenHeight);

            painter.setPen(Qt::white);
            
            painter.drawText(
                QRectF(
                    screenX - screenWidth / 2.0,
                    screenY - screenHeight / 2.0,
                    screenWidth,
                    screenHeight
                    ),
                Qt::AlignCenter,
                "OUT"
                );
        }
        
        bool contains(double worldX, double worldY) const override
        {
            return worldX >= x - width / 2.0 &&
                worldX <= x + width / 2.0 &&
                worldY >= y - height / 2.0 &&
                worldY <= y + height / 2.0;
        }

        void setDisplayColor(const QColor &color)
        {
            displayColor = color;
        }
    private:
        QColor displayColor = QColor(128, 128, 128);
    };
}
