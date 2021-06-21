#ifndef SEAVM_VM_H
#define SEAVM_VM_H
#include <stdint.h>
#include <stdbool.h>
#include "./stack.h"
#include "./bytecode.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef void (*vm_function)(Stack*);

typedef struct {
    char* name;
    vm_function func;
} ss_BaseFunction;

typedef struct {
    Stack stack;
    int ip;
    const ss_BaseFunction* c_functions;
    int* globals;
    int global_size;
    int global_used;
} Vm;

void vm_init(Vm* vm, int global_size, const ss_BaseFunction* func_list);
void vm_free(Vm* vm);
bool vm_execute(Vm* vm, Bytecode* bytecode);

#endif // SEAVM_VM_H
