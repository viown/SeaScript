#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "./stack.h"

enum Opcode {
    EXIT,
    LOADCONST,
    POP,
    STORE, // TODO: local memory
    LOAD,
    ADD,
    SUB,
    MUL,
    DIV,
    EQ,
    LT,
    GT,
    NEWARRAY,
    JUMP,
    CALL,
    RETURN,
    CALLC, // Call C function
    IPRINT, // Print int
    CPRINT, // Print char
};

typedef enum Opcode Opcode;
#endif // SEAVM_OPCODES_H
