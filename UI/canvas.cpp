#include "canvas.h"
#include "wire.h"
#include "gates.h" //GUI Gates class
#include "Gates.h" //Logic Gates class
                   //Probably should change these names

#include "camera.h"

#include <iostream>

namespace gui
{
    Canvas::Canvas(QWidget *parent)
        : QOpenGLWidget(parent),
          spriteSheet("resources/Logisimp-spritesheet.png")
    {
        setFocusPolicy(Qt::StrongFocus);

        // Create Logical Circuit
        ::Circuit& circuit = projectController.create_circuit();

        // Open an Editor for logical circuit
        editorController = std::make_unique<EditorController>(
            projectController,
            circuit.id(),
            circuitLayout
            );
        
        // Testing initial gate setups
        ComponentId andId = editorController->add_component(
                                     ComponentKind::AndGate,
                                     CanvasPosition{300, 500});
        placeGate(new gui::AndGate(), andId, 300, 500);

        ComponentId orId = editorController->add_component(
                                     ComponentKind::OrGate,
                                     CanvasPosition{500, 500});
        placeGate(new gui::OrGate(), orId, 500, 500);

        ComponentId notId = editorController->add_component(
            ComponentKind::NotGate,
            CanvasPosition{700, 500}
            );

        placeGate(new gui::NotGate(), notId, 700, 500);

        cameraTimer.setInterval(16);
        connect(&cameraTimer, &QTimer::timeout,
                this, &Canvas::updateCamera);
        cameraTimer.start();
    }

    double Canvas::snapToGrid(double value)
    {
        return std::round(value/gridsize) * gridsize;
    }

    void Canvas::updateCamera()
    {
        camera.updateMovement(wPressed, sPressed, aPressed, dPressed, 5.0);
        update();
    }

    void Canvas::placeGate(Gate *newGate, ComponentId componentId,
                           double worldX, double worldY)
    {
        mousePosition = mapFromGlobal(QCursor::pos());
        newGate->setPosition(worldX, worldY);
        newGate->setComponentId(componentId);

        bindPorts(newGate, componentId);
        
        gates.push_back(newGate);

        update();
    }
    
    void Canvas::deleteSelectedGate()
    {
        if(selectedGate == NULL)
            return;

        Gate *gateToDelete = selectedGate;
        ComponentId id = gateToDelete->getComponentId();

        try
        {
            editorController->delete_component(id);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Unable to delete component: "
                      << e.what() << '\n';
            return;
        }

        // Cancel unfinished wires
        if(wireStart != NULL &&
           wireStart->getParentGate() == gateToDelete)
        {
            wireStart == NULL;
            drawingWire = false;
        }

        // Delete all gates attached to deleted gate
        for(auto it = wires.begin(); it != wires.end();)
        {
            Wire *wire = *it;

            bool connected =
                wire->getStart()->getParentGate() == gateToDelete ||
                wire->getEnd()->getParentGate() == gateToDelete;

            if(connected)
            {
                delete wire;
                it = wires.erase(it);
            }
            else
                ++it;
        }

        //Remove gate from the graphical gate vector
        auto it = std::find(
            gates.begin(),
            gates.end(),
            gateToDelete);

        if(it != gates.end())
            gates.erase(it);

        selectedGate = NULL;
        draggingGate = false;

        delete gateToDelete;

        refreshSimulation();
    }

    Wire* Canvas::findWire(const QPointF &screenPos) const
    {
        const double tolerance = 8.0;
        const double toleranceSquared = tolerance * tolerance;

        for(auto it = wires.rbegin(); it != wires.rend(); ++it)
        {
            Wire* wire = *it;

            Port* start = wire->getStart();
            Port* end = wire->getEnd();

            QPointF a = camera.worldToScreen(QPointF(start->getX(), start->getY()));
            QPointF b = camera.worldToScreen(QPointF(end->getX(), end->getY()));

            double dx = b.x() - a.x();
            double dy = b.y() - a.y();

            double lengthSquared = dx * dx + dy * dy;

            double t = 0.0;

            if (lengthSquared > 0.0)
            {
                t = ((screenPos.x() - a.x()) * dx + (screenPos.y() - a.y()) * dy) / lengthSquared;

                t = std::clamp(t, 0.0, 1.0);
            }

            double closestX = a.x() + t * dx;
            double closestY = a.y() + t * dy;

            double distanceX = screenPos.x() - closestX;
            double distanceY = screenPos.y() - closestY;

            double distanceSquared =
                distanceX * distanceX + distanceY * distanceY;

            if (distanceSquared <= toleranceSquared)
                return wire;
        }

        return NULL;
    }

