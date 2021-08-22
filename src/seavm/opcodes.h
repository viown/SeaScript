#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "stack.h"

typedef unsigned char Opcode;

#define NOP         0
#define EXIT        1
#define LOADBOOL    2
#define LOADC       5
#define POP         7
#define INC         8
#define DEC         9
#define EQ          10
#define LT          11
#define GT          12
#define JUMP        13
#define JUMPIF      14
#define NOT         15
#define ADD         16
#define SUB         17
#define MUL         18
#define DIV         19
/* 20-26 reserved for future use */
#define CALL        27
#define RET         28
#define CALLC       29
#define STORE       30
#define LOAD        31
#define LBL         32
#define LBLJMP      33
#define LBLJMPIF    34


#endif // SEAVM_OPCODES_H
