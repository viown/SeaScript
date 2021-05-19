#include "./vm.h"
#include <stdlib.h>
#include <stdio.h>

void vm_init(Vm* vm, int global_size) {
	vm->stack = create_stack();
	vm->ip = 0;
	vm->global_size = global_size;
	vm->global_used = 0;
	vm->globals = calloc(global_size, sizeof(int));
}

void vm_free(Vm* vm) {
	free(vm->globals);
}

bool vm_execute(Vm* vm, Bytecode* bytecode)
{
	Instruction* instrs;
	Instruction cinstr;
	instrs = to_instructions(bytecode);

	stack_type* top;
	int f = 0;
	int s = 0;

	while (vm->ip != bytecode->length)
	{
		cinstr = instrs[vm->ip];

		switch (cinstr.op)
		{
		case EXIT:
			free_bytecode(bytecode);
			free(instrs);
			return !(cinstr.args[0] == EXIT_SUCCESS);
		case LOADCONST:
			push_stack(&vm->stack, cinstr.args[0]);
			vm->ip++;
			break;
		case POP:
			pop_stack(&vm->stack);
			vm->ip++;
		case STORE:
			vm->globals[cinstr.args[0]] = *top_stack(&vm->stack);
			vm->ip++;
			break;
		case LOAD:
			push_stack(&vm->stack, vm->globals[cinstr.args[0]]);
			vm->ip++;
			break;
		case ADD:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			push_stack(&vm->stack, f + s);
			vm->ip++;
			break;
		case SUB:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			push_stack(&vm->stack, f - s);
			vm->ip++;
			break;
		case MUL:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			push_stack(&vm->stack, f * s);
			vm->ip++;
			break;
		case DIV:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			push_stack(&vm->stack, f / s);
			vm->ip++;
			break;
		case EQ:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			if (f == s)
				vm->ip = cinstr.args[0];
			else
				vm->ip++;
			break;
		case LT:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			if (f < s)
				vm->ip = cinstr.args[0];
			else
				vm->ip++;
			break;
		case GT:
			top = top_stack(&vm->stack);
			f = *(top--);
			s = *(top--);
			if (f > s)
				vm->ip = cinstr.args[0];
			else
				vm->ip++;
			break;
		case JUMP:
			vm->ip = cinstr.args[0];
			break;
		case CALL:
			//TODO
			break;
		case RETURN:
			//TODO
			break;
		case IPRINT:
			printf("%d", *top_stack(&vm->stack));
			vm->ip++;
			break;
		case CPRINT:
			printf("%c", (char)*top_stack(&vm->stack));
			vm->ip++;
			break;
		default:
			return EXIT_FAILURE;
		}
	}

	free_bytecode(bytecode);
	free(instrs);
	return EXIT_FAILURE; // Should exit properly through (EXIT 0)
}
