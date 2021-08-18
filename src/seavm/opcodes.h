#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "stack.h"

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
#define ADD         15
#define SUB         16
#define MUL         17
#define DIV         18
/* 19-26 reserved for future use */
#define CALL        27
#define RET         28
#define CALLC       29
#define STORE       30
#define LOAD        31
#define LBL         32
#define LBLJMP      33
#define LBLJMPIF    34
#define IPRINT      35


#endif // SEAVM_OPCODES_H
