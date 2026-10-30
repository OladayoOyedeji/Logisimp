/**
 * @file LogicValue.cpp
 * @brief Implements basic operations on four-state logic values.
 * @author Cole Schwandt
 */

#include "LogicValue.h"

// Preserve UNKNOWN as an undetermined value. A controlling LOW fixes AND and a
// controlling HIGH fixes OR even if the other operand is UNKNOWN or ERROR.
// Otherwise ERROR propagates before UNKNOWN. Indices follow LOW/HIGH/UNKNOWN/ERROR.
static const LogicValue AND_TABLE[4][4] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN   b = ERROR
    /* a = LOW     */ { LOW,      LOW,        LOW,          LOW   },
    /* a = HIGH    */ { LOW,      HIGH,       UNKNOWN,      ERROR },
    /* a = UNKNOWN */ { LOW,      UNKNOWN,    UNKNOWN,      ERROR },
    /* a = ERROR   */ { LOW,      ERROR,      ERROR,        ERROR }
};

static const LogicValue OR_TABLE[4][4] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN   b = ERROR
    /* a = LOW     */ { LOW,      HIGH,       UNKNOWN,      ERROR },
    /* a = HIGH    */ { HIGH,     HIGH,       HIGH,         HIGH  },
    /* a = UNKNOWN */ { UNKNOWN,  HIGH,       UNKNOWN,      ERROR },
    /* a = ERROR   */ { ERROR,    HIGH,       ERROR,        ERROR }
};

static const LogicValue NOT_TABLE[4] =
{
    //                 input
    /* LOW     */      HIGH,
    /* HIGH    */      LOW,
    /* UNKNOWN */      UNKNOWN,
    /* ERROR   */      ERROR
};

static const LogicValue XOR_TABLE[4][4] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN   b = ERROR
    /* a = LOW     */ { LOW,      HIGH,       UNKNOWN,      ERROR },
    /* a = HIGH    */ { HIGH,     LOW,        UNKNOWN,      ERROR },
    /* a = UNKNOWN */ { UNKNOWN,  UNKNOWN,    UNKNOWN,      ERROR },
    /* a = ERROR   */ { ERROR,    ERROR,      ERROR,        ERROR }
};

static const LogicValue NAND_TABLE[4][4] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN   b = ERROR
    /* a = LOW     */ { HIGH,     HIGH,       HIGH,         HIGH  },
    /* a = HIGH    */ { HIGH,     LOW,        UNKNOWN,      ERROR },
    /* a = UNKNOWN */ { HIGH,     UNKNOWN,    UNKNOWN,      ERROR },
    /* a = ERROR   */ { HIGH,     ERROR,      ERROR,        ERROR }
};

static const LogicValue NOR_TABLE[4][4] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN   b = ERROR
    /* a = LOW     */ { HIGH,     LOW,        UNKNOWN,      ERROR },
    /* a = HIGH    */ { LOW,      LOW,        LOW,          LOW   },
    /* a = UNKNOWN */ { UNKNOWN,  LOW,        UNKNOWN,      ERROR },
    /* a = ERROR   */ { ERROR,    LOW,        ERROR,        ERROR }
};

static const LogicValue XNOR_TABLE[4][4] =
{
    //                  b = LOW   b = HIGH   b = UNKNOWN   b = ERROR
    /* a = LOW     */ { HIGH,     LOW,        UNKNOWN,      ERROR },
    /* a = HIGH    */ { LOW,      HIGH,       UNKNOWN,      ERROR },
    /* a = UNKNOWN */ { UNKNOWN,  UNKNOWN,    UNKNOWN,      ERROR },
    /* a = ERROR   */ { ERROR,    ERROR,      ERROR,        ERROR }
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
