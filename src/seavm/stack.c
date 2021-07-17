#include "../debug.h"
#include "./stack.h"
#include <stdbool.h>
#include <stdlib.h>

Stack create_stack() {
    Stack stack;
    stack.total_size = 10;
    stack.allocated = 0;
    stack.stack = (StackObject*)malloc(MAX_STACK_SIZE);
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
