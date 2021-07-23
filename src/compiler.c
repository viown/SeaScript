#include "./compiler.h"
#include "./debug.h"
#include "./ssfunctions.h"
#include <stdint.h>
#include <string.h>
#include <malloc.h>

Instruction create_instruction(Opcode op) {
    Instruction instruction;
    instruction.op = op;
    for (int i = 0; i < MAX_ARGS; ++i) {
        instruction.args[i] = 0;
    }
    return instruction;
}

void append_instruction(InstructionMap* map, Instruction instruction) {
    if (map->length == map->size) {
        map->size *= 2;
        map->instructions = (Instruction*)realloc(map->instructions, map->size * sizeof(Instruction));
    }
    map->instructions[map->length++] = instruction;
}

/* Push a constant onto the stack */
void push_constant(InstructionMap* map, int32_t constant) {
    Instruction instruction;
    instruction.op = ICONST;
    instruction.args[0] = constant;
    append_instruction(map, instruction);
}

void push_argless_instruction(InstructionMap* map, Opcode op) {
    append_instruction(map, create_instruction(op));
}

void push_instruction1(InstructionMap* map, Opcode op, int arg) {
    Instruction instruction = create_instruction(op);
    instruction.args[0] = arg;
    append_instruction(map, instruction);
}

int get_reference_by_name(char* name, VariableReference* references, int ref_used) {
    for (int i = 0; i < ref_used; ++i) {
        if (!strcmp(references[i].var_name, name))
            return references[i].var_reference;
    }
    return -1;
}


void push_singular_state(ReferenceTable* reftable, State* state) {
    switch (state->type) {
        case s_LITERAL: {
            ss_Literal value = get_literal(state->state);
            push_instruction1(reftable->map, ICONST, load_literal(value));
            break;
        }
        case s_IDENTIFIER: {
            char* identifier = get_identifier(state->state).identifier;
            push_instruction1(reftable->map, LOAD, get_reference_by_name(identifier, reftable->variable_references, reftable->var_reference_length));
            break;
        }
        default:
            ss_throw("Invalid Type");
    }
}

/* Pushes a variable onto the instruction map and returns its global id */
int push_variable(ReferenceTable* reftable, State* state) {
    ss_assert(state->type == s_VARIABLE);
    ss_Variable variable = get_variable(state->state);
    ss_Literal value = get_literal(variable.states.states[0].state); /* NOTE: assumes first value is a literal */
    if (value.type == l_INTEGER) {
        push_instruction1(reftable->map, ICONST, load_literal(value));
        push_instruction1(reftable->map, STORE, reftable->map->global_counter++);
        return reftable->map->global_counter - 1;
    } else {
        ss_throw("Compiler Error: Strings not supported yet");
    }
    return -1;
}


void push_function_call(ReferenceTable* reftable, State* state) {
    ss_assert(state->type == s_FUNCTIONCALL);
    ss_FunctionCall fcall = get_functioncall(state->state);
    for (int i = fcall.arg_count; i >= 0; --i) {
        if (fcall.arguments[i].length == 1) {
            push_singular_state(reftable, &fcall.arguments[i].states[0]);
        } else {
            /* todo */
        }
    }
    int global_func = lookup_global_function(fcall.function_name);
    if (global_func != -1) {
        push_instruction1(reftable->map, CALLC, global_func);
    } else {
        /* todo */
    }
}

void init_reftable(ReferenceTable* table) {
    table->map = (InstructionMap*)ss_malloc(sizeof(InstructionMap));
    table->map->instructions = (Instruction*)ss_malloc(1000 * sizeof(Instruction));
    table->map->size = 1000;
    table->map->length = 0;
    table->map->global_counter = 0;
    table->variable_references = (VariableReference*)ss_malloc(1000 * sizeof(VariableReference));
    table->var_reference_size = 1000;
    table->var_reference_length = 0;
}

ReferenceTable compile(ParseObject* object) {
    ReferenceTable reftable;
    init_reftable(&reftable);
    State* current_state = &object->states[0];
    State* end_state = &object->states[object->length];
    while (current_state != end_state) {
        if (current_state->type == s_VARIABLE) {
            int ref = push_variable(&reftable, current_state);
            VariableReference reference;
            reference.var_name = get_variable(current_state->state).variable_name;
            reference.var_reference = ref;
            reftable.variable_references[reftable.var_reference_length++] = reference;
        } else if (current_state->type == s_FUNCTIONCALL) {
            push_function_call(&reftable, current_state);
        }
        current_state++;
    }
    push_instruction1(reftable.map, EXIT, 0);
    return reftable;
}

void reftable_free(ReferenceTable* table) {
    free_and_null(table->map->instructions);
    free_and_null(table->map);
    free_and_null(table->variable_references);
}

void map_free(InstructionMap* map) {
    free_and_null(map->instructions);
}
