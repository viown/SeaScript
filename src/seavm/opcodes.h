#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "./stack.h"

typedef unsigned char Opcode;

#define EXIT        0
#define LOADBOOL    1
#define ICONST      2
#define LCONST      3
#define DCONST      4
#define CAST        5
#define POP         6
#define INC         7
#define DEC         8
#define EQ          9
#define LT          10
#define GT          11
#define JUMP        12
#define JUMPIF      13
#define NOT         14
#define IADD        15
#define ISUB        16
#define IMUL        17
#define IDIV        18
#define CALL        19
#define RET         20
#define CALLC       21
#define IPRINT      22


#endif // SEAVM_OPCODES_H
