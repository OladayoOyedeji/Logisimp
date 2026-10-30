/**
 * @file LogicValue.h
 * @brief Four-state logic values (LOW, HIGH, UNKNOWN, ERROR) and their basic operations.
 * @author Cole Schwandt
 */

#ifndef LOGIC_VALUE_H
#define LOGIC_VALUE_H

#include <iostream>
#include <string>

/**
 * @brief Represents four logical values: 0, 1, X, and E.
 *
 * UNKNOWN remains undetermined through gate operations unless a controlling
 * LOW fixes AND or a controlling HIGH fixes OR. ERROR propagates through gate
 * operations with the same controlling-value exceptions.
 */
enum LogicValue
{
    LOW = 0, ///< Logic 0.
    HIGH = 1, ///< Logic 1.
    UNKNOWN = 2, ///< Unknown value, such as an undriven wire.
    ERROR = 3 ///< Invalid signal, such as conflicting output drivers.
};

/**
 * @brief Computes the logical AND of two values.
 * @param a First operand.
 * @param b Second operand.
 * @return Result of the AND operation.
 */
LogicValue logic_and(LogicValue a, LogicValue b);

/**
 * @brief Computes the logical OR of two values.
 * @param a First operand.
 * @param b Second operand.
 * @return Result of the OR operation.
 */
LogicValue logic_or(LogicValue a, LogicValue b);

/**
 * @brief Computes the logical NOT of a values.
 * @param value logical value to be NOTed.
 * @return Result of the NOT operation.
 */
LogicValue logic_not(LogicValue value);

/**
 * @brief Computes the logical XOR of two values.
 * @param a First operand.
 * @param b Second operand.
 * @return Result of the XOR operation.
 */
LogicValue logic_xor(LogicValue a, LogicValue b);

/**
 * @brief Computes the logical NAND of two values.
 * @param a First operand.
 * @param b Second operand.
 * @return Result of the NAND operation.
 */
LogicValue logic_nand(LogicValue a, LogicValue b);

/**
 * @brief Computes the logical NOR of two values.
 * @param a First operand.
 * @param b Second operand.
 * @return Result of the NOR operation.
 */
LogicValue logic_nor(LogicValue a, LogicValue b);

/**
 * @brief Computes the logical XNOR of two values.
 * @param a First operand.
 * @param b Second operand.
 * @return Result of the XNOR operation.
 */
LogicValue logic_xnor(LogicValue a, LogicValue b);

/**
 * @brief Converts a logic value to its string representation.
 * @param value Logic value to convert.
 * @return "0", "1", "X", or "E".
 */
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
    else if (value == ERROR)
    {
        return "E";
    }

    return "X";
}

/**
 * @brief Writes a logic value to an output stream.
 * @param cout Output stream.
 * @param value Logic value to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, LogicValue value)
{
    cout << to_string(value);
    return cout;
}

/**
 * @brief Reads a logic value from an input stream.
 * @param cin Input stream.
 * @param value Logic value to update.
 * @return Reference to the input stream.
 * @note Accepts 0 (LOW), 1 (HIGH), 2 or X/x (UNKNOWN), and 3 or E/e (ERROR).
 * @note Invalid input leaves the logic value unchanged and sets the stream's fail state.
 */
inline std::istream & operator>>(std::istream & cin, LogicValue & value)
{
    std::string input;

    if (!(cin >> input))
    {
        return cin;
    }

    if (input == "0")
    {
        value = LOW;
    }
    else if (input == "1")
    {
        value = HIGH;
    }
    else if (input == "2" || input == "X" || input == "x")
    {
        value = UNKNOWN;
    }
    else if (input == "3" || input == "E" || input == "e")
    {
        value = ERROR;
    }
    else
    {
        cin.setstate(std::ios::failbit);
    }

    return cin;
}

#endif // LOGIC_VALUE_H
