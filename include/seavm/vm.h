#ifndef SEAVM_VM_H
#define SEAVM_VM_H
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stack.h"
#include "instruction.h"
#include "bytecode.h"

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
    Stack stack[256];
    size_t ip;
    const ss_BaseFunction* c_functions;

    StackObject* globals;
    size_t global_size;
    size_t global_used;

    StackObject* locals[256];
    size_t local_size[256];
    size_t local_used[256];

    int return_addresses[255]; /* stack for return addresses */
    int sp;

    int* label_addresses;
    size_t label_addr_size;
    size_t label_addr_used;

    void** heap_table;
    size_t heap_table_size;
    size_t heap_table_used;
};

const char* instruction_to_string(Opcode op);

void vm_init(VirtualMachine* vm, const ss_BaseFunction* func_list);
void push_heap_object(VirtualMachine* vm, void* mem_block);
int vm_execute(VirtualMachine* vm, StringPool* pool, Instruction* instrs, size_t length);
void vm_clear(VirtualMachine* vm);
void vm_free(VirtualMachine* vm);
void vm_raise(VirtualMachine* vm, unsigned char exit_code);

#endif // SEAVM_VM_H
