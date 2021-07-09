#include "./vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

void vm_init(Vm* vm, int global_size, const ss_BaseFunction* func_list) {
    vm->stack = create_stack();
    vm->ip = 0;
    vm->c_functions = func_list;
    vm->global_size = global_size;
    vm->global_used = 0;
    vm->globals = calloc(global_size, sizeof(int));
}

void vm_free(Vm* vm) {
    free(vm->globals);
}

bool vm_execute(Vm* vm, Instruction* instrs, size_t length) {
    Instruction cinstr;

    stack_type* top;
    stack_type f = 0;
    stack_type s = 0;

    while (vm->ip != length) {
        cinstr = instrs[vm->ip];

        switch (cinstr.op) {
        case EXIT:
            vm_free(vm);
            return !(cinstr.args[0] == EXIT_SUCCESS);
        case ICONST:
            push_stack(&vm->stack, cinstr.args[0]);
            vm->ip++;
            break;
        case POP:
            pop_stack(&vm->stack);
            vm->ip++;
        case STORE:
            vm->globals[cinstr.args[0]] = *top_stack(&vm->stack);
            vm->global_used++;
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
            vm->ip = (f == s) ? cinstr.args[0] : vm->ip + 1;
            break;
        case LT:
            top = top_stack(&vm->stack);
            f = *(top--);
            s = *(top--);
            vm->ip = (f < s) ? cinstr.args[0] : vm->ip + 1;
            break;
        case GT:
            top = top_stack(&vm->stack);
            f = *(top--);
            s = *(top--);
            vm->ip = (f > s) ? cinstr.args[0] : vm->ip + 1;
            break;
        case JUMP:
            vm->ip = cinstr.args[0];
            break;
        case CALL:
            push_stack(&vm->stack, vm->ip); // Push return address
            vm->ip = cinstr.args[0]; // Jump to function
            break;
        case RETURN:
            vm->ip = vm->globals[cinstr.args[0]] + 1; // Return to call function.
            break;
        case CALLC:
            vm->c_functions[cinstr.args[0]].func(vm);
            vm->ip++;
            break;
        case IPRINT:
            printf("%f", *top_stack(&vm->stack));
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
    vm_free(vm);
    return EXIT_FAILURE; // Should exit properly through (EXIT 0)
}

const char* instruction_to_string(Opcode op) {
    switch (op) {
    case EXIT:
        return "EXIT";
    case ICONST:
        return "ICONST";
    case POP:
        return "POP";
    case STORE:
        return "STORE";
    case LOAD:
        return "LOAD";
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case DIV:
        return "DIV";
    case EQ:
        return "EQ";
    case LT:
        return "LT";
    case GT:
        return "GT";
    case JUMP:
        return "JUMP";
    case CALL:
        return "CALL";
    case RETURN:
        return "RETURN";
    case CALLC:
        return "CALLC";
    case IPRINT:
        return "IPRINT";
    case CPRINT:
        return "CPRINT";
    default:
        return "UNKNOWN";
    }
}
