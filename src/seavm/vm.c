#include "./vm.h"
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

#define perform_32bitarithmetic(stack, first, op, second)                                   \
    StackObject object;                                                                     \
    object.object.m_int64 = (int64_t)(second.object.m_int32 op first.object.m_int32);       \
    object.type = INT64;                                                                    \
    push_stack(&stack, object);


void vm_init(VirtualMachine* vm, int global_size, const ss_BaseFunction* func_list) {
    vm->stack = create_stack();
    vm->ip = 0;
    vm->c_functions = func_list;
    vm->globals = (StackObject*)malloc(global_size * sizeof(StackObject));
    vm->global_size = global_size;
    vm->global_used = 0;
    vm->ret_sp = 0;
    vm->label_addresses = (int*)malloc(500 * sizeof(int));
    vm->label_addr_size = 500;
    vm->label_addr_used = 0;
}

void vm_free(VirtualMachine* vm) {
    free(vm->globals);
    terminate_stack(&vm->stack);
}

/* casts a 32-bit integer to a different type */
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

/* casts an object to a different type */
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
    return; /* TODO */
}

/* test for a == b */
bool is_equal(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    compare_objects(a, ==, b);
}

/* tests for a < b */
bool is_lt(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    compare_objects(a, <, b);
}

bool is_gt(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    compare_objects(a, >, b);
}

static inline StackObject create_bool(bool n) {
    StackObject object;
    object.object.m_bool = n;
    object.type = BOOL;
    return object;
}

void resolve_labels(VirtualMachine* vm, Instruction* instrs, uint64_t length) {
    for (unsigned int i = 0; i < length; ++i) {
        if (instrs[i].op == LBL) {
            vm->label_addresses[instrs[i].args[0]] = i;
        }
    }
}

int vm_execute(VirtualMachine* vm, Instruction* instrs, uint64_t length) {
    Instruction cinstr;

    StackObject* top;
    StackObject object;

    resolve_labels(vm, instrs, length);
    while (vm->ip != length) {
        cinstr = instrs[vm->ip];

        switch (cinstr.op) {
        case EXIT:
            vm_free(vm);
            return cinstr.args[0];
        case LOADBOOL:
            object.object.m_bool = (bool)cinstr.args[0];
            object.type = BOOL;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case ICONST:
            object.object.m_int32 = cinstr.args[0];
            object.type = INT32;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case LCONST:
            object.object.m_int64 = cinstr.args[0];
            object.type = INT64;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case DCONST:
            object.object.m_double = cinstr.args[0];
            object.type = DOUBLE;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        case CAST:
            perform_cast(top_stack(&vm->stack), (StackObjType)cinstr.args[0]);
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
            vm->ip = cinstr.args[0];
            break;
        case JUMPIF:
            top = top_stack(&vm->stack);
            if (top->object.m_bool) {
                vm->ip = cinstr.args[0];
            } else {
                vm->ip++;
            }
            break;
        case NOT:
            top = top_stack(&vm->stack);
            push_stack(&vm->stack, create_bool(!top->object.m_bool));
            vm->ip++;
            break;
        case IADD: {
            perform_32bitarithmetic(vm->stack, pop_stack(&vm->stack), +, pop_stack(&vm->stack));
            vm->ip++;
            break;
        }
        case ISUB: {
            StackObject first = pop_stack(&vm->stack);
            StackObject second = pop_stack(&vm->stack);
            perform_32bitarithmetic(vm->stack, first, -, second);
            vm->ip++;
            break;
        }
        case IMUL: {
            perform_32bitarithmetic(vm->stack, pop_stack(&vm->stack), *, pop_stack(&vm->stack));
            vm->ip++;
            break;
        }
        case IDIV: {
            StackObject first = pop_stack(&vm->stack);
            StackObject second = pop_stack(&vm->stack);
            if (second.object.m_int32 == 0)
                return VM_DIVBYZERO;
            StackObject object;
            object.object.m_double = (double)second.object.m_int32 / (double)first.object.m_int32;
            object.type = DOUBLE;
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        }
        case CALL:
            vm->return_addresses[vm->ret_sp++] = vm->ip; /* store return address in stack */
            vm->ip = vm->label_addresses[cinstr.args[0]]; /* jump to the (assumed) function */
            break;
        case RET:
            vm->ip = vm->return_addresses[--vm->ret_sp] + 1;
            break;
        case CALLC:
            vm->c_functions[cinstr.args[0]].func(vm);
            vm->ip++;
            break;
        case STORE:
            vm->globals[vm->global_used++] = pop_stack(&vm->stack);
            vm->ip++;
            break;
        case LOAD:
            push_stack(&vm->stack, vm->globals[cinstr.args[0]]);
            vm->ip++;
            break;
        case LBL:
            /* Labels are handled by the address resolver */
            vm->ip++;
            break;
        case LBLJMP:
            vm->ip = vm->label_addresses[cinstr.args[0]];
            break;
        case LBLJMPIF:
            top = top_stack(&vm->stack);
            if (top->object.m_bool)
                vm->ip = vm->label_addresses[cinstr.args[0]];
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
                printf("%lld", top->object.m_int64);
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
    vm_free(vm);
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
    case IADD:
        return "IADD";
    case ISUB:
        return "ISUB";
    case IMUL:
        return "IMUL";
    case IDIV:
        return "IDIV";
    case CALL:
        return "CALL";
    case RET:
        return "RET";
    case CALLC:
        return "CALLC";
    case IPRINT:
        return "IPRINT";
    default:
        return "UNKNOWN";
    }
    return "";
}
