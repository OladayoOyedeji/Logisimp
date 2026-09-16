// Gates.h

#ifndef GATES_H
#define GATES_H

#include "Gate.h"

class AndGate : public Gate
{
public:
    AndGate(int id, int input_count = 2)
        : Gate(id, input_count)
    {}

    void evaluate(Simulator & simulator) 
    {
        LogicValue result = HIGH;

        for (int i = 0; i < input_count(); ++i)
        {
            result = logic_and(result, input_value(i));

            if (result == LOW)
            {
                break;
            }
        }

        drive_output(result, simulator);
    }

    void print(std::ostream & cout) const
    {
        cout << "AndGate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
};

class OrGate : public Gate
{
public:
    OrGate(int id, int input_count = 2)
        : Gate(id, input_count)
    {}

    void evaluate(Simulator & simulator) 
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

    void print(std::ostream & cout) const
    {
        cout << "OrGate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
};

class NotGate : public Gate
{
public:
    NotGate(int id)
        : Gate(id, 1)
    {}

    void evaluate(Simulator & simulator) 
    {
        drive_output(logic_not(input_value(0)), simulator);
    }

    void print(std::ostream & cout) const
    {
        cout << "NotGate("
             << "id=" << id()
             << ')';
    }
};

class NandGate : public Gate
{
public:
    NandGate(int id, int input_count = 2)
        : Gate(id, input_count)
    {}

    void evaluate(Simulator & simulator) 
    {
        LogicValue result = HIGH;

        for (int i = 0; i < input_count(); i++)
        {
            // Note that NAND is not associative,
            // which is why it gets not-ed at the end
            // and not here
            result = logic_and(result, input_value(i));

            if (result == LOW)
            {
                break;
            }
        }

        drive_output(logic_not(result), simulator);
    }

    void print(std::ostream & cout) const
    {
        cout << "NandGate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
};

class NorGate : public Gate
{
public:
    NorGate(int id, int input_count = 2)
        : Gate(id, input_count)
    {}

    void evaluate(Simulator & simulator) 
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

    void print(std::ostream & cout) const
    {
        cout << "NorGate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
};

class XorGate : public Gate
{
public:
    XorGate(int id, int input_count = 2)
        : Gate(id, input_count)
    {}

    void evaluate(Simulator & simulator) 
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

    void print(std::ostream & cout) const
    {
        cout << "XorGate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
};

class XnorGate : public Gate
{
public:
    XnorGate(int id, int input_count = 2)
        : Gate(id, input_count)
    {}

    void evaluate(Simulator & simulator) 
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

    void print(std::ostream & cout) const
    {
        cout << "XnorGate("
             << "id=" << id()
             << ", inputs=" << input_count()
             << ')';
    }
};

#endif // Gates.h
