// Gates.cpp

#include "Gates.h"

AndGate::AndGate(int id, int input_count)
    : Gate(id, input_count)
{}

void AndGate::evaluate(Simulator & simulator)
{
    LogicValue result = HIGH;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_and(result, input_value(i));

        if (result == LOW)
        {
            break;
        }
    }

    drive_output(result, simulator);
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

void OrGate::evaluate(Simulator & simulator)
{
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_or(result, input_value(i));

        if (result == HIGH)
        {
            break;
        }
    }

    drive_output(result, simulator);
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

void NotGate::evaluate(Simulator & simulator)
{
    drive_output(logic_not(input_value(0)), simulator);
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

void NandGate::evaluate(Simulator & simulator)
{
    LogicValue result = HIGH;

    for (int i = 0; i < input_count(); i++)
    {
        // NAND is not associative, which is why the result
        // is inverted after all inputs have been evaluated.
        result = logic_and(result, input_value(i));

        if (result == LOW)
        {
            break;
        }
    }

    drive_output(logic_not(result), simulator);
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

void NorGate::evaluate(Simulator & simulator)
{
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_or(result, input_value(i));

        if (result == HIGH)
        {
            break;
        }
    }

    drive_output(logic_not(result), simulator);
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

void XorGate::evaluate(Simulator & simulator)
{
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_xor(result, input_value(i));

        if (result == UNKNOWN)
        {
            break;
        }
    }

    drive_output(result, simulator);
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

void XnorGate::evaluate(Simulator & simulator)
{
    LogicValue result = LOW;

    for (int i = 0; i < input_count(); i++)
    {
        result = logic_xor(result, input_value(i));

        if (result == UNKNOWN)
        {
            break;
        }
    }

    drive_output(logic_not(result), simulator);
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
