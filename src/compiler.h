#ifndef SS_COMPILER_H
#define SS_COMPILER_H
#include "./parser.h"
#include "./seavm/instruction.h"
#include "./seavm/vm.h"
#include "./seavm/bytecode.h"

/* whether or not an ss_Number is an int or float */
#define IS_INT(x) ((x - (int)x) == 0)

typedef struct {
    Instruction* instructions;
    size_t size;
    size_t length;
    int global_counter;
} InstructionMap;

typedef struct {
    char* var_name;
    int var_reference;
} VariableReference;

typedef struct {
    InstructionMap* map;
    VariableReference* variable_references;
    size_t var_reference_size;
    size_t var_reference_length;
} ReferenceTable;

ReferenceTable compile(ParseObject* object);
void reftable_free(ReferenceTable* table);

#endif // SS_COMPILER_H
