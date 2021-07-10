#include "./stack.h"
#include <stdbool.h>
#include <stdlib.h>

Stack create_stack() {
    Stack stack;
    stack.total_size = 10;
    stack.allocated = 0;
    stack.stack = (StackObject*)malloc(stack.total_size * sizeof(StackObject));
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
        resize_stack(stack, stack->total_size * 2);
    }
    stack->stack[stack->allocated++] = value;
}

inline void pop_stack(Stack* stack) {
    free_stackobject(&stack->stack[--stack->allocated]);
}

inline StackObject* top_stack(Stack* stack) {
    if (stack->allocated == 0)
        return &stack->stack[0];
    return &stack->stack[stack->allocated-1];
}

void terminate_stack(Stack* stack) {
    for (int i = 0; i < stack->allocated; ++i) {
        free_stackobject(&stack->stack[i]);
    }
    free(stack->stack);
}

void free_stackobject(StackObject* object) {
    free(object->object);
    object->object = NULL;
}
