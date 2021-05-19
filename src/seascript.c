#include <stdio.h>
#include "./seavm/vm.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

/*
int main() {
	const char* path = "C:\\Users\\User\\Desktop\\Projects\\SeaScript\\byte.ssb";
	Bytecode bytecode;
	Vm vm;
	vm_init(&vm, 100);
	read_from_file(&bytecode, path);
	return vm_execute(&vm, &bytecode);
}*/


int main() {
	Instruction instructions[] = {
		{
			LOADCONST, {2}
		},
		{
			LOADCONST, {10}
		},
		{
			MUL, {}
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
	save_to_file(&bytecode, "test.ssb");

	return vm_execute(&vm, &bytecode);
}
