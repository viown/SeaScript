#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>


void vm_init(VirtualMachine* vm, const ss_BaseFunction* func_list) {
    for (int i = 0; i < 256; ++i) {
        vm->stack[i].init = false;
    }
    vm->stack[0] = create_stack();
    vm->ip = 0;
    vm->c_functions = func_list;
    vm->globals = (StackObject*)ss_malloc(50 * sizeof(StackObject));
    vm->global_size = 50;
    vm->global_used = 0;
    vm->locals[0] = (StackObject*)ss_malloc(50 * sizeof(StackObject));
    vm->local_size[0] = 50;
    vm->local_used[0] = 50;
    vm->sp = 0;
    vm->label_addresses = (int*)ss_malloc(500 * sizeof(int));
    vm->label_addr_size = 500;
    vm->label_addr_used = 0;
    vm->heap_table = (void**)ss_malloc(50 * sizeof(void*));
    vm->heap_table_size = 50;
    vm->heap_table_used = 0;
}

void push_global(VirtualMachine* vm, StackObject object) {
    if (vm->global_used == vm->global_size) {
        vm->global_size *= 2;
        vm->globals = (StackObject*)realloc(vm->globals, vm->global_size * sizeof(StackObject));
    }
    vm->globals[vm->global_used++] = object;
}

void set_global(VirtualMachine* vm, StackObject object, long long int id) {
    if (id >= vm->global_size) {
        vm->global_size *= 2;
        vm->globals = (StackObject*)realloc(vm->globals, vm->global_size * sizeof(StackObject));
    }
    vm->globals[id] = object;
}

void push_heap_object(VirtualMachine* vm, void* mem_block) {
    if (vm->heap_table_used == vm->heap_table_size) {
        vm->heap_table_size += 50;
        vm->heap_table = (void**)realloc(vm->heap_table, vm->heap_table_size * sizeof(void*));
    }
    vm->heap_table[vm->heap_table_used++] = mem_block;
}

/* Reset the virtual machine's memory state */
void vm_clear(VirtualMachine* vm) {
    vm->ip = 0;
    vm->global_used = 0;
    for (size_t i = 0; i < vm->heap_table_used; ++i) {
        free_and_null(vm->heap_table[i]);
    }
    vm->heap_table_used = 0;
    vm->label_addr_used = 0;
    vm->stack[vm->sp].allocated = 0;
}


void vm_free(VirtualMachine* vm) {
    free_and_null(vm->globals);
    free_and_null(vm->label_addresses);
    for (size_t i = 0; i < vm->heap_table_used; ++i) {
        free_and_null(vm->heap_table[i]);
    }
    free_and_null(vm->heap_table);
    terminate_stack(&vm->stack[0]);
}

void resolve_labels(VirtualMachine* vm, Instruction* instrs, uint64_t length) {
    for (uint64_t i = 0; i < length; ++i) {
        if (instrs[i].op == LBL) {
            vm->label_addresses[(long long int)instrs[i].args[0]] = i;
        }
    }
}

void vm_raise(VirtualMachine* vm, unsigned char exit_code) {
    // TODO: Use an environment exit instead of terminating the program
    vm_free(vm);
    exit(exit_code);
}

