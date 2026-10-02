// Gates.h
/**
 * Primitive logic gate components, such as AND, OR, NOT, etc.
 */

#ifndef GATES_H
#define GATES_H

#include "Gate.h"

class AndGate : public Gate
{
public:
    AndGate(int id, int input_count = 2);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

class OrGate : public Gate
{
public:
    OrGate(int id, int input_count = 2);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

class NotGate : public Gate
{
public:
    NotGate(int id);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

class NandGate : public Gate
{
public:
    NandGate(int id, int input_count = 2);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

class NorGate : public Gate
{
public:
    NorGate(int id, int input_count = 2);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

class XorGate : public Gate
{
public:
    XorGate(int id, int input_count = 2);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

class XnorGate : public Gate
{
public:
    XnorGate(int id, int input_count = 2);

    void evaluate(Simulator & simulator) override;
    std::string to_string() const override;
};

#endif // GATES_H
