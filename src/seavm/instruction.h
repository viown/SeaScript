#ifndef SEAVM_INSTRUCTION_H
#define SEAVM_INSTRUCTION_H
#include "./opcodes.h"

#define MAX_ARGS 3

struct Instruction {
	Opcode op;
	int args[MAX_ARGS];
};

typedef struct Instruction Instruction;

#endif // SEAVM_INSTRUCTION_H
