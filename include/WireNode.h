/**
 * @file WireNode.h
 * @brief Junction or endpoint in a circuit's wire graph.
 * @author Cole Schwandt
 */

#ifndef WIRE_NODE_H
#define WIRE_NODE_H

#include <ostream>
#include <string>
#include <vector>

class Circuit;
class Net;
class Port;
class WireEdge;

class WireNode
{
public:
    WireNode(const WireNode & other) = delete;

    WireNode & operator=(const WireNode & other) = delete;

    WireNode(WireNode && other) = delete;

    WireNode & operator=(WireNode && other) = delete;

    ~WireNode() {}

    int id() const { return id_; }

    Circuit * circuit() const { return circuit_; }

    int width() const { return width_; }

    std::string & label() { return label_; }

    std::string label() const { return label_; }

    const std::vector< WireEdge * > & edges() const { return edges_; }

    const std::vector< Port * > & ports() const { return ports_; }

    Net * net() const { return net_; }

    std::string to_string() const;

private:
    friend class Circuit;

    WireNode(Circuit & circuit, int id, int width = 0, const std::string & label = "");

    int id_;
    Circuit * circuit_;
    int width_;
    std::string label_;
    std::vector< WireEdge * > edges_;
    std::vector< Port * > ports_;
    Net * net_;
};

inline std::ostream & operator<<(std::ostream & cout, const WireNode & node)
{
    cout << node.to_string();
    return cout;
}

#endif // WIRE_NODE_H
