// Port.h

#ifndef PORT_H
#define PORT_H

#include <string>

class Component;
class Signal;

enum PortDirection
{
    INPUT,
    OUTPUT
};

class Port
{
public:
    Port(int id, int width, PortDirection direction, const std::string & label = "")
        : id_(id),
          width_(width),
          direction_(direction),
          owner_(nullptr),
          signal_(nullptr),
          label_(label)
    {
        if (width <= 0)
        {
            throw std::invalid_argument("Port width must be positive");
        }
    }

    Port(const Port & other) = delete;
    Port & operator=(const Port & other) = delete;

    int & id() { return id_; }
    int id() const { return id_; }

    int & width() { return width_; }
    int width() const { return width_; }

    PortDirection & direction() { return direction_; }
    PortDirection direction() const { return direction_; }

    Component * & owner() { return owner_; }
    Component * owner() const { return owner_; }

    Signal * & signal() { return signal_; }
    Signal * signal() const { return signal_; }

    std::string & label() { return label_; }
    std::string label() const { return label_; }

    bool connected() const
    {
        return signal_ != nullptr;
    }

private:
    int id_;
    int width_;
    PortDirection direction_;
    Component * owner_;
    Signal * signal_;
    std::string label_;
};

#endif // Port.h
