// File: camera.h
// Author: Ethan Graham

#pragma once

#include <QPointF>

namespace gui
{

    class Camera
    {
      public:
        Camera() = default;

        void setPosition(double, double);
        void move(double, double);         // Move by world units
        void moveScreen(double, double);   // Move by screen pixels, accounting for zoom

        double getX() const;
        double getY() const;

        void setZoom(double);
        void zoomBy(double);
        void zoomAt(const QPointF&, double);
        double getZoom() const;

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

        QPointF screenToWorld(const QPointF&) const;
        QPointF worldToScreen(const QPointF&) const;

        void updateMovement(bool, bool, bool, bool, double);

      private:
        double cameraX = 0.0, cameraY = 0.0;
        double zoom = 1.0;

        double minZoom = 0.5;
        double maxZoom = 10.0;
        
    };

}
