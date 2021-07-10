#ifndef SEAVM_VM_H
#define SEAVM_VM_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "./stack.h"
#include "./instruction.h"

typedef enum {
    VM_EXIT_SUCCESS = 0,
    VM_EXIT_FAILURE = 1,
    VM_INVALID_INSTRUCTION = 2,
    VM_BADCAST = 3
} VmExitCode;


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
    void** globals;
    int global_size;
    int global_used;
};

const char* instruction_to_string(Opcode op);

void vm_init(Vm* vm, int global_size, const ss_BaseFunction* func_list);
void vm_free(Vm* vm);
int vm_execute(Vm* vm, Instruction* instrs, uint64_t length);

#endif // SEAVM_VM_H
