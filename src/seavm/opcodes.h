#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "./stack.h"

typedef unsigned char Opcode;

#define EXIT        0x0
#define LOADCONST   0x1
#define POP         0x2
#define STORE       0x3
#define LOAD        0x4
#define ADD         0x5
#define SUB         0x6
#define MUL         0x7
#define DIV         0x8
#define EQ          0x9
#define LT          0xa
#define GT          0xb
#define NEWARRAY    0xc
#define JUMP        0xd
#define CALL        0xe
#define RETURN      0xf
#define CALLC       0x10
#define IPRINT      0x11
#define CPRINT      0x12


#endif // SEAVM_OPCODES_H
