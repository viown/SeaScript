#ifndef SEAVM_STACK_H
#define SEAVM_STACK_H

typedef unsigned long stack_size;
typedef double stack_type;

typedef enum {
    INT32,
    INT64,
    FLOAT,
    DOUBLE,
    STRING, /* can only be loaded from a constant pool or CALLC function */
    ARRAY,
} StackObjType;

typedef struct {
    void* object;
    StackObjType type;
} StackObject;

typedef struct {
    stack_type* stack;
    stack_size total_size;
    stack_size allocated;
} Stack;

Stack create_stack();
void push_stack(Stack* stack, stack_type value);
void pop_stack(Stack* stack);
stack_type* top_stack(Stack* stack);
void terminate_stack(Stack* stack);

#endif // SEAVM_STACK_H
