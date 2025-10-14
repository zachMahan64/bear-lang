// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef BVM_H
#define BVM_H

typedef enum {
    // TODO needs a lot of refinement -> have a 32/64 bit bitflag in the opcode or just have
    // dedicated opcodes for a 1:1 jump table?
    // TODO -> FINALIZE PURE STACK MACHINE
    // INT/LONG OPS (32/64 BIT)
    ADD,
    SUB,
    MULT,
    DIV,
    MOD,
    UMULT,
    UDIV,
    UMOD,

    // BITWISE OPS (32/64 BIT)
    OR,
    AND,
    XOR,
    NOT,
    LSH,
    ARSH,
    LRSH,

    // FLOAT OPS (32/64 BIT)
    FADD,
    FSUB,
    FMUL,
    FDIV,
    FMOD,

    // CONVERSIONS
    I2F,
    F2I,

    // MEMORY OPS
    PUSHI,
    LOAD,
    STORE,
    DUP,
    ALLOC,
    FREE,

    // BUILT-IN
    COUT, // for now, hook into printf

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
