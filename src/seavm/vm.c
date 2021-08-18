#include "vm.h"
#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define compare_objects(a, op, b)                               \
    switch (highest_type) {                                     \
    case INT32:                                                 \
        perform_cast(a, INT32);                                 \
        perform_cast(b, INT32);                                 \
        return a->object.m_int32 op b->object.m_int32;          \
    case INT64:                                                 \
        perform_cast(a, INT64);                                 \
        perform_cast(b, INT64);                                 \
        return a->object.m_int64 op b->object.m_int64;          \
    case DOUBLE:                                                \
        perform_cast(a, DOUBLE);                                \
        perform_cast(b, DOUBLE);                                \
        return a->object.m_double op b->object.m_double;        \
    default:                                                    \
        return 0;                                               \
    }

#define compute(stack, first, op, second)                                                   \
    StackObject object;                                                                     \
    object.object.m_int64 = (int64_t)(second.object.m_int32 op first.object.m_int32);       \
    object.type = INT64;                                                                    \
    push_stack(&stack, object);


void vm_init(VirtualMachine* vm, const ss_BaseFunction* func_list) {
    vm->stack = create_stack();
    vm->ip = 0;
    vm->c_functions = func_list;
    vm->globals = (StackObject*)ss_malloc(50 * sizeof(StackObject));
    vm->global_size = 50;
    vm->global_used = 0;
    vm->ret_sp = 0;
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
    vm->stack.allocated = 0;
}


void vm_free(VirtualMachine* vm) {
    free_and_null(vm->globals);
    free_and_null(vm->label_addresses);
    for (size_t i = 0; i < vm->heap_table_used; ++i) {
        free_and_null(vm->heap_table[i]);
    }
    free_and_null(vm->heap_table);
    terminate_stack(&vm->stack);
}


void cast_int(StackObject* object, StackObjType type) {
    int32_t num = object->object.m_int32;
    switch (type) {
    case INT64:
        object->object.m_int64 = (int64_t)num;
        object->type = INT64;
        break;
    case DOUBLE:
        object->object.m_double = (double)num;
        object->type = DOUBLE;
        break;
    default:
        return;
    }
}

void cast_long(StackObject* object, StackObjType type) {
    int64_t num = object->object.m_int64;
    switch (type) {
    case INT32:
        object->object.m_int32 = (int32_t)num;
        object->type = INT32;
        break;
    case DOUBLE:
        object->object.m_double = (double)num;
        object->type = DOUBLE;
        break;
    default:
        return;
    }
}

void cast_double(StackObject* object, StackObjType type) {
    double num = object->object.m_double;
    switch (type) {
    case INT32:
        object->object.m_int32 = (int32_t)num;
        object->type = INT32;
        break;
    case INT64:
        object->object.m_int64 = (int64_t)num;
        object->type = INT64;
        break;
    default:
        return;
    }
}


void perform_cast(StackObject* object, StackObjType type) {
    if (object->type == type)
        return;
    switch (object->type) {
    case INT32:
        cast_int(object, type);
        break;
    case INT64:
        cast_long(object, type);
        break;
    case DOUBLE:
        cast_double(object, type);
        break;
    default:
        return;
    }
}

bool is_equal(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    compare_objects(a, ==, b);
}


bool is_lt(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    compare_objects(a, <, b);
}