    void Canvas::deleteWire(Wire* wire)
    {
        if(wire == NULL)
            return;

        PortId inputID = wire->getEnd()->getPortId();

        editorController->disconnect_port(inputID);

        auto it = std::find(wires.begin(), wires.end(), wire);

        if (it != wires.end())
        {
            delete *it;
            wires.erase(it);
        }

        refreshSimulation();
    }

    Port *Canvas::findport(double worldX, double worldY)
    {
        for(Gate *gate : gates)
        {
            for(Port *port : gate->getInputPorts())
            {
                if(port->contains(worldX, worldY))
                    return port;
            }

            for(Port *port : gate->getOutputPorts())
            {
                if(port->contains(worldX, worldY))
                    return port;
            }
        }

        return NULL;
    }

    void Canvas::bindPorts(Gate* visualGate, ComponentId componentId)
    {
        const auto& visualInputs = visualGate->getInputPorts();
        const auto& visualOutputs = visualGate->getOutputPorts();

        std::vector<PortId> logicalInputs =
            editorController->inputs(componentId);
        std::vector<PortId> logicalOutputs =
            editorController->outputs(componentId);

        // Verify Port Counts Match
        if (visualInputs.size() != logicalInputs.size() ||
            visualOutputs.size() != logicalOutputs.size())
        {
            throw std::runtime_error("Port count mismatch");
        }

        for (size_t i = 0; i < visualInputs.size(); ++i)
        {
            visualInputs[i]->setPortId(logicalInputs[i]);
        }

        for (size_t i = 0; i < visualOutputs.size(); ++i)
        {
            visualOutputs[i]->setPortId(logicalOutputs[i]);
        }

        // Debugging
        std::cout << "Component ID: " << componentId.value << '\n';

        for (Port* port : visualInputs)
        {
            PortId id = port->getPortId();

            std::cout << "Input port ID: "
                      << id.value
                      << " generation: " << id.generation
                      << '\n';
        }

        for (Port* port : visualOutputs)
        {
            PortId id = port->getPortId();

            std::cout << "Output port ID: "
                      << id.value
                      << " generation: " << id.generation
                      << '\n';
        }
    }

    bool Canvas::validConnection(Port* output, Port* input) const
    {
        // Both ports must exist.
        if (!output || !input)
            return false;

        // Require output-to-input connections.
        if (output->getType() != PortType::OUTPUT ||
            input->getType() != PortType::INPUT)
            return false;
        
        PortId outputId = output->getPortId();
        PortId inputId = input->getPortId();

        // Check for unassigned IDs.
        if (outputId.value < 0 || outputId.generation < 0 ||
            inputId.value < 0 || inputId.generation < 0)
            return false;

        // Reject connecting a gate to itself.
        if (output->getParentGate() == input->getParentGate())
            return false;

        // Prevent duplicate graphical wires.
        for (Wire* wire : wires)
        {
            if (wire->getStart() == output &&
                wire->getEnd() == input)
            {
                return false;
            }
        }

        // Prevent two outputs in one input
        if(inputOccupied(input))
            return false;

        return true;
    }

    bool Canvas::inputOccupied(Port* input) const
    {
        if(input == NULL)
            return false;

        for(Wire* wire : wires)
        {
            if(wire->getEnd() == input)
                return true;
        }
        return false;
    }

    void Canvas::refreshSimulation()
    {
        try
        {
            if (!editorController->run_simulation())
                std::cerr << "Simulation did not converge\n";

            update();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Simulation failed: "
                      << e.what() << '\n';
        }
    }

    /*
    void Canvas::setPlacementTool(PlacementTool tool)
    {
        activeTool = tool;
    }
*/

