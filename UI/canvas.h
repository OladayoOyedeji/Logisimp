/**
 * @file canvas.h
 * @author Ethan Graham
 * @date 10/16/2026
 * @brief This declares the interactive circuit workspace
 */

/**
 * @class Canvas
 * @brief This class provides an interactive graphical workspace
 *        for circuit components
 *
 * @details
 * Canvas is responsible for displaying and managing the
 * GUI for the workspace of the logic simulator
 *
 * The program uses QOpenGLWidget OpenGl rendering
 * and QPainter for drawing the grid and circuit elements
 *
 * Current Features include:
 * - Rendering a grid-based workspace
 * - Displaying AND, OR, and NOT gates
 * - Dragging a gate using the leftmouse button
 * - Panning the grid using either the middle mouse button
 *   or using the WASD keys
 * - Zooming in and out toward the mouse cursor
 *
 * @note Circuit simulation logic is not yet integrated into
 *       the graphical user interface
 *
 * @see Circuit
 * @see Gate
 * @see Gates
 */

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QCursor>

#include <cmath>
#include <algorithm>
#include <vector>

#include "port.h"
#include "spritesheet.h"
#include "circuitcontroller.h"
#include "Component.h"
#include "gate.h"
#include "gates.h"

class Component;

namespace gui
{
    class Gate;
    class Wire;
    class Port;

    class Canvas : public QOpenGLWidget,
                   protected QOpenGLFunctions
    {
        Q_OBJECT

    public:
        /**
         * @brief Canvas constructor
         *
         *Initializes the canvas
         *and for now initializes an AND, OR, and NOT gate
         *
         * @param parent Parent QWidget
         */
        Canvas(QWidget * parent = NULL);


        /**
         * @brief Draws the workspace grid
         *
         * @details
         * Draws vertical and horizontal grid line,
         * based on the current camera position,
         * zoom, and grid spacing
         * 
         * @param painter Painter is used for rendering
         */
        void drawGrid(QPainter&);

        /**
         * @brief Converts screenX coordinates to world space
         *
         * @param screenX Horizontal screen coordinate
         * @return Horizontal world coordinate
         */
        double screenToWorldX(double) const;
    
        /**
         * @brief Converts screenY coordinates to world space
         *
         * @param screenY Vertical screen coordinate
         * @return Vertical world coordinate
         */
        double screenToWorldY(double) const;

        /**
         * @brief Converts worldX coordinates to screen space
         *
         * @param worldX Horizontal world coordinate
         * @return Horizontal screen coordinate
         */
        double worldToScreenX(double) const;

        /**
         * @brief Converts worldY coordinates to screen space
         *
         * @param worldY vertical world coordinate
         * @return Vertical screen coordinate
         */
        double worldToScreenY(double) const;

        /**
         * @brief Rounds a coordinate down to a grid boundary for circuit objects
         *
         * @param  value Coordinate to align
         * @return Coordinate aligned to the grid
         */
        double snapToGrid(double);

        /**
         * @brief Updates camera's position
         *
         * @details The camera's panning speed
         * determines the displacement of its
         * x/y coordinates, while the zoom magnifies
         * its position relative to the screen
         */
        // ---- MOVE TO CAMERA CLASS LATER ----
        void updateCamera();
        // ---- MOVE TO CAMERA CLASS LATER ----

        /**
         * @brief Creats a new gate object relative to the mouse's coordinates
         *
         * @param newGate New gate to place in the workspace
         * @param logicGate New logical gate component set to visual gate
         */
        void placeGate(Gate*, ::Component*);

        /**
         * @brief Finds the type of port selected at the specified world coordinate
         *
         * @param worldX Horizontal world coordinate
         * @param worldY Vertical world coordinate
         * @return Port type found at world coordinate, NULL if no port found
         */
        Port *findport(double, double);
 
    protected:
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;
        void drawPort(QPainter&, Port*);
        void drawWire(QPainter&, Wire*);
        void drawTempWire(QPainter&);

        // Control Events
        /**
         * @brief Processes mouse buttons that are pressed
         *
         * @details
         *
         * The behavior depends on the event that happens
         *
         * Convert the event position to mouse Position coordinates
         *
         * When left button clicked:
         * - Check if mouse clicked a port or a gate
         * - If mouse clicked a port:
         *     - Draw wire from start to end
         *     - drawing wire state is true
         * - If mouse clicked a gate:
         *     - Gate is selected
         *     - Dragging State is true
         *     - Set dragging offset coordinates
         *
         * When middle button is clicked
         * - Set panning state to true
         *
         * @param event QT mouse event
         *
         * @see mouseMoveEvent()
         * @see mouseReleaseEvent()
         * @see screenToWorldX()
         * @see screenToWorldY()
         */
        void mousePressEvent(QMouseEvent *event) override;

        /**
         * @brief Processes mouse movement within the workspace
         *
         * @details
         * The behavor depends on the current interaction state
         *
         * When drawing wire state is true
         * - requests a redraw to the screen
         *
         * When dragging state is true:
         * - Converts mouse coordinates to world coordinates
         * - Applies the drag offset
         * - Snaps the gate position ot the nearest grid point
         * - Requests a redraw of the screen
         * 
         * When panning state is true:
         * - Calculate mouse displacement
         * - Adjust camera position relative to zoom
         * - Requests a redraw to the screen
         *
         * @param event Qt mouse movement event
         *
         * @see mousePressEvent()
         * @see mouseReleaseEvent()
         * @see screenToWorldX()
         * @see screenToWorldY()
         */
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

        /**
         * @brief Changes the workspace zoom using the mouse wheel
         *
         * @details
         * Zooming is centered at the current mouse position
         * 
         * The function first records the world coordinates
         * beneath the cursor. It then modifies the zoom factor
         * and recalculates the camera position so the same
         * world point remains beneath the cursor.
         *
         * The zoom factor is limited to [0.5, 10.0].
         *
         * @param event QT mouse wheel event containing the scroll
         *              direction and cursor position
         *
         * @note Each wheel event scales zoom by a factor of 1.1.
         */
        void wheelEvent(QWheelEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;

    private:
        Spritesheet spriteSheet;

        // ---- MOVE TO CAMERA CLASS LATER ----
        double zoom = 1.0;
        double cameraX = 0.0;
        double cameraY = 0.0;

        QTimer cameraTimer;
        // ---- MOVE TO CAMERA CLASS LATER ----

        const double gridsize = 25;

        bool panning = false;
        bool draggingGate = false;
        Gate *selectedGate = NULL;
        QPoint lastMousePosition;
        QPoint dragOffset;

        bool wPressed = false;
        bool aPressed = false;
        bool sPressed = false;
        bool dPressed = false;

        Port *wireStart = NULL;
        bool drawingWire = false;

        std::vector<Gate*> gates;
        std::vector<Wire*> wires;

        QPointF mousePosition;

        CircuitController controller;
    };
}
