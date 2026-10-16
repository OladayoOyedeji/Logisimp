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

class Wire;
class Port;

class Canvas : public QOpenGLWidget,
               protected QOpenGLFunctions
{
    Q_OBJECT

public:
    Canvas(QWidget * parent = NULL);


    void drawGrid(QPainter&);

    double screenToWorldX(double) const;
    double screenToWorldY(double) const;

    double worldToScreenX(double) const;
    double worldToScreenY(double) const;

    double snapToGrid(double);

    // ---- MOVE TO CAMERA CLASS LATER ----
    void updateCamera();
    // ---- MOVE TO CAMERA CLASS LATER ----

    void placeGate(Gate*);

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
