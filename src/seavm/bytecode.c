#include "./bytecode.h"
#include <stdio.h>
#include <stdlib.h>

/*
	TODO:
	- Allow storage of data types larger than byte (unsigned char)
*/

void to_bytecode(Bytecode* bytecode, Instruction* instructions, size_t length) {
	bytecode->length = length;
	STACK_TYPE* buffer = malloc((length * 4) * sizeof(STACK_TYPE));
	int cursor = 0;
	for (size_t i = 0; i < length; ++i) {
		Instruction current_instruction = instructions[i];
		buffer[cursor] = current_instruction.op;
		buffer[++cursor] = current_instruction.args[0];
		buffer[++cursor] = current_instruction.args[1];
		buffer[++cursor] = current_instruction.args[2];
		cursor++;
	}
	bytecode->raw_data = buffer;
}

Instruction* to_instructions(Bytecode* bytecode) {
	Instruction* instructions = malloc(sizeof(Instruction) * bytecode->length);
	int cursor = 0;
	for (size_t i = 0; i < bytecode->length; ++i) {
		Instruction instruction;
		instruction.op = bytecode->raw_data[cursor];
		instruction.args[0] = bytecode->raw_data[++cursor];
		instruction.args[1] = bytecode->raw_data[++cursor];
		instruction.args[2] = bytecode->raw_data[++cursor];
		cursor++;
		instructions[i] = instruction;
	}
	return instructions;
}

void save_to_file(Bytecode* bytecode, const char* path) {
	FILE* file = fopen(path, "wb");
	int cursor = 0;
	for (size_t i = 0; i < bytecode->length; ++i) {
		STACK_TYPE buffer[4 /* * sizeof(STACK_TYPE) */] = {bytecode->raw_data[cursor], bytecode->raw_data[++cursor], bytecode->raw_data[++cursor], bytecode->raw_data[++cursor]};
		cursor++;
		fwrite(buffer, sizeof(buffer), 1, file);
	}
	fclose(file);
}

void read_from_file(Bytecode* bytecode, const char* path) {
	FILE* file = fopen(path, "rb");
	size_t buffer_size = 4 /* * sizeof(STACK_TYPE)*/;
	size_t allocated_size = buffer_size;
	size_t cursor = 0;
	STACK_TYPE* raw_data = malloc(allocated_size);
	int current_byte = getc(file);
	while (current_byte != EOF) {
		if (cursor == allocated_size) {
			allocated_size += buffer_size;
			raw_data = realloc(raw_data, allocated_size);
		}
		raw_data[cursor++] = current_byte;
		current_byte = getc(file);
	}
	fclose(file);
	bytecode->length = allocated_size / 4;
	bytecode->raw_data = raw_data;
}

void free_bytecode(Bytecode* bytecode) {
	free(bytecode->raw_data);
}
