#ifndef SEAVM_STACK_H
#define SEAVM_STACK_H
#include <stdbool.h>
#include <stdint.h>

typedef double stack_type;

typedef char* t_string;

typedef enum {
    BOOL,
    NUMBER,
    DOUBLE,
    STRING, /* can only be loaded from a constant pool or CALLC function */
    ARRAY,
} StackObjType;

typedef union {
    bool m_bool;
    double m_number;
    t_string m_string;
} DataObject;

typedef struct {
    DataObject object;
    StackObjType type;
} StackObject;

typedef struct {
    StackObject* stack;
    uint64_t total_size;
    uint64_t allocated;
    bool init;
} Stack;

Stack create_stack();
void push_stack(Stack* stack, StackObject value);
StackObject pop_stack(Stack* stack);
StackObject* top_stack(Stack* stack);
void terminate_stack(Stack* stack);

StackObject create_bool(bool val);
StackObject create_number(double val);
StackObject create_string(t_string val);

#endif // SEAVM_STACK_H
