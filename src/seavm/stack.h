#ifndef SEAVM_STACK_H
#define SEAVM_STACK_H
#include <stdint.h>

typedef double stack_type;

typedef enum {
    INT32,
    INT64,
    DOUBLE,
    STRING, /* can only be loaded from a constant pool or CALLC function */
    ARRAY,
    BOOL,
} StackObjType;

typedef struct {
    void* object;
    StackObjType type;
} StackObject;

typedef struct {
    StackObject* stack;
    uint64_t total_size;
    uint64_t allocated;
} Stack;

Stack create_stack();
void push_stack(Stack* stack, StackObject value);
void pop_stack(Stack* stack);
StackObject* top_stack(Stack* stack);
void terminate_stack(Stack* stack);
void free_stackobject(StackObject* object);

#endif // SEAVM_STACK_H
