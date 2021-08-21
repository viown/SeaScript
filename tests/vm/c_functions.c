#include "./seavm/vm.h"
#include "./seavm/bytecode.h"

void test_print(Stack* stack) {
    printf("Hello, world!\n");
}

void test_add(Stack* stack) {
    stack_type* top = top_stack(stack);
    push_stack(stack, *top + 50); /* add 50 */
}

int main() {
	Instruction instructions[] = {
        {
            LOADCONST, {50} /* load 50 */
        },
		{
            CALLC, {1} /* test_add */
		},
		{
            IPRINT, {} /* 100 */
		},
		{
			EXIT, {0}
		}
	};

	Bytecode bytecode;
	Vm vm;

	vm_init(&vm, 100);
	vm_register_function(&vm, &test_print);
	vm_register_function(&vm, &test_add);
	to_bytecode(&bytecode, instructions, LEN(instructions));
	save_to_file(&bytecode, "test.ssb");

	return vm_execute(&vm, &bytecode);
}