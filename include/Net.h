/**
 * @file Net.h
 * @brief Describes the electrical connections derived from a circuit's wire graph.
 * @author Cole Schwandt
 */

#ifndef NET_H
#define NET_H

#include <ostream>
#include <string>
#include <vector>

class Circuit;
class Port;
class WireNode;

class Net
{
public:
    Net(const Net & other) = delete;

    Net & operator=(const Net & other) = delete;

    Net(Net && other) = delete;

    Net & operator=(Net && other) = delete;

    ~Net() {}

    int id() const { return id_; }

    Circuit * circuit() const { return circuit_; }

    std::string label() const;

    Port * driver() const { return driver_; }

    const std::vector< Port * > & listeners() const { return listeners_; }

    const std::vector< Port * > & ports() const { return ports_; }

    const std::vector< WireNode * > & nodes() const { return nodes_; }

    int width() const { return width_; }

    std::string to_string() const;

private:
    friend class Circuit;

    Net(Circuit & circuit, int id, int width);

    int id_;
    Circuit * circuit_;
    int width_;
    Port * driver_;
    std::vector< Port * > listeners_;
    std::vector< Port * > ports_;
    std::vector< WireNode * > nodes_;
};

inline std::ostream & operator<<(std::ostream & cout, const Net & net)
{
    cout << net.to_string();
    return cout;
}

#endif // NET_H
