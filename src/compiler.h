#ifndef SS_COMPILER_H
#define SS_COMPILER_H
#include "parser.h"
#include "instruction.h"
#include "vm.h"
#include "bytecode.h"

/* whether or not an ss_Number is an int or float */
#define IS_INT(x) ((x - (int)x) == 0)

static const char* const ss_globals[] = {
    "false", "true"
};
static const int global_count = LEN(ss_globals);

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

ReferenceTable init_reftable();
void compile(ParseObject* object, ReferenceTable* table);
void reftable_free(ReferenceTable* table);

#endif // SS_COMPILER_H