    void Canvas::drawGrid(QPainter &painter)
    {
        QPen gridPen(QColor(60, 60, 60));
        painter.setPen(gridPen);

        double startX = snapToGrid(camera.getX()); 
        double startY = snapToGrid(camera.getY());

        double right = camera.screenToWorldX(width());
        double bottom = camera.screenToWorldY(height());

        for(double x = startX; x < right; x += gridsize)
        {
            double screenX = camera.worldToScreenX(x);
            painter.drawLine(screenX, 0, screenX, height());
        }
        
        for(double y = startY; y < bottom; y += gridsize)
        {
            double screenY = camera.worldToScreenY(y);
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
        QPainter painter(this);

        // Draw Background
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw Grid
        drawGrid(painter);

        // Draw Wires
        for(Wire *wire : wires)
            drawWire(painter, wire);

        if(drawingWire)
            drawTempWire(painter);

        // Draw Gates / I/O Pins
        for(Gate *gate : gates)
        {
            // Update Output Display
            if(auto *output = dynamic_cast<gui::OutputPin*>(gate))
            {
                if(!output->getInputPorts().empty())
                {
                    PortId id = output->getInputPorts()[0]->getPortId();
                    output->setDisplayColor(signalColor(id));
                }
            }
            // Replace with sprites ^^^^^^^^^
            
            double screenX = camera.worldToScreenX(gate->getX());
            double screenY = camera.worldToScreenY(gate->getY());

            //gate->draw(painter, spriteSheet, screenX, screenY, zoom);
            gate->draw(painter, spriteSheet, screenX, screenY, camera.getZoom());

            for(Port *port : gate->getInputPorts())
            {
                drawPort(painter, port);
            }
            for(Port *port : gate->getOutputPorts())
            {
                drawPort(painter, port);
            }

            // Draws outline on selected gate
            if (gate == selectedGate)
            {
                double size = 60.0 * camera.getZoom();

                QPen selectionPen(QColor(0, 150, 255));
                selectionPen.setWidth(2);
                selectionPen.setStyle(Qt::DashLine);

                painter.setPen(selectionPen);
                painter.setBrush(Qt::NoBrush);

                painter.drawRect(
                    QRectF(
                        screenX - size / 2.0,
                        screenY - size / 2.0,
                        size,
                        size
                        )
                    );
            }
            // Replace with sprites ^^^^^^^^^
        }
    }

    void Canvas::resizeGL(int width, int height)
    {
        // Update Viewport/Projection
        glViewport(0, 0, width, height);
    }

    void Canvas::drawPort(QPainter &painter, Port *port)
    {
        double screenX = camera.worldToScreenX(port->getX());
        double screenY = camera.worldToScreenY(port->getY());

        double radius = 5.0 * camera.getZoom();

        QColor color = signalColor(port->getPortId());
        
        painter.setPen(QPen(color));
        painter.setBrush(QBrush(color));

        painter.drawEllipse(QPointF(screenX, screenY), radius, radius);
        painter.setBrush(Qt::NoBrush);
    }

    void Canvas::drawWire(QPainter &painter, Wire *wire)
    {
        Port *start = wire->getStart();
        Port *end = wire->getEnd();

        QColor color = signalColor(start->getPortId());
        
        QPen wirePen(color);
        wirePen.setWidthF(2.0);
        painter.setPen(wirePen);

        double x1 = camera.worldToScreenX(start->getX());
        double y1 = camera.worldToScreenY(start->getY());

        double x2 = camera.worldToScreenX(end->getX());
        double y2 = camera.worldToScreenY(end->getY());

        painter.drawLine(QPointF(x1, y1), QPointF(x2, y2));
    }

    void Canvas::drawTempWire(QPainter &painter)
    {
        QPen gridPen(QColor(128, 128, 128));
        painter.setPen(gridPen);

        if(wireStart == NULL)
            return;

        double startX = camera.worldToScreenX(wireStart->getX());
        double startY = camera.worldToScreenY(wireStart->getY());

        painter.drawLine(QPointF(startX, startY), mousePosition);
    }

    QColor Canvas::signalColor(PortId portId) const
    {
        LogicVector signal = editorController->value(portId);

        // The current primitive gates have one-bit ports.
        if (signal.width() != 1)
            return QColor(160, 160, 160);

        switch (signal[0])
        {
            case LOW:
                return QColor(100, 100, 100);

            case HIGH:
                return QColor(0, 220, 0);

            case UNKNOWN:
                return QColor(255, 210, 0);

            case ERROR:
                return QColor(255, 0, 0);
        }

        return QColor(160, 160, 160);
    }

    void Canvas::mousePressEvent(QMouseEvent *event)
    {
        mousePosition = event->position();

        setFocus();

        QPointF worldPos = camera.screenToWorld(event->position());

        double worldX = worldPos.x();
        double worldY = worldPos.y();

        if (event->button() == Qt::RightButton)
        {
            if (drawingWire)
            {
                drawingWire = false;
                wireStart = NULL;

                update();
            }

            deleteSelectedGate();
            deleteWire(findWire(mousePosition));

            return;
        }

        if(event->button() == Qt::LeftButton)
        {
            Port *clickedPort = findport(worldX, worldY);

            if(clickedPort != NULL)
            {
                // Start wire
                if(!drawingWire && clickedPort->getType() == PortType::OUTPUT)
                {
                    wireStart = clickedPort;
                    drawingWire = true;
                    
                    update();
                    return;
                }

                // Finish wire
                if(drawingWire && clickedPort->getType() == PortType::INPUT
                   && validConnection(wireStart, clickedPort))
                {
                    PortId outputId = wireStart->getPortId();
                    PortId inputId = clickedPort->getPortId();

                    try
                    {
                        editorController->connect_ports(outputId, inputId);

                        wires.push_back(new Wire(wireStart, clickedPort));

                        refreshSimulation();
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "Connection failed: "
                                  << e.what() << '\n';

                        wireStart = nullptr;
                        drawingWire = false;
                        update();
                        return;
                    }

                    wireStart = NULL;
                    drawingWire = false;

                    update();
                    return;
                }
            }

            if (drawingWire)
            {
                return;
            }

            selectedGate = NULL;
            draggingGate = false;

            for(auto it = gates.rbegin(); it != gates.rend(); ++it)
            {
                Gate *gate = *it;
                
                if(gate->contains(worldX, worldY))
                {
                    selectedGate = gate;
                    draggingGate = true;

                    dragOffset.setX(worldX - gate->getX());
                    dragOffset.setY(worldY - gate->getY());

                    break;
                }
            }

            update();
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

        QPointF worldPos = camera.screenToWorld(event->position());

        double worldX = worldPos.x();
        double worldY = worldPos.y();

        if(drawingWire)
            update();

        if(draggingGate && selectedGate != NULL)
        {
            double newX = worldX - dragOffset.x();
            double newY = worldY - dragOffset.y();

            double snapX = snapToGrid(newX);
            double snapY = snapToGrid(newY);

            if(selectedGate->getX() != snapX ||
               selectedGate->getY() != snapY)
            {
                try
                {
                    editorController->move_component(
                        selectedGate->getComponentId(),
                        CanvasPosition{snapX, snapY});
                
                    selectedGate->setPosition(snapX, snapY);
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Unable to move component: "
                              << e.what() << '\n';
                }
            }
            
            update();
        }

        if(panning)
        {
            QPoint currentPos = event->pos();

            int dx = currentPos.x() - lastMousePosition.x();
            int dy = currentPos.y() - lastMousePosition.y();

            camera.moveScreen(dx, dy);
            
            lastMousePosition = currentPos;

            update();
        }
    }

    void Canvas::mouseReleaseEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::LeftButton)
        {
            draggingGate = false;
        }
        if (event->button() == Qt::MiddleButton)
            panning = false;
    }
    
