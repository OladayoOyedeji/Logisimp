/**
 * @file WireEdge.h
 * @brief Connects two wire nodes in a circuit.
 * @author Cole Schwandt
 */

#ifndef WIRE_EDGE_H
#define WIRE_EDGE_H

#include <ostream>
#include <string>

class Circuit;
class WireNode;

class WireEdge
{
public:
    WireEdge(const WireEdge & other) = delete;

    WireEdge & operator=(const WireEdge & other) = delete;

    WireEdge(WireEdge && other) = delete;

    WireEdge & operator=(WireEdge && other) = delete;

    ~WireEdge() {}

    int id() const { return id_; }

    Circuit * circuit() const { return circuit_; }

    WireNode * first() const { return first_; }

    WireNode * second() const { return second_; }

    std::string to_string() const;

private:
    friend class Circuit;

    WireEdge(Circuit & circuit, int id, WireNode & first, WireNode & second);

    int id_;
    Circuit * circuit_;
    WireNode * first_;
    WireNode * second_;
};

inline std::ostream & operator<<(std::ostream & cout, const WireEdge & edge)
{
    cout << edge.to_string();
    return cout;
}

#endif // WIRE_EDGE_H
