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
    VM_BADCAST = 3,
    VM_BADDATA = 4,
    VM_BADINSTR = 5,
    VM_STACKOVERFLOW = 6,
    VM_DIVBYZERO = 7
} VmExitCode;


typedef struct VirtualMachine VirtualMachine;

typedef void (*vm_function)(VirtualMachine*);

typedef struct {
    char* name;
    vm_function func;
    int arg_count;
} ss_BaseFunction;

struct VirtualMachine {
    Stack stack;
    size_t ip;
    const ss_BaseFunction* c_functions;

    StackObject* globals;
    size_t global_size;
    size_t global_used;

    int return_addresses[255]; /* stack for return addresses */
    int ret_sp;

    int* label_addresses;
    size_t label_addr_size;
    size_t label_addr_used;

    void** heap_table;
    size_t heap_table_size;
    size_t heap_table_used;
};

const char* instruction_to_string(Opcode op);

void vm_init(VirtualMachine* vm, const ss_BaseFunction* func_list);
void vm_free(VirtualMachine* vm);
int vm_execute(VirtualMachine* vm, Instruction* instrs, uint64_t length);

#endif // SEAVM_VM_H