    void Canvas::mouseDoubleClickEvent(QMouseEvent* event)
    {
        if (event->button() != Qt::LeftButton)
            return;
        
        QPointF worldPos = camera.screenToWorld(event->position());

        double worldX = worldPos.x();
        double worldY = worldPos.y();

        for (Gate *gate : gates)
        {
            gui::InputPin *input = dynamic_cast<gui::InputPin*>(gate);

            if (input == NULL)
                continue;

            if (!input->contains(worldX, worldY))
                continue;

            try
            {
                editorController->toggle_input(input->getComponentId());
                refreshSimulation();
            }
            catch (const std::exception& e)
            {
                std::cerr << "Input toggle failed: "
                          << e.what() << '\n';
            }

            return;
        }
    }

    void Canvas::wheelEvent(QWheelEvent *event)
    {
        int delta = event->angleDelta().y();

        if(delta == 0)
            return;

        double factor = delta > 0 ? 1.1 : 1.0 / 1.1;

        camera.zoomAt(event->position(), factor);

        update();
    }

    void Canvas::keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_R && !event->isAutoRepeat())
        {
            refreshSimulation();
            return;
        }
        
        if(event->key() >= Qt::Key_1 &&
           event-> key() <= Qt::Key_5 &&
           !event->isAutoRepeat())
        {
            QPointF screenPos = mapFromGlobal(QCursor::pos()).toPointF();

            QPointF worldPos = camera.screenToWorld(screenPos);
            
            double worldX = snapToGrid(worldPos.x());
            double worldY = snapToGrid(worldPos.y());

            ComponentKind kind;
            Gate* visualGate = NULL;

            switch(event->key())
            {
                case Qt::Key_1:
                    kind = ComponentKind::AndGate;
                    visualGate = new gui::AndGate();
                    break;
                case Qt::Key_2:
                    kind = ComponentKind::OrGate;
                    visualGate = new gui::OrGate();
                    break;
                case Qt::Key_3:
                    kind = ComponentKind::NotGate;
                    visualGate = new gui::NotGate();
                    break;
                case Qt::Key_4:
                    kind = ComponentKind::InputPin;
                    visualGate = new gui::InputPin();
                    break;
                case Qt::Key_5:
                    kind = ComponentKind::OutputPin;
                    visualGate = new gui::OutputPin();
                    break;
            }

            ComponentId id = editorController->add_component(
                    kind,
                    CanvasPosition{worldX, worldY}
                    );

            placeGate(visualGate, id, worldX, worldY);
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
}
