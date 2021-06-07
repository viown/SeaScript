#ifndef SEAVM_STACK_H
#define SEAVM_STACK_H

typedef unsigned long stack_size;
typedef double stack_type;

struct Stack {
    stack_type* stack;
    stack_size total_size;
    stack_size allocated;
};

typedef struct Stack Stack;

Stack create_stack();
void push_stack(Stack* stack, stack_type value);
void pop_stack(Stack* stack);
stack_type* top_stack(Stack* stack);
void terminate_stack(Stack* stack);

#endif // SEAVM_STACK_H
