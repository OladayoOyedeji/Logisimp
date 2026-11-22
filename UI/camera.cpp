#include "camera.h"

#include <algorithm>

namespace gui
{
    
    void Camera::setPosition(double x, double y)
    {
        cameraX = x;
        cameraY = y;
    }

    void Camera::move(double dx, double dy)
    {
        cameraX += dx;
        cameraY += dy;
    }

    void Camera::moveScreen(double dx, double dy)
    {
        move(-dx / zoom, -dy / zoom);
    }

    double Camera::getX() const
    {
        return cameraX;
    }

    double Camera::getY() const
    {
        return cameraY;
    }

    void Camera::setZoom(double value)
    {
        zoom = std::clamp(value, minZoom, maxZoom);
    }

    void Camera::zoomBy(double factor)
    {
        if(factor <= 0.0)
            return;

        setZoom(zoom * factor);
    }

    double Camera::getZoom() const
    {
        return zoom;
    }

    void Camera::zoomAt(const QPointF& screenPoint, double factor)
    {
        if(factor <= 0.0)
            return;

        QPointF worldPoint = screenToWorld(screenPoint);

        zoomBy(factor);

        setPosition(worldPoint.x() - screenPoint.x() / zoom,
                    worldPoint.y() - screenPoint.y() / zoom);
    }
    
    double Camera::screenToWorldX(double screenX) const
    {
        return screenX / zoom + cameraX;
    }

    double Camera::screenToWorldY(double screenY) const
    {
        return screenY / zoom + cameraY;
    }

    double Camera::worldToScreenX(double worldX) const
    {
        return (worldX - cameraX) * zoom;
    }

    double Camera::worldToScreenY(double worldY) const
    {
        return (worldY - cameraY) * zoom;
    }

    QPointF Camera::screenToWorld(const QPointF& point) const
    {
        return QPointF(
            screenToWorldX(point.x()),
            screenToWorldY(point.y()));
    }

    QPointF Camera::worldToScreen(const QPointF& point) const
    {
        return QPointF(
            worldToScreenX(point.x()),
            worldToScreenY(point.y()));
    }

    void Camera::updateMovement(
        bool up,
        bool down,
        bool left,
        bool right,
        double speed)
    {
        double dx = 0.0;
        double dy = 0.0;

        if(up)
            dy -= speed / zoom;
        if(down)
            dy += speed / zoom;
        if(left)
            dx -= speed / zoom;
        if(right)
            dx += speed / zoom;

        move(dx, dy);
    }
}
