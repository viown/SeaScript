#include "../debug.h"
#include "./stack.h"
#include <stdbool.h>
#include <stdlib.h>

Stack create_stack() {
    Stack stack;
    stack.total_size = 100;
    stack.allocated = 0;
    stack.stack = (StackObject*)ss_malloc(100 * sizeof(StackObject));
    return stack;
}

bool resize_stack(Stack* stack, unsigned long new_size) {
    stack->stack = (StackObject*)realloc(stack->stack, new_size * sizeof(StackObject));
    if (stack->stack != 0) {
        stack->total_size = new_size;
        return 1;
    } else {
        return -1;
    }
}

inline void push_stack(Stack* stack, StackObject value) {
    if (stack->allocated == stack->total_size) {
        stack->total_size *= 2;
        stack->stack = (StackObject*)realloc(stack->stack, stack->total_size * sizeof(StackObject));
    }
    stack->stack[stack->allocated++] = value;
}

inline StackObject pop_stack(Stack* stack) {
    return stack->stack[--stack->allocated];
}

inline StackObject* top_stack(Stack* stack) {
    if (stack->allocated == 0)
        return NULL;
    return &stack->stack[stack->allocated-1];
}

void terminate_stack(Stack* stack) {
    free_and_null(stack->stack);
}

StackObject create_bool(bool val) {
    StackObject object;
    object.object.m_bool = val;
    object.type = BOOL;
    return object;
}

StackObject create_int32(int32_t val) {
    StackObject object;
    object.object.m_int32 = val;
    object.type = INT32;
    return object;
}

StackObject create_int64(int64_t val) {
    StackObject object;
    object.object.m_int64 = val;
    object.type = INT64;
    return object;
}

StackObject create_string(t_string val) {
    StackObject object;
    object.object.m_string = val;
    object.type = STRING;
    return object;
}
