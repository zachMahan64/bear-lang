#ifndef BVM_H
#define BVM_H

typedef enum {
    // 32-BIT INT OPS
    ADD_I,
    SUB_I,
    MULT_I,
    DIV_I,
    MOD_I,
    UMULT_I,
    UDIV_I,
    UMOD_I,

    // 64-BIT LONG OPS
    ADD_L,
    SUB_L,
    MULT_L,
    DIV_L,
    MOD_L,
    UMULT_L,
    UDIV_L,
    UMOD_L,

    // BITWISE OPS (64 BIT)
    OR,
    AND,
    XOR,
    NOT,
    LSH,
    ARSH,
    LRSH,

    // FLOAT OPS
    ADD_F,
    SUB_F,
    MULT_F,
    DIV_F,

    // DOUBLE OPS
    ADD_D,
    SUB_D,
    MULT_D,
    DIV_D,

    // MEMORY OPS
    MOV,
    PUSH,
    POP,

    // CTRL FULL OPS
    JMP,
    CALL,
    RET,
    BLT,
    BGT,
    BLE,
    BGE,
    BEQ,
    BNE,

} bvm_opcode;

#endif
