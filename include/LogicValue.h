// LogicValue.h

#ifndef LOGIC_VALUE_H
#define LOGIC_VALUE_H

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

inline LV logicAnd(LV a, LV b)
{
    return AND_TABLE[a][b];
}

inline LV logicOr(LV a, LV b)
{
    return OR_TABLE[a][b];
}

inline LV logicNot(LV value)
{
    return NOT_TABLE[value];
}

inline LV logicXor(LV a, LV b)
{
    return XOR_TABLE[a][b];
}

inline LV logicNand(LV a, LV b)
{
    return NAND_TABLE[a][b];
}

inline LV logicNor(LV a, LV b)
{
    return NOR_TABLE[a][b];
}

inline LV logicXnor(LV a, LV b)
{
    return XNOR_TABLE[a][b];
}

// undo macro for LogicValue
#undef LV

#endif // LOGIC_VALUE_H
