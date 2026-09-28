#include "canvas.h"

#include <iostream>

Canvas::Canvas(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    gates.push_back(new AndGate());
    gates.push_back(new OrGate());
    gates.push_back(new NotGate());

    gates[0]->setPosition(300, 500);
    gates[1]->setPosition(500, 500);
    gates[2]->setPosition(700, 500);

    // ---- MOVE CAMERA ----
    cameraTimer.setInterval(16);

    connect(&cameraTimer, &QTimer::timeout,
            this, &Canvas::updateCamera);

    cameraTimer.start();
    // ---- MOVE CAMERA ----
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
    return std::round(value/gridsize) * gridsize;
}

void Canvas::updateCamera()
{
    double speed = 5.0;

    if (wPressed)
        cameraY -= speed / zoom;
    if (aPressed)
        cameraX -= speed / zoom;
    if (sPressed)
        cameraY += speed / zoom;
    if (dPressed)
        cameraX += speed / zoom;

    update();
}

void Canvas::placeGate(Gate *newGate)
{
    mousePosition = mapFromGlobal(QCursor::pos());

    double worldX = screenToWorldX(mousePosition.x());
    double worldY = screenToWorldY(mousePosition.y());

    worldX = snapToGrid(worldX);
    worldY = snapToGrid(worldY);

    newGate->setPosition(snapToGrid(worldX), snapToGrid(worldY));

    gates.push_back(newGate);

    update();
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

    double screenX;
    double screenY;


    for(Gate *gate : gates)
    {
        double screenX = worldToScreenX(gate->getX());
        double screenY = worldToScreenY(gate->getY());

        gate->draw(painter, screenX, screenY, zoom);

        std::cout << "Drawing Gate" << '\n';
        gate->get_gate();
        std::cout << "X pos: " << gate->getX() << " Y pos: " << gate->getY() << '\n';
    }

    // Draw I/O pins
}

void Canvas::resizeGL(int width, int height)
{
    // Update Viewport/Projection
    glViewport(0, 0, width, height);
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
    mousePosition = event->position();

    setFocus();

    if(event->button() == Qt::LeftButton)
    {
        double worldX = screenToWorldX(event->position().x());
        double worldY = screenToWorldY(event->position().y());

        for(Gate *gate : gates)
        {
            if(gate->contains(worldX, worldY))
            {
                selectedGate = gate;
                draggingGate = true;

                dragOffset.setX(worldX - gate->getX());
                dragOffset.setY(worldY - gate->getY());

                break;
            }
        }
    }

    if (event->button() == Qt::MiddleButton)
    {
        panning = true;
        lastMousePosition = event->pos();
    }
}

void Canvas::mouseMoveEvent(QMouseEvent *event)
{
    mousePosition = event->position();

    if(draggingGate)
    {
        double worldX = screenToWorldX(event->position().x());
        double worldY = screenToWorldY(event->position().y());

        worldX -= dragOffset.x();
        worldY -= dragOffset.y();

        // Snap to fit grid
        double snapX = std::round(worldX / gridsize) * gridsize;
        double snapY = std::round(worldY / gridsize) * gridsize;

        selectedGate->setPosition(snapX, snapY);

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
    mousePosition = event->position();

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

void Canvas::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_1)
    {
        Gate *newGate = new AndGate;

        placeGate(newGate);
    }
    if (event->key() == Qt::Key_2)
    {
        Gate *newGate = new OrGate;

        placeGate(newGate);
    }
    if (event->key() == Qt::Key_3)
    {
        Gate *newGate = new NotGate;

        placeGate(newGate);
    }

    if(event->key() == Qt::Key_W)
        wPressed = true;
    if(event->key() == Qt::Key_A)
        aPressed = true;
    if(event->key() == Qt::Key_S)
        sPressed = true;
    if(event->key() == Qt::Key_D)
        dPressed = true;
}

void Canvas::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W)
        wPressed = false;
    if (event->key() == Qt::Key_A)
        aPressed = false;
    if (event->key() == Qt::Key_S)
        sPressed = false;
    if (event->key() == Qt::Key_D)
        dPressed = false;
}
