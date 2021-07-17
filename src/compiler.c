#include "./compiler.h"
#include "./debug.h"
#include "./ssfunctions.h"
#include <stdint.h>
#include <string.h>
#include <malloc.h>

Instruction create_instruction(Opcode op) {
    Instruction instruction;
    instruction.op = op;
    instruction.args[0] = 0;
    instruction.args[1] = 0;
    instruction.args[2] = 0;
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

/* Pushes a variable onto the instruction map and returns its global id */
int push_variable(InstructionMap* map, State* state) {
    ss_assert(state->type == s_VARIABLE);
    ss_Variable variable = get_variable(state->state);
    ss_Literal value = get_literal(variable.states.states[0].state); /* NOTE: assumes first value is a literal */
    if (value.type == l_INTEGER) {
        push_instruction1(map, ICONST, load_literal(value));
        push_instruction1(map, STORE, map->global_counter++);
        return map->global_counter - 1;
    } else {
        ss_throw("Compiler Error: Strings not supported yet");
    }
    return -1;
}

int get_reference_by_name(char* name, VariableReference* references, int ref_used) {
    for (int i = 0; i < ref_used; ++i) {
        if (!strcmp(references[i].var_name, name))
            return references[i].var_reference;
    }
    return -1;
}

void push_function_call(InstructionMap* map, State* state, VariableReference* references, int ref_used) {
    ss_assert(state->type == s_FUNCTIONCALL);
    //ss_FunctionCall fcall = get_functioncall(state->state);
}

void init_map(InstructionMap* map) {
    map->instructions = (Instruction*)ss_malloc(1000 * sizeof(Instruction));
    map->size = 1000;
    map->length = 0;
    map->global_counter = 0;
}

InstructionMap compile(ParseObject* object) {
    InstructionMap map;
    init_map(&map);
    VariableReference variable_references[1000]; /* todo */
    int var_ref_used = 0;
    State* current_state = &object->states[0];
    State* end_state = &object->states[object->length];
    while (current_state != end_state) {
        if (current_state->type == s_VARIABLE) {
            int ref = push_variable(&map, current_state);
            VariableReference reference;
            reference.var_name = get_variable(current_state->state).variable_name;
            reference.var_reference = ref;
            variable_references[var_ref_used++] = reference;
        } else if (current_state->type == s_FUNCTIONCALL) {
            push_function_call(&map, current_state, variable_references, var_ref_used);
        }
        current_state++;
    }
    push_instruction1(&map, EXIT, 0);
    return map;
}


void map_free(InstructionMap* map) {
    free_and_null(map->instructions);
}
