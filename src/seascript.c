#include <stdio.h>
#include "./lex.h"
#include "./seavm/vm.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

int main() {
	Instruction instructions[] = {
		{
			LOADCONST, {5}
		},
		{
			LOADCONST, {6}
		},
		{
			DIV, {}
		},
		{
			IPRINT, {}
		},
		{
			EXIT, {0}
		}
	};

	Bytecode bytecode;
	struct Vm vm;

	vm_init(&vm, 100);
	to_bytecode(&bytecode, instructions, LEN(instructions));
	save_to_file(&bytecode, "int.ssb");

	return vm_execute(&vm, &bytecode);
	return 0;
}



