#include <stdio.h>
#include "./seavm/vm.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))



int main() {
	Instruction instructions[] = {
		{
			LOADCONST, {100} // Pushes 100 onto the stack
		},
		{
			STORE, {0} // Stores `100` as a global in address 0
		},
		{
			LOADCONST, {35} // blocker
		},
		{
			LOAD, {0} // Loads address 0 onto the stack
		},
		{
			IPRINT, {} // 100
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
}



