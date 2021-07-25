#ifndef SEAVM_INSTRUCTION_H
#define SEAVM_INSTRUCTION_H
#include <stdlib.h>
#include "../debug.h"
#include "./opcodes.h"

#define MAX_ARGS 1

typedef long long int arg_type;

typedef struct {
    Opcode op;
    arg_type args[MAX_ARGS];
} Instruction;

typedef struct {
    Instruction* instructions;
    size_t size;
    size_t length;
} InstructionHolder;


#endif // SEAVM_INSTRUCTION_H
