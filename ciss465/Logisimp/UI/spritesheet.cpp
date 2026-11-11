#include "spritesheet.h"
#include <iostream>

namespace gui
{
    Spritesheet::Spritesheet(const QString &filename)
    {
        spriteSheet.load(filename);

        if(!spriteSheet.load(filename))
        {
            std::cout << "FAILED TO LOAD SPRITE SHEET\n";
        }
        else
        {
            std::cout << "SPRITE SHEET LOADED\n";
        }
    }

    void Spritesheet::drawSprite(QPainter &painter, int row, int col,
                                 double x, double y,
                                 double width, double height)
    {
        QRect source(col * spriteWidth, row * spriteHeight, spriteWidth, spriteHeight);
        QRect destination(x, y, width, height);

        painter.drawPixmap(destination, spriteSheet, source);
    }
}
