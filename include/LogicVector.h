// LogicVector.h
/**
 * Fixed-width collection of three-state logic values.
 */

#ifndef LOGIC_VECTOR_H
#define LOGIC_VECTOR_H

#include "LogicValue.h"

#include <ostream>
#include <string>
#include <vector>

class LogicVector
{
public:
    LogicVector(int width = 1, LogicValue initial_value = UNKNOWN);

    std::vector< LogicValue > & bits() { return bits_; }
    std::vector< LogicValue > bits() const { return bits_; }

    int width() const { return int(bits_.size()); }

    LogicValue & operator[](int index);
    LogicValue operator[](int index) const;

    bool operator==(const LogicVector & other) const;
    bool operator!=(const LogicVector & other) const;

    bool fully_known() const;

    std::string to_string() const;

private:
    std::vector< LogicValue > bits_;
};

inline std::ostream & operator<<(std::ostream & cout, const LogicVector & value)
{
    cout << value.to_string();
    return cout;
}

#endif // LOGIC_VECTOR_H
