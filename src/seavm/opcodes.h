#ifndef SEAVM_OPCODES_H
#define SEAVM_OPCODES_H
#include "stack.h"

/*
*   Reserved opcodes:
*   1. 177 - Used by the bytecode serializer to store constants
*/

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
#define NEQ         11
#define JUMP        12
#define JUMPIF      13
#define NOT         14
#define ADD         15
#define SUB         16
#define MUL         17
#define DIV         18
#define CALL        19
#define RET         20
#define CALLC       21
#define STORE       22
#define LOAD        23
#define LBL         24
#define LBLJMP      25
#define LBLJMPIF    26


#endif // SEAVM_OPCODES_H
