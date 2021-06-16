#ifndef SEAVM_VM_H
#define SEAVM_VM_H
#include <stdint.h>
#include <stdbool.h>
#include "./stack.h"
#include "./bytecode.h"

#define DEBUG
//#undef DEBUG

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef void (*vm_function)(Stack*);

typedef struct {
    Stack stack;
    int ip;

    vm_function c_functions[255]; /* temporary 255 size for testing */
    int func_used;

    int* globals;
    int global_size;
    int global_used;
} Vm;

void vm_init(Vm* vm, int global_size);
void vm_free(Vm* vm);
void vm_register_function(Vm* vm, vm_function func);
bool vm_execute(Vm* vm, Bytecode* bytecode);

#endif // SEAVM_VM_H
