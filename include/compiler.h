#ifndef SS_COMPILER_H
#define SS_COMPILER_H
#include "parser.h"
#include "instruction.h"
#include "vm.h"
#include "bytecode.h"

/* whether or not an ss_Number is an int or float */
#define IS_INT(x) ((x - (int)x) == 0)

static const char* const ss_globals[] = {
    "false", "true", "_env"
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
    InstructionMap* function_instructions;
    char function_name[MAX_IDENTIFIER_SIZE];
    size_t reference;
} Function;

typedef struct {
    InstructionMap* map;
    VariableReference* variable_references;
    size_t var_reference_size;
    size_t var_reference_length;
    StringPool* string_pool;
    Function* functions;
    size_t functions_size;
    size_t functions_length;
} ReferenceTable;

ReferenceTable init_reftable();
void compile_objects(ParseObject* object, ReferenceTable* reftable);
void compile(ParseObject* object, ReferenceTable* table);
void reftable_free(ReferenceTable* table);

#endif // SS_COMPILER_H
