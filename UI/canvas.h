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

#ifndef CANVAS_H
#define CANVAS_H

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
     *
     */
    void drawGrid(QPainter&);

    /**
     *
     */
    double screenToWorldX(double) const;
    double screenToWorldY(double) const;

    double worldToScreenX(double) const;
    double worldToScreenY(double) const;

    double snapToGrid(double);

    /**
     *
     */
    // ---- MOVE TO CAMERA CLASS LATER ----
    void updateCamera();
    // ---- MOVE TO CAMERA CLASS LATER ----

    /**
     *
     */
    void placeGate(Gate*);

    /**
     *
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
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
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
};

#endif // CANVAS_H
