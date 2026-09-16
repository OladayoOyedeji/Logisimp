// LogicValue.h

#ifndef LOGIC_VALUE_H
#define LOGIC_VALUE_H

#include <iostream>

enum LogicValue {
    LOW = 0,
    HIGH = 1,
    UNKNOWN = 2
};

// Shortens LogicValue to LV (only within this file though)
#define LV LogicValue

const LV AND_TABLE[3][3] = {
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { LOW,      LOW,        LOW     },
    /* a = HIGH    */ { LOW,      HIGH,       UNKNOWN },
    /* a = UNKNOWN */ { LOW,      UNKNOWN,    UNKNOWN }
};

const LV OR_TABLE[3][3] = {
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { LOW,      HIGH,       UNKNOWN },
    /* a = HIGH    */ { HIGH,     HIGH,       HIGH    },
    /* a = UNKNOWN */ { UNKNOWN,  HIGH,       UNKNOWN }
};

const LV NOT_TABLE[3] = {
    //                 input
    /* LOW     */      HIGH,
    /* HIGH    */      LOW,
    /* UNKNOWN */      UNKNOWN
};

const LV XOR_TABLE[3][3] = {
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { LOW,      HIGH,       UNKNOWN },
    /* a = HIGH    */ { HIGH,     LOW,        UNKNOWN },
    /* a = UNKNOWN */ { UNKNOWN,  UNKNOWN,    UNKNOWN }
};

const LV NAND_TABLE[3][3] = {
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { HIGH,     HIGH,       HIGH    },
    /* a = HIGH    */ { HIGH,     LOW,        UNKNOWN },
    /* a = UNKNOWN */ { HIGH,     UNKNOWN,    UNKNOWN }
};

const LV NOR_TABLE[3][3] = {
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { HIGH,     LOW,        UNKNOWN },
    /* a = HIGH    */ { LOW,      LOW,        LOW     },
    /* a = UNKNOWN */ { UNKNOWN,  LOW,        UNKNOWN }
};

const LV XNOR_TABLE[3][3] = {
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { HIGH,     LOW,        UNKNOWN },
    /* a = HIGH    */ { LOW,      HIGH,       UNKNOWN },
    /* a = UNKNOWN */ { UNKNOWN,  UNKNOWN,    UNKNOWN }
};

inline LV logic_and(LV a, LV b)
{
    return AND_TABLE[a][b];
}

inline LV logic_or(LV a, LV b)
{
    return OR_TABLE[a][b];
}

inline LV logic_not(LV value)
{
    return NOT_TABLE[value];
}

inline LV logic_xor(LV a, LV b)
{
    return XOR_TABLE[a][b];
}

inline LV logic_nand(LV a, LV b)
{
    return NAND_TABLE[a][b];
}

inline LV logic_nor(LV a, LV b)
{
    return NOR_TABLE[a][b];
}

inline LV logic_xnor(LV a, LV b)
{
    return XNOR_TABLE[a][b];
}

// undo macro for LogicValue
#undef LV

inline std::ostream & operator<<(std::ostream & cout, LogicValue value)
{
    if (value == LOW)
    {
        cout << "0";
    }
    else if (value == HIGH)
    {
        cout << "1";
    }
    else
    {
        cout << "X";
    }

    return cout;
}

#endif // LOGIC_VALUE_H
