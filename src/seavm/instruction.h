#ifndef SEAVM_INSTRUCTION_H
#define SEAVM_INSTRUCTION_H
#include "./opcodes.h"

#define MAX_ARGS 3

typedef struct {
    Opcode op;
    int args[MAX_ARGS];
} Instruction;

#endif // SEAVM_INSTRUCTION_H
