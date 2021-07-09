#ifndef SEAVM_VM_H
#define SEAVM_VM_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "./stack.h"
#include "./instruction.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1


typedef struct Vm Vm;

typedef void (*vm_function)(Vm*);

typedef struct {
    char* name;
    vm_function func;
} ss_BaseFunction;

struct Vm {
    Stack stack;
    int ip;
    const ss_BaseFunction* c_functions;
    int* globals;
    int global_size;
    int global_used;
};

const char* instruction_to_string(Opcode op);

void vm_init(Vm* vm, int global_size, const ss_BaseFunction* func_list);
void vm_free(Vm* vm);
bool vm_execute(Vm* vm, Instruction* instrs, size_t length);

#endif // SEAVM_VM_H
