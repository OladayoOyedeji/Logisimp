// LogicValue.cpp

#include "LogicValue.h"

static const LogicValue AND_TABLE[3][3] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { LOW,      LOW,        LOW     },
    /* a = HIGH    */ { LOW,      HIGH,       UNKNOWN },
    /* a = UNKNOWN */ { LOW,      UNKNOWN,    UNKNOWN }
};

static const LogicValue OR_TABLE[3][3] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { LOW,      HIGH,       UNKNOWN },
    /* a = HIGH    */ { HIGH,     HIGH,       HIGH    },
    /* a = UNKNOWN */ { UNKNOWN,  HIGH,       UNKNOWN }
};

static const LogicValue NOT_TABLE[3] =
{
    //                 input
    /* LOW     */      HIGH,
    /* HIGH    */      LOW,
    /* UNKNOWN */      UNKNOWN
};

static const LogicValue XOR_TABLE[3][3] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { LOW,      HIGH,       UNKNOWN },
    /* a = HIGH    */ { HIGH,     LOW,        UNKNOWN },
    /* a = UNKNOWN */ { UNKNOWN,  UNKNOWN,    UNKNOWN }
};

static const LogicValue NAND_TABLE[3][3] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { HIGH,     HIGH,       HIGH    },
    /* a = HIGH    */ { HIGH,     LOW,        UNKNOWN },
    /* a = UNKNOWN */ { HIGH,     UNKNOWN,    UNKNOWN }
};

static const LogicValue NOR_TABLE[3][3] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { HIGH,     LOW,        UNKNOWN },
    /* a = HIGH    */ { LOW,      LOW,        LOW     },
    /* a = UNKNOWN */ { UNKNOWN,  LOW,        UNKNOWN }
};

static const LogicValue XNOR_TABLE[3][3] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN
    /* a = LOW     */ { HIGH,     LOW,        UNKNOWN },
    /* a = HIGH    */ { LOW,      HIGH,       UNKNOWN },
    /* a = UNKNOWN */ { UNKNOWN,  UNKNOWN,    UNKNOWN }
};

LogicValue logic_and(LogicValue a, LogicValue b)
{
    return AND_TABLE[a][b];
}

LogicValue logic_or(LogicValue a, LogicValue b)
{
    return OR_TABLE[a][b];
}

LogicValue logic_not(LogicValue value)
{
    return NOT_TABLE[value];
}

LogicValue logic_xor(LogicValue a, LogicValue b)
{
    return XOR_TABLE[a][b];
}

LogicValue logic_nand(LogicValue a, LogicValue b)
{
    return NAND_TABLE[a][b];
}

LogicValue logic_nor(LogicValue a, LogicValue b)
{
    return NOR_TABLE[a][b];
}

LogicValue logic_xnor(LogicValue a, LogicValue b)
{
    return XNOR_TABLE[a][b];
}
