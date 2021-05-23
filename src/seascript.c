#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./lex.h"
#include "./seavm/bytecode.h"
#include "./seavm/vm.h"


void visualize_token(Token* token) {
	printf("<SPECIFIER='%d', VALUE='%s'>\n", token->token,token->value);
}

char* read_file(const char* path) {
	FILE* file = fopen(path, "r");
	char* buffer = malloc(get_file_size(file) + 1);
	int new_index = 0;
	int current_char = getc(file);
	while (current_char != EOF) {
		buffer[new_index++] = current_char;
		current_char = getc(file);
	}
	buffer[new_index++] = '\0';
	return buffer;
}

int main() {
	Instruction instructions[] = {
		{
			LOADCONST, {100}
		},
		{
			CALL, {4},
		},
		{
			IPRINT, {}
		},
		{
			EXIT, {0}
		},
		// FUNCTION: return 10
		{
			STORE, {1} // Store return address
		},
		{
			LOADCONST, {10}
		},
		{
			RETURN, {1}
		}

	};
	Bytecode bytecode;
	struct Vm vm;

	vm_init(&vm, 100);
	to_bytecode(&bytecode, instructions, LEN(instructions));
	save_to_file(&bytecode, "test.ssb");

	return vm_execute(&vm, &bytecode);
}

