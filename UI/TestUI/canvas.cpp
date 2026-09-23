#include "canvas.h"

Canvas::Canvas(QWidget *parent)
    : QOpenGLWidget(parent)
{
    andGate.setPosition(300, 500);
}

double Canvas::screenToWorldX(double screenX) const
{
    return screenX / zoom + cameraX;
}
double Canvas::screenToWorldY(double screenY) const
{
    return screenY / zoom + cameraY;
}

double Canvas::worldToScreenX(double worldX) const
{
    return (worldX - cameraX) * zoom;
}
double Canvas::worldToScreenY(double worldY) const
{
    return (worldY - cameraY) * zoom;
}

double Canvas::snapToGrid(double value)
{
    return std::floor(value/gridsize) * gridsize;
}

void Canvas::drawGrid(QPainter &painter)
{
    QPen gridPen(QColor(60, 60, 60));

    painter.setPen(gridPen);

    double startX = snapToGrid(cameraX);
    double startY = snapToGrid(cameraY);

    for(double x = startX; x < cameraX + width() / zoom; x += gridsize)
    {
        double screenX = worldToScreenX(x);

        painter.drawLine(screenX, 0, screenX, height());
    }
    for(double y = startY; y < cameraY + height() / zoom; y += gridsize)
    {
        double screenY = worldToScreenY(y);

        painter.drawLine(0, screenY, width(), screenY);
    }
}

void Canvas::initializeGL()
{
    // OpenGL Initialization
    initializeOpenGLFunctions();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void Canvas::paintGL()
{
    // Draw Background
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw Grid
    QPainter painter(this);
    drawGrid(painter);

    // Draw Wires

    // Draw Gates
    double screenX = worldToScreenX(andGate.getX());
    double screenY = worldToScreenY(andGate.getY());

    andGate.draw(painter, screenX, screenY, zoom);

    // Draw I/O pins
}

void Canvas::resizeGL(int width, int height)
{
    // Update Viewport/Projection
    glViewport(0, 0, width, height);
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        double worldX = screenToWorldX(event->position().x());
        double worldY = screenToWorldY(event->position().y());

        if(andGate.contains(worldX, worldY))
        {
            draggingGate = true;

            dragOffset.setX(worldX - andGate.getX());
            dragOffset.setY(worldY - andGate.getY());

        }
    }

    if (event->button() == Qt::MiddleButton)
    {
        panning = true;
        lastMousePosition = event->pos();
    }

    // Change the panning to WASD later
    /*
    switch(event)
    {
    case Qt::Key_W:
        panning = true;
        lastMousePosition = event->pos();
        break;
    case Qt::Key_A:
        panning = true;
        lastMousePosition = event->pos();
        break;
    case Qt::Key_S:
        panning = true;
        lastMousePosition = event->pos();
        break;
    case Qt::Key_D:
        panning = true;
        lastMousePosition = event->pos();
        break;
    }
    */

}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    if(draggingGate)
    {
        double worldX = screenToWorldX(event->position().x());
        double worldY = screenToWorldY(event->position().y());

        worldX -= dragOffset.x();
        worldY -= dragOffset.y();

        // Snap to fit grid
        double snapX = std::round(worldX / gridsize) * gridsize;
        double snapY = std::round(worldY / gridsize) * gridsize;

        andGate.setPosition(snapX, snapY);

        update();
    }

    if(panning)
    {
        QPoint currentPos = event->pos();

        int dx = currentPos.x() - lastMousePosition.x();
        int dy = currentPos.y() - lastMousePosition.y();

        cameraX -= dx / zoom;
        cameraY -= dy / zoom;

        lastMousePosition = currentPos;

        update();
    }
}

void Canvas::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
        draggingGate = false;
    if (event->button() == Qt::MiddleButton)
        panning = false;
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    QPointF mousePosition = event->position();

    double worldX = screenToWorldX(mousePosition.x());
    double worldY = screenToWorldY(mousePosition.y());

    if(event->angleDelta().y() > 0)
        zoom *= 1.1;
    else
        zoom /= 1.1;

    // Limit zoom
    zoom = std::clamp(zoom, 0.5, 10.0);

    // Adjust camera relative to mouse
    cameraX = worldX - mousePosition.x() / zoom;
    cameraY = worldY - mousePosition.y() / zoom;

    update();
}
