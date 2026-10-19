/**
 * @file Circuit.h
 * @author Ethan Graham
 * @date 10/19/2026
 * @brief Base class for circuit elements
 */

/**
 * @class Circuit
 * @brief Abstract base class for graphical circuit elements
 *
 * @details 
 * Defines the shared positions and rendering interface
 * for objects displayed in the circuit workspace
 *
 * Derived classes must implement draw() to provide
 * their own visual representation
 * 
 * 
 * @note Positions are stored within world coordinates
 *
 * @see Gate
 */

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "spritesheet.h"

#include <QPainter>

class Circuit
{
public:
    virtual ~Circuit() = default;

    /**
     * @brief Renders the circuit element
     * 
     * @param painter QPainter used for drawing
     * @param sprites Spritesheet stored in this variable
     * @param screenX Horizontal screen positon
     * @param screenY Vertical screen position
     * @param zoom Current canvas zoom factor
     */
    virtual void draw(QPainter &painter, Spritesheet &sprites, double screenX, double screenY, double zoom) = 0;
    virtual bool contains(double worldX, double worldY) const = 0;

    double getX() const;
    double getY() const;

    /**
     * @brief Sets the element's world position
     *
     * @param newX New horizontal world coordinate
     * @param newY New vertical world coordinate
     */
    void setPosition(double newX, double newY);

protected:
    double x = 0;
    double y = 0;
};

#endif // CIRCUIT_H
