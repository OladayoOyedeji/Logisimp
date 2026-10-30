#pragma once

#include <QPixmap>
#include <QPainter>

namespace gui
{
    class Spritesheet
    {
    public:
        Spritesheet(const QString &);

        void drawSprite(QPainter&, int, int, double, double, double, double);
    private:
        QPixmap spriteSheet;

        int spriteWidth = 64;
        int spriteHeight = 64;
    };
}
