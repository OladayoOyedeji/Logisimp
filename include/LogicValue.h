// LogicValue.h
/**
 * Three-state logic values (0, 1, X) (LOW, HIGH, UNKNOWN)
 * and their basic logical operations.
 */

#ifndef LOGIC_VALUE_H
#define LOGIC_VALUE_H

#include <iostream>
#include <string>

enum LogicValue
{
    LOW = 0,
    HIGH = 1,
    UNKNOWN = 2
};

LogicValue logic_and(LogicValue a, LogicValue b);
LogicValue logic_or(LogicValue a, LogicValue b);
LogicValue logic_not(LogicValue value);
LogicValue logic_xor(LogicValue a, LogicValue b);
LogicValue logic_nand(LogicValue a, LogicValue b);
LogicValue logic_nor(LogicValue a, LogicValue b);
LogicValue logic_xnor(LogicValue a, LogicValue b);

inline std::string to_string(LogicValue value)
{
    if (value == LOW)
    {
        return "0";
    }
    else if (value == HIGH)
    {
        return "1";
    }

    return "X";
}

inline std::ostream & operator<<(std::ostream & cout, LogicValue value)
{
    cout << to_string(value);
    return cout;
}

inline std::istream & operator>>(std::istream & cin, LogicValue & value)
{
    int v;
    cin >> v;

    if (v == 0)
    {
        value = LOW;
    }
    else if (v == 1)
    {
        value = HIGH;
    }
    else if (v == 2)
    {
        value = UNKNOWN;
    }
    else
    {
        std::cout << "No value\n";
    }

    return cin;
}

#endif // LOGIC_VALUE_H
