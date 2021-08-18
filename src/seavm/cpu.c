#include "cpu.h"

StackObject cpu_add(VirtualMachine* vm) {
    StackObject object;
    StackObject a = pop_stack(&vm->stack);
    StackObject b = pop_stack(&vm->stack);
    if (a.type != DOUBLE && b.type != DOUBLE) {
        object.object.m_int64 = (int64_t)(a.object.m_int64 + b.object.m_int64);
        object.type = INT64;
    } else {
        if (a.type == DOUBLE && b.type != DOUBLE) {
            object.object.m_double = (a.object.m_double + b.object.m_int64);
        } else if (b.type == DOUBLE && a.type != DOUBLE) {
            object.object.m_double = (a.object.m_int64 + b.object.m_double);
        } else if (a.type == DOUBLE && b.type == DOUBLE) {
            object.object.m_double = (a.object.m_double + b.object.m_double);
        }
        object.type = DOUBLE;
    }
    return object;
}

StackObject cpu_sub(VirtualMachine* vm) {
    StackObject object;
    StackObject b = pop_stack(&vm->stack);
    StackObject a = pop_stack(&vm->stack);
    if (a.type != DOUBLE && b.type != DOUBLE) {
        object.object.m_int64 = (int64_t)(a.object.m_int64 - b.object.m_int64);
        object.type = INT64;
    } else {
        if (a.type == DOUBLE && b.type != DOUBLE) {
            object.object.m_double = (a.object.m_double - b.object.m_int64);
        } else if (b.type == DOUBLE && a.type != DOUBLE) {
            object.object.m_double = (a.object.m_int64 - b.object.m_double);
        } else if (a.type == DOUBLE && b.type == DOUBLE) {
            object.object.m_double = (a.object.m_double - b.object.m_double);
        }
        object.type = DOUBLE;
    }
    return object;
}

StackObject cpu_mul(VirtualMachine* vm) {
    StackObject object;
    StackObject b = pop_stack(&vm->stack);
    StackObject a = pop_stack(&vm->stack);
    if (a.type != DOUBLE && b.type != DOUBLE) {
        object.object.m_int64 = (int64_t)(a.object.m_int32 * b.object.m_int32);
        object.type = INT64;
    } else {
        if (a.type == DOUBLE && b.type != DOUBLE) {
            object.object.m_double = (a.object.m_double * b.object.m_int32);
        } else if (b.type == DOUBLE && a.type != DOUBLE) {
            object.object.m_double = (a.object.m_int32 * b.object.m_double);
        } else if (a.type == DOUBLE && b.type == DOUBLE) {
            object.object.m_double = (a.object.m_double * b.object.m_double);
        }
        object.type = DOUBLE;
    }
    return object;
}

StackObject cpu_div(VirtualMachine* vm) {
    StackObject b = pop_stack(&vm->stack);
    StackObject a = pop_stack(&vm->stack);
    if (a.object.m_int32 == 0)
        vm_raise(vm, VM_DIVBYZERO);
    StackObject object;
    if (a.type != DOUBLE && b.type != DOUBLE) {
        object.object.m_double = (double)a.object.m_int32 / (double)b.object.m_int32;
    } else {
        if (a.type == DOUBLE && b.type != DOUBLE) {
            object.object.m_double = (a.object.m_double / (double)b.object.m_int32);
        } else if (b.type == DOUBLE && a.type != DOUBLE) {
            object.object.m_double = ((double)a.object.m_int32 / b.object.m_double);
        } else if (a.type == DOUBLE && b.type == DOUBLE) {
            object.object.m_double = (a.object.m_double / b.object.m_double);
        }
        object.type = DOUBLE;
    }
    object.type = DOUBLE;
    return object;
}
