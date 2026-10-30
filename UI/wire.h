#pragma once

namespace gui
{
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
}
