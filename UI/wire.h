#ifndef WIRE_H
#define WIRE_H

class Port;

class Wire
{
public:
    Wire(Port*, Port*);

    Port *getStart() const;
    Port *getEnd() const;
private:
    Port *start;
    Port *end;
};

#endif // WIRE_H
