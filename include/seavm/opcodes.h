#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "stack.h"

typedef unsigned char Opcode;

#define NOP         0
#define EXIT        1
#define LOADBOOL    2
#define LOADC       3
#define LOADPOOL    4
#define POP         5
#define INC         6
#define DEC         7
#define EQ          8
#define LT          9
#define GT          10
#define JUMP        11
#define JUMPIF      12
#define NOT         13
#define ADD         14
#define SUB         15
#define MUL         16
#define DIV         17
#define CALL        18
#define RET         19
#define CALLC       20
#define STORE       21
#define LOAD        22
#define LBL         23
#define LBLJMP      24
#define LBLJMPIF    25
#define LBLCALL     26
#define LSTORE      27
#define LLOAD       28


#endif // SEAVM_OPCODES_H
