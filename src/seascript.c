#include <stdio.h>
#include "./seavm/vm.h"

#define LEN(x) (sizeof(x) / sizeof(x[0]))

#define READ

#ifdef READ
int main() {
	const char* path = "C:\\Users\\User\\Desktop\\Projects\\SeaScript\\int.ssb";
	Bytecode bytecode;
	Vm vm;
	vm_init(&vm, 100);
	read_from_file(&bytecode, path);
	return vm_execute(&vm, &bytecode);
}
#else
int main() {
	Instruction instructions[] = {
		{
			LOADCONST, {500}
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
	save_to_file(&bytecode, "int.ssb");

	return vm_execute(&vm, &bytecode);
}
#endif


