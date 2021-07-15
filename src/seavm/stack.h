#ifndef SEAVM_STACK_H
#define SEAVM_STACK_H
#include <stdbool.h>
#include <stdint.h>

#define MAX_STACK_SIZE 4080

typedef double stack_type;

typedef char* t_string;

typedef enum {
    BOOL,
    INT32,
    INT64,
    DOUBLE,
    STRING, /* can only be loaded from a constant pool or CALLC function */
    ARRAY,
} StackObjType;

typedef union {
    bool m_bool;
    int32_t m_int32;
    int64_t m_int64;
    double m_double;
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
} Stack;

Stack create_stack();
void push_stack(Stack* stack, StackObject value);
StackObject pop_stack(Stack* stack);
StackObject* top_stack(Stack* stack);
void terminate_stack(Stack* stack);

#endif // SEAVM_STACK_H