bool is_gt(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    compare_objects(a, >, b);
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

int vm_execute(VirtualMachine* vm, Instruction* instrs, uint64_t length) {
    vm->ip = 0; // Reset ip
    Instruction* cinstr;

    StackObject* top;
    StackObject object;

    resolve_labels(vm, instrs, length);
    while (vm->ip != length) {
        cinstr = &instrs[vm->ip];

        switch (cinstr->op) {
        case EXIT:
            vm->ip = 0;
            return cinstr->args[0];
        case LOADBOOL:
            push_stack(&vm->stack, create_bool((bool)cinstr->args[0]));
            vm->ip++;
            break;
        case ICONST:
            object.object.m_int32 = cinstr->args[0];
            object.type = INT32;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case LCONST:
            object.object.m_int64 = cinstr->args[0];
            object.type = INT64;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case DCONST:
            object.object.m_double = cinstr->args[0];
            object.type = DOUBLE;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case CAST:
            perform_cast(top_stack(&vm->stack), (StackObjType)cinstr->args[0]);
            vm->ip++;
            break;
        case POP:
            pop_stack(&vm->stack);
            vm->ip++;
            break;
        case INC:
            top = top_stack(&vm->stack);
            switch (top->type) {
            case INT32:
                top->object.m_int32++;
                break;
            case INT64:
                top->object.m_int64++;
                break;
            case DOUBLE:
                top->object.m_double++;
                break;
            default:
                break;
            }
            vm->ip++;
            break;
        case DEC:
            top = top_stack(&vm->stack);
            switch (top->type) {
            case INT32:
                top->object.m_int32--;
                break;
            case INT64:
                top->object.m_int64--;
                break;
            case DOUBLE:
                top->object.m_double--;
                break;
            default:
                break;
            }
            vm->ip++;
            break;
        case EQ:
            top = top_stack(&vm->stack);
            push_stack(&vm->stack, create_bool(is_equal(top-1, top)));
            vm->ip++;
            break;
        case LT:
            top = top_stack(&vm->stack);
            push_stack(&vm->stack, create_bool(is_lt(top-1, top)));
            vm->ip++;
            break;
        case GT:
            top = top_stack(&vm->stack);
            push_stack(&vm->stack, create_bool(is_gt(top-1, top)));
            vm->ip++;
            break;
        case JUMP:
            vm->ip = cinstr->args[0];
            break;
        case JUMPIF:
            top = top_stack(&vm->stack);
            if (top->object.m_bool) {
                vm->ip = cinstr->args[0];
            } else {
                vm->ip++;
            }
            break;
        case NOT:
            top = top_stack(&vm->stack);
            if (top->object.m_bool) {
                push_stack(&vm->stack, create_bool(0));
            } else {
                push_stack(&vm->stack, create_bool(1));
            }
            vm->ip++;
            break;
        case ADD:
            push_stack(&vm->stack, cpu_add(vm));
            vm->ip++;
            break;
        case SUB:
            push_stack(&vm->stack, cpu_sub(vm));
            vm->ip++;
            break;
        case MUL:
            push_stack(&vm->stack, cpu_mul(vm));
            vm->ip++;
            break;
        case DIV:
            push_stack(&vm->stack, cpu_div(vm));
            vm->ip++;
            break;
        case CALL:
            vm->return_addresses[vm->ret_sp++] = vm->ip; /* store return address in stack */
            vm->ip = vm->label_addresses[cinstr->args[0]]; /* jump to the (assumed) function */
            break;
        case RET:
            vm->ip = vm->return_addresses[--vm->ret_sp] + 1;
            break;
        case CALLC:
            vm->c_functions[cinstr->args[0]].func(vm);
            vm->ip++;
            break;
        case STORE:
            set_global(vm, pop_stack(&vm->stack), cinstr->args[0]);
            vm->ip++;
            break;
        case LOAD:
            push_stack(&vm->stack, vm->globals[cinstr->args[0]]);
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
            top = top_stack(&vm->stack);
            if (top->object.m_bool)
                vm->ip = vm->label_addresses[cinstr->args[0]];
            else
                vm->ip++;
            break;
        case IPRINT:
            top = top_stack(&vm->stack);
            if (top == NULL)
                return VM_BADINSTR;
            switch (top->type) {
            case BOOL:
                printf("%d", top->object.m_bool);
                break;
            case INT32:
                printf("%d", top->object.m_int32);
                break;
            case INT64:
                printf("%ld", top->object.m_int64);
                break;
            case DOUBLE:
                printf("%f", top->object.m_double);
                break;
            case STRING:
                printf("%s", top->object.m_string);
                break;
            default:
                return VM_BADDATA;
            }
            vm->ip++;
            break;
        default:
            return VM_INVALID_INSTRUCTION;
        }
    }
    vm->ip = 0;
    return VM_EXIT_FAILURE; // Should exit properly through (EXIT 0)
}

const char* instruction_to_string(Opcode op) {
    switch (op) {
    case EXIT:
        return "EXIT";
    case LOADBOOL:
        return "LOADBOOL";
    case ICONST:
        return "ICONST";
    case LCONST:
        return "LCONST";
    case DCONST:
        return "DCONST";
    case CAST:
        return "CAST";
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
    case IPRINT:
        return "IPRINT";
    default:
        return "UNKNOWN";
    }
    return "";
}