int vm_execute(VirtualMachine* vm, StringPool* pool, Instruction* instrs, size_t length) {
    vm->ip = 0; // Reset ip
    Instruction* cinstr;

    StackObject* top;
    StackObject* top2;
    StackObject object;

    // Arithmetic operands
    StackObject a;
    StackObject b;

    resolve_labels(vm, instrs, length);
    while (vm->ip != length) {
        cinstr = &instrs[vm->ip];

        switch (cinstr->op) {
        case NOP:
            vm->ip++;
            break;
        case EXIT:
            vm->ip = 0;
            return cinstr->args[0];
        case LOADBOOL:
            push_stack(&vm->stack[vm->sp], create_bool((bool)cinstr->args[0]));
            vm->ip++;
            break;
        case LOADC: // load constant
            object.object.m_number = cinstr->args[0];
            object.type = NUMBER;
            push_stack(&vm->stack[vm->sp], object);
            vm->ip++;
            break;
        case LOADPOOL:
            object.object.m_string = pool->constants[cinstr->args[0]];
            object.type = STRING;
            push_stack(&vm->stack[vm->sp], object);
            vm->ip++;
            break;
        case POP:
            pop_stack(&vm->stack[vm->sp]);
            vm->ip++;
            break;
        case INC:
            top = top_stack(&vm->stack[vm->sp]);
            top->object.m_number++;
            vm->ip++;
            break;
        case DEC:
            top = top_stack(&vm->stack[vm->sp]);
            top->object.m_number--;
            vm->ip++;
            break;
        case EQ:
            top = top_stack(&vm->stack[vm->sp]);
            top2 = top - 1;
            if (top->type == STRING && top2->type == STRING) {
                push_stack(&vm->stack[vm->sp], create_bool( !strcmp(top->object.m_string, top2->object.m_string) ));
            } else if (top->type == STRING || top2->type == STRING) {
                push_stack(&vm->stack[vm->sp], create_bool(false));
            } else {
                push_stack(&vm->stack[vm->sp], create_bool( (top-1)->object.m_number == top->object.m_number) );
            }
            vm->ip++;
            break;
        case LT:
            top = top_stack(&vm->stack[vm->sp]);
            push_stack(&vm->stack[vm->sp], create_bool((top-1)->object.m_number < top->object.m_number));
            vm->ip++;
            break;
        case GT:
            top = top_stack(&vm->stack[vm->sp]);
            push_stack(&vm->stack[vm->sp], create_bool((top-1)->object.m_number > top->object.m_number));
            vm->ip++;
            break;
        case JUMP:
            vm->ip = cinstr->args[0];
            break;
        case JUMPIF:
            top = top_stack(&vm->stack[vm->sp]);
            if (top->object.m_bool) {
                vm->ip = cinstr->args[0];
            } else {
                vm->ip++;
            }
            break;
        case NOT:
            top = top_stack(&vm->stack[vm->sp]);
            if (top->object.m_bool) {
                push_stack(&vm->stack[vm->sp], create_bool(0));
            } else {
                push_stack(&vm->stack[vm->sp], create_bool(1));
            }
            vm->ip++;
            break;
        case ADD:
            b = pop_stack(&vm->stack[vm->sp]);
            a = pop_stack(&vm->stack[vm->sp]);
            object.object.m_number = a.object.m_number + b.object.m_number;
            object.type = NUMBER;
            push_stack(&vm->stack[vm->sp], object);
            vm->ip++;
            break;
        case SUB:
            b = pop_stack(&vm->stack[vm->sp]);
            a = pop_stack(&vm->stack[vm->sp]);
            object.object.m_number = a.object.m_number - b.object.m_number;
            object.type = NUMBER;
            push_stack(&vm->stack[vm->sp], object);
            vm->ip++;
            break;
        case MUL:
            b = pop_stack(&vm->stack[vm->sp]);
            a = pop_stack(&vm->stack[vm->sp]);
            object.object.m_number = a.object.m_number * b.object.m_number;
            object.type = NUMBER;
            push_stack(&vm->stack[vm->sp], object);
            vm->ip++;
            break;
        case DIV:
            b = pop_stack(&vm->stack[vm->sp]);
            a = pop_stack(&vm->stack[vm->sp]);
            object.object.m_number = a.object.m_number / b.object.m_number;
            object.type = NUMBER;
            push_stack(&vm->stack[vm->sp], object);
            vm->ip++;
            break;
        case CALL:
            vm->return_addresses[vm->sp++] = vm->ip;
            vm->ip = cinstr->args[0];
            vm->stack[vm->sp] = create_stack();
            vm->locals[vm->sp] = (StackObject*)ss_malloc(50 * sizeof(StackObject));
            vm->local_size[vm->sp] = 50;
            vm->local_used[vm->sp] = 0;
            break;
        case LBLCALL:
            vm->return_addresses[vm->sp++] = vm->ip;
            vm->ip = vm->label_addresses[cinstr->args[0]];
            vm->stack[vm->sp] = create_stack();
            vm->locals[vm->sp] = (StackObject*)ss_malloc(50 * sizeof(StackObject));
            vm->local_size[vm->sp] = 50;
            vm->local_used[vm->sp] = 0;
            break;
        case RET:
            terminate_stack(&vm->stack[vm->sp]);
            free_and_null(vm->locals[vm->sp]);
            vm->ip = vm->return_addresses[--vm->sp] + 1;
            break;
        case CALLC:
            vm->c_functions[cinstr->args[0]].func(vm);
            vm->ip++;
            break;
        case STORE:
            set_global(vm, pop_stack(&vm->stack[vm->sp]), cinstr->args[0]);
            vm->ip++;
            break;
        case LOAD:
            push_stack(&vm->stack[vm->sp], vm->globals[cinstr->args[0]]);
            vm->ip++;
            break;
        case LBL:
            /* Labels are handled by the address resolver */
            vm->ip++;
            break;
        case LBLJMP:
            vm->ip = vm->label_addresses[cinstr->args[0]];
            break;
        case LBLJMPIF:
            top = top_stack(&vm->stack[vm->sp]);
            if (top->object.m_bool)
                vm->ip = vm->label_addresses[cinstr->args[0]];
            else
                vm->ip++;
            break;
        case LSTORE:
            if (cinstr->args[0] >= vm->local_size[vm->sp]) {
                vm->local_size[vm->sp] *= 2;
                vm->locals[vm->sp] = (StackObject*)realloc(vm->locals[vm->sp], vm->local_size[vm->sp] * sizeof(StackObject));
            }
            vm->locals[vm->sp][cinstr->args[0]] = pop_stack(&vm->stack[vm->sp]);
            vm->ip++;
            break;
        case LLOAD:
            push_stack(&vm->stack[vm->sp], vm->locals[vm->sp][cinstr->args[0]]);
            vm->ip++;
            break;
        default:
            return VM_INVALID_INSTRUCTION;
        }
    }
    return VM_EXIT_FAILURE; // Should exit properly through (EXIT 0)
}

const char* instruction_to_string(Opcode op) {
    switch (op) {
    case NOP:
        return "NOP";
    case EXIT:
        return "EXIT";
    case LOADBOOL:
        return "LOADBOOL";
    case LOADC:
        return "LOADC";
    case LOADPOOL:
        return "LOADPOOL";
    case POP:
        return "POP";
    case INC:
        return "INC";
    case DEC:
        return "DEC";
    case EQ:
        return "EQ";
    case LT:
        return "LT";
    case GT:
        return "GT";
    case JUMP:
        return "JUMP";
    case JUMPIF:
        return "JUMPIF";
    case NOT:
        return "NOT";
    case ADD:
        return "ADD";
    case SUB:
        return "SUB";
    case MUL:
        return "MUL";
    case DIV:
        return "DIV";
    case CALL:
        return "CALL";
    case RET:
        return "RET";
    case CALLC:
        return "CALLC";
    case STORE:
        return "STORE";
    case LOAD:
        return "LOAD";
    case LBL:
        return "LBL";
    case LBLJMP:
        return "LBLJMP";
    case LBLJMPIF:
        return "LBLJMPIF";
    case LBLCALL:
        return "LBLCALL";
    case LSTORE:
        return "LSTORE";
    case LLOAD:
        return "LLOAD";
    default:
        return "UNKNOWN";
    }
    return "";
}
