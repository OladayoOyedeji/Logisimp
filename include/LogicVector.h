/**
 * @file LogicVector.h
 * @brief Vector of @ref LogicValue elements.
 * @author Cole Schwandt
 */

#ifndef LOGIC_VECTOR_H
#define LOGIC_VECTOR_H

#include "LogicValue.h"

#include <ostream>
#include <string>
#include <vector>

/**
 * @brief Vector of @ref LogicValue elements.
 */
class LogicVector
{
public:
    /**
     * @brief Constructs a logic vector of width width where every value is initial_value.
     */
    LogicVector(int width = 1, LogicValue initial_value = UNKNOWN);

    /**
     * @brief Gets the vector of logic values.
     * @return Reference to the vector of logic values.
     */
    std::vector< LogicValue > & bits() { return bits_; }

    /**
     * @brief Gets the vector of logic values.
     * @return Copy of the vector of logic values.
     */
    std::vector< LogicValue > bits() const { return bits_; }

    /**
     * @brief Gets the number of bits in the logic vector.
     * @return Number of bits.
     */
    int width() const { return int(bits_.size()); }

    /**
     * @brief Accesses a bit at the specified index.
     * @param index Index of the bit.
     * @return Reference to the bit.
     */
    LogicValue & operator[](int index);

    /**
     * @brief Accesses a bit at the specified index.
     * @param index Index of the bit.
     * @return Copy of the bit.
     */
    LogicValue operator[](int index) const;

    /**
     * @brief Checks whether two logic vectors are equal.
     * @param other Logic vector to compare against.
     * @return True if the vectors are equal, false otherwise.
     */
    bool operator==(const LogicVector & other) const;

     /**
     * @brief Checks whether two logic vectors are different.
     * @param other Logic vector to compare against.
     * @return False if the vectors are equal, true otherwise
     */
    bool operator!=(const LogicVector & other) const;

     /**
     * @brief Checks whether every bit has a known binary value.
     * @return True if every bit is LOW or HIGH, false for UNKNOWN or ERROR.
     */
    bool fully_known() const;

     /**
     * @brief Converts the logic vector to its string representation.
     * @return String representation of the bits.
     */
    std::string to_string() const;

private:
    std::vector< LogicValue > bits_; ///< Vector of logic values, such as [LOW, HIGH, UNKNOWN, ERROR].
};

/**
 * @brief Writes a logic vector to an output stream.
 * @param cout Output stream.
 * @param value Logic vector to write.
 * @return Reference to the output stream.
 */
inline std::ostream & operator<<(std::ostream & cout, const LogicVector & value)
{
    cout << value.to_string();
    return cout;
}

#endif // LOGIC_VECTOR_H
