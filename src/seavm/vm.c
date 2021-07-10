#include "./vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#define cast_object(regType, enumType) \
    regType* number = (regType*)malloc(sizeof(regType)); \
    *number = (regType)*num; \
    free(object->object); \
    object->object = (void*)number; \
    object->type = enumType;

void vm_init(Vm* vm, int global_size, const ss_BaseFunction* func_list) {
    vm->stack = create_stack();
    vm->ip = 0;
    vm->c_functions = func_list;
    vm->global_size = global_size;
    vm->global_used = 0;
    vm->globals = (void**)malloc(global_size * sizeof(void*));
}

void vm_free(Vm* vm) {
    free(vm->globals);
    terminate_stack(&vm->stack);
}

/* casts a 32-bit integer to a different type */
void cast_int(StackObject* object, StackObjType type) {
    int32_t* num = (int32_t*)object->object;
    switch (type) {
    case INT64: {
        cast_object(long long, INT64);
        break;
    }
    case DOUBLE: {
        cast_object(double, DOUBLE);
        break;
    }
    default:
        return;
    }
}

void cast_long(StackObject* object, StackObjType type) {
    long long* num = (long long*)object->object;
    switch (type) {
    case INT32: {
        cast_object(int32_t, INT32);
        break;
    }
    case DOUBLE: {
        cast_object(double, DOUBLE);
        break;
    }
    default:
        return;
    }
}

void cast_double(StackObject* object, StackObjType type) {
    double* num = (double*)object->object;
    switch (type) {
    case INT32: {
        cast_object(int32_t, INT32);
        break;
    }
    case INT64: {
        cast_object(long long, INT64);
        break;
    }
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

bool is_equal(StackObject* a, StackObject* b) {
    StackObjType highest_type = (a->type > b->type) ? a->type : b->type;
    switch (highest_type) {
    case INT32:
        perform_cast(a, INT32);
        perform_cast(b, INT32);
        return *(int32_t*)a->object == *(int32_t*)b->object;
    case INT64:
        perform_cast(a, INT64);
        perform_cast(b, INT64);
        return *(long long*)a->object == *(long long*)b->object;
    case DOUBLE:
        perform_cast(a, DOUBLE);
        perform_cast(b, DOUBLE);
        return *(double*)a->object == *(double*)b->object;
    default:
        return 0;
    }
}

int vm_execute(Vm* vm, Instruction* instrs, uint64_t length) {
    Instruction cinstr;

    while (vm->ip != length) {
        cinstr = instrs[vm->ip];

        switch (cinstr.op) {
        case EXIT:
            vm_free(vm);
            return !(cinstr.args[0] == VM_EXIT_SUCCESS);
        case ICONST: {
            int32_t* num = (int32_t*)malloc(sizeof(int32_t));
            *num = cinstr.args[0];
            StackObject object = {(void*)num, INT32};
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        }
        case LCONST: {
            long long* num = (long long*)malloc(sizeof(long long));
            *num = cinstr.args[0];
            StackObject object = {(void*)num, INT64};
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        }
        case DCONST: {
            double* num = (double*)malloc(sizeof(double));
            *num = cinstr.args[0];
            StackObject object = {(void*)num, DOUBLE};
            push_stack(&vm->stack, object);
            vm->ip++;
            break;
        }
        case CAST: {
            StackObjType to_cast = (StackObjType)cinstr.args[0];
            StackObject* top = top_stack(&vm->stack);
            perform_cast(top, to_cast);
            vm->ip++;
            break;
        }
        case EQ: {
            StackObject* a = top_stack(&vm->stack);
            StackObject* b = a - 1;
            bool* is_eq = (bool*)malloc(sizeof(bool));
            *is_eq = is_equal(a, b);
            StackObject result = {(void*)is_eq, BOOL};
            push_stack(&vm->stack, result);
            vm->ip++;
            break;
        }
        case IPRINT: {
            /* for testing */
            StackObject* top = top_stack(&vm->stack);
            printf("%d", *(bool*)top->object);
            vm->ip++;
            break;
        }
        default:
            return VM_INVALID_INSTRUCTION;
        }
    }
    vm_free(vm);
    return VM_EXIT_FAILURE; // Should exit properly through (EXIT 0)
}

const char* instruction_to_string(Opcode op) {
    /*
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
    */
    return "";
}
