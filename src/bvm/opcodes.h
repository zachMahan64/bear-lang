// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef BVM_H
#define BVM_H

typedef enum {
    // TODO finish this section
    // INT/LONG OPS (32/64 BIT)
    ADDI,
    SUBI,
    MULTI,
    DIVI,
    MODI,
    UMULTI,
    UDIVI,
    UMODI,

    ADDL,
    SUBL,
    MULTL,
    DIVL,
    MODL,
    UMULTL,
    UDIVL,
    UMODL,

    // BITWISE OPS (32/64 BIT)
    ORI,
    ANDI,
    XORI,
    NOTI,
    LSHI,
    ARSHI,
    LRSHI,

    ORL,
    ANDL,
    XORL,
    NOTL,
    LSHL,
    ARSHL,
    LRSHL,

    // FLOAT OPS (32/64 BIT)
    ADDF,
    SUBF,
    MULF,
    DIVF,
    MODF,

    ADDD,
    SUBD,
    MULD,
    DIVD,
    MODD,

    // CONVERSIONS
    // same width signed <-> unsigned are just bitcasts, so no dedicated op
    // int -> _
    I2L,
    I2F,
    I2D,
    // uint -> _
    UI2L,
    UI2F,
    UI2D,

    // long -> _
    L2I,
    L2F,
    L2D,
    // ulong -> _
    UL2I,
    UL2F,
    UL2D,

    // float -> _
    F2I,
    F2L,
    F2D,

    F2UI,
    F2UL,
    // doub -> _
    D2I,
    D2L,
    D2F,

    D2UI,
    D2UL,
    // char convs
    I2C,
    C2I,

    // MEMORY OPS
    PUSHI,
    PUSHL,
    LOAD,
    STORE,
    DUP,
    ALLOC,
    FREE,

    // BUILT-IN, TODO: figure out the practical mechanics of this
    // hook into printf directly?
    COUT_STR,
    COUT_INT,
    COUT_FLT,

    // comparisons (push bool result)
    CMP_EQ, // push (lhs == rhs)
    CMP_NE, // push (lhs != rhs)
    CMP_LT, // push (lhs < rhs)
    CMP_LE, // push (lhs <= rhs)
    CMP_GT, // push (lhs > rhs)
    CMP_GE, // push (lhs >= rhs)

    // conditional jumps
    JMP,
    JMP_IF_TRUE,
    JMP_IF_FALSE,

    CALL,
    RET,

    NOP,
    HALT,

} bvm_opcode;

#endif
