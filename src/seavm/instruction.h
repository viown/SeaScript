#ifndef SEAVM_INSTRUCTION_H
#define SEAVM_INSTRUCTION_H
#include "./stack.h" // for word
#include "./opcodes.h"
#include <stdint.h>

#define MAX_ARGS 3


struct Instruction {
	Opcode op;
	word args[MAX_ARGS];
};

typedef struct Instruction Instruction;

#endif
