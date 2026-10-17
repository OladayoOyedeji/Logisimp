#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <QPixmap>
#include <QPainter>

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

#endif // SPRITESHEET_H
