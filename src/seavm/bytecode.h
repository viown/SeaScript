#ifndef SEAVM_BYTECODE_H
#define SEAVM_BYTECODE_H
#include <stddef.h>
#include "./instruction.h"
#include "./stack.h"


struct Bytecode {
	STACK_TYPE* raw_data;
	size_t length;
};

typedef struct Bytecode Bytecode;

void to_bytecode(Bytecode* bytecode, Instruction* instructions, size_t length);
Instruction* to_instructions(Bytecode* bytecode);

void save_to_file(Bytecode* bytecode, const char* path);
void read_from_file(Bytecode* bytecode, const char* path);

void free_bytecode(Bytecode* bytecode);

#endif // SEAVM_BYTECODE_H
