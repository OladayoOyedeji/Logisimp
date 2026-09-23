#ifndef CANVAS_H
#define CANVAS_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <algorithm>

#include "gates.h"

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

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

    // Control Events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    double zoom = 1.0;
    double cameraX = 0.0;
    double cameraY = 0.0;

    const double gridsize = 25;

    bool panning = false;
    bool draggingGate = false;
    QPoint lastMousePosition;
    QPoint dragOffset;

    AndGate andGate;
};

#endif // CANVAS_H
