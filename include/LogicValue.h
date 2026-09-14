// LogicValue.h

#ifndef LOGIC_VALUE_H
#define LOGIC_VALUE_H

enum LogicValue {
    LOW = 0,
    HIGH = 1,
    UNKNOWN = 2
};

const LogicValue AND_TABLE[3][3] = {
    // b:               LOW      HIGH      UNKNOWN
    /* a = LOW     */ { LOW,     LOW,     LOW     },
    /* a = HIGH    */ { LOW,     HIGH,    UNKNOWN },
    /* a = UNKNOWN */ { LOW,     UNKNOWN, UNKNOWN }
};

const LogicValue OR_TABLE[3][3] = {
    // b:               LOW      HIGH      UNKNOWN
    /* a = LOW     */ { LOW,     HIGH,    UNKNOWN },
    /* a = HIGH    */ { HIGH,    HIGH,    HIGH    },
    /* a = UNKNOWN */ { UNKNOWN, HIGH,    UNKNOWN }
};

const LogicValue XOR_TABLE[3][3] = {
    // b:               LOW      HIGH      UNKNOWN
    /* a = LOW     */ { LOW,     HIGH,    UNKNOWN },
    /* a = HIGH    */ { HIGH,    LOW,     UNKNOWN },
    /* a = UNKNOWN */ { UNKNOWN, UNKNOWN, UNKNOWN }
};

const LogicValue NOT_TABLE[3] = {
    // input: LOW   HIGH   UNKNOWN
    HIGH, LOW,   UNKNOWN
};

inline LogicValue logicAnd(LogicValue a, LogicValue b)
{
    return AND_TABLE[a][b];
}

inline LogicValue logicOr(LogicValue a, LogicValue b)
{
    return OR_TABLE[a][b];
}

inline LogicValue logicXor(LogicValue a, LogicValue b)
{
    return XOR_TABLE[a][b];
}

inline LogicValue logicNot(LogicValue value)
{
    return NOT_TABLE[value];
}

inline LogicValue logicNand(LogicValue a, LogicValue b)
{
    return logicNot(logicAnd(a, b));
}

inline LogicValue logicNor(LogicValue a, LogicValue b)
{
    return logicNot(logicOr(a, b));
}

inline LogicValue logicXnor(LogicValue a, LogicValue b)
{
    return logicNot(logicXor(a, b));
}

#endif // LogicValue.h
