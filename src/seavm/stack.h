#ifndef SEAVM_STACK_H
#define SEAVM_STACK_H

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned long stack_size;

#define STACK_TYPE byte

struct Stack {
    STACK_TYPE* stack;
    stack_size total_size;
    stack_size allocated;
};

typedef struct Stack Stack;

Stack create_stack();
void push_stack(Stack* stack, int value);
void pop_stack(Stack* stack);
STACK_TYPE* top_stack(Stack* stack);
void terminate_stack(Stack* stack);

#endif // SEAVM_STACK_H
