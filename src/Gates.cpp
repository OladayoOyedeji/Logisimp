/**
 * @file Gates.cpp
 * @brief Implements primitive logic gates, such as AND, OR, and NOT.
 * @author Cole Schwandt
 */

#include "Gates.h"

AndGate::AndGate(int id, int input_count)
    : Gate(id, input_count)
{}

void AndGate::evaluate(Simulator & simulator, CircuitState & state)
{
    // HIGH is the AND identity, so the first input is not changed by the fold.
    LogicValue result = HIGH;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_and(result, input_value(i, state));

        if (result == LOW)
        {
            // LOW controls AND even if a remaining input is UNKNOWN.
            break;
        }
    }

    drive_output(result, simulator, state);
}

std::string AndGate::to_string() const
{
    std::string ret;

    ret += "AndGate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}

OrGate::OrGate(int id, int input_count)
    : Gate(id, input_count)
{}

void OrGate::evaluate(Simulator & simulator, CircuitState & state)
{
    // LOW is the OR identity, so the fold starts without forcing HIGH.
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_or(result, input_value(i, state));

        if (result == HIGH)
        {
            // HIGH controls OR even if a remaining input is UNKNOWN.
            break;
        }
    }

    drive_output(result, simulator, state);
}

std::string OrGate::to_string() const
{
    std::string ret;

    ret += "OrGate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}

NotGate::NotGate(int id)
    : Gate(id, 1)
{}

void NotGate::evaluate(Simulator & simulator, CircuitState & state)
{
    drive_output(logic_not(input_value(0, state)), simulator, state);
}

std::string NotGate::to_string() const
{
    std::string ret;

    ret += "NotGate(id=";
    ret += std::to_string(id());
    ret += ')';

    return ret;
}

NandGate::NandGate(int id, int input_count)
    : Gate(id, input_count)
{}

void NandGate::evaluate(Simulator & simulator, CircuitState & state)
{
    // Reduce with associative AND first; repeated binary NAND would alternate
    // inversions and would not represent a multi-input NAND gate.
    LogicValue result = HIGH;

    for (int i = 0; i < input_count(); i++)
    {
        // NAND is not associative, which is why the result
        // is inverted after all inputs have been evaluated.
        result = logic_and(result, input_value(i, state));

        if (result == LOW)
        {
            // A controlling LOW fixes the final inverted result to HIGH.
            break;
        }
    }

    drive_output(logic_not(result), simulator, state);
}

std::string NandGate::to_string() const
{
    std::string ret;

    ret += "NandGate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}

NorGate::NorGate(int id, int input_count)
    : Gate(id, input_count)
{}

void NorGate::evaluate(Simulator & simulator, CircuitState & state)
{
    // Invert the completed OR reduction once; binary NOR is not associative.
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_or(result, input_value(i, state));

        if (result == HIGH)
        {
            // A controlling HIGH fixes the final inverted result to LOW.
            break;
        }
    }

    drive_output(logic_not(result), simulator, state);
}

std::string NorGate::to_string() const
{
    std::string ret;

    ret += "NorGate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}

XorGate::XorGate(int id, int input_count)
    : Gate(id, input_count)
{}

void XorGate::evaluate(Simulator & simulator, CircuitState & state)
{
    // LOW is the XOR identity; each known HIGH toggles the accumulated parity.
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_xor(result, input_value(i, state));

        if (result == UNKNOWN)
        {
            // Unlike AND/OR, later known inputs cannot resolve unknown parity.
            break;
        }
    }

    drive_output(result, simulator, state);
}

std::string XorGate::to_string() const
{
    std::string ret;

    ret += "XorGate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}

XnorGate::XnorGate(int id, int input_count)
    : Gate(id, input_count)
{}

void XnorGate::evaluate(Simulator & simulator, CircuitState & state)
{
    // Complement the full XOR parity once, including for more than two inputs.
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_xor(result, input_value(i, state));

        if (result == UNKNOWN)
        {
            // Unknown parity remains unknown under both XOR and final inversion.
            break;
        }
    }

    drive_output(logic_not(result), simulator, state);
}

std::string XnorGate::to_string() const
{
    std::string ret;

    ret += "XnorGate(id=";
    ret += std::to_string(id());
    ret += ", inputs=";
    ret += std::to_string(input_count());
    ret += ')';

    return ret;
}
