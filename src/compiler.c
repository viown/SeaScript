#include "./compiler.h"
#include "./debug.h"
#include "./ssfunctions.h"
#include <stdint.h>
#include <malloc.h>

Instruction create_instruction(Opcode op) {
    Instruction instruction;
    instruction.op = op;
    instruction.args[0] = 0;
    instruction.args[1] = 0;
    instruction.args[2] = 0;
    return instruction;
}

/* Push a constant onto the stack */
void push_constant(InstructionMap* map, int32_t constant) {
    Instruction instruction;
    instruction.op = ICONST;
    instruction.args[0] = constant;
    map->instructions[map->length++] = instruction;
}

void push_argless_instruction(InstructionMap* map, Opcode op) {
    map->instructions[map->length++] = create_instruction(op);
}

void push_instruction1(InstructionMap* map, Opcode op, int arg) {
    Instruction instruction;
    instruction.op = op;
    instruction.args[0] = arg;
    map->instructions[map->length++] = instruction;
}

/* Pushes a variable onto the instruction map and returns its global id */
int push_variable(InstructionMap* map, State* state) {
    if (state->type != s_VARIABLE)
        ss_throw("Internal Bad Error: Expected s_VARIABLE instead got %d", state->type);
    ss_Variable variable = get_variable(state->state);
    ss_Literal value = get_literal(variable.states.states[0].state); /* warning: assumes first value is a literal (could also be a function) */
    if (value.type == l_INTEGER) {
        push_instruction1(map, ICONST, load_literal(value));
        //push_instruction1(map, STORE, map->global_counter++);
        return map->global_counter;
    } else {
        ss_throw("Strings not supported yet");
    }
    return -1;
}

void push_function_call(InstructionMap* map, State* state) {

}

void init_map(InstructionMap* map) {
    map->instructions = (Instruction*)malloc(1000 * sizeof(Instruction));
    map->length = 0;
    map->global_counter = 0;
}

InstructionMap compile(ParseObject* object) {
    InstructionMap map;
    init_map(&map);
    State* current_state = &object->states[0];
    State* end_state = &object->states[object->length];
    while (current_state != end_state) {
        if (current_state->type == s_VARIABLE) {
            push_variable(&map, current_state);
        } else if (current_state->type == s_FUNCTIONCALL) {
            push_function_call(&map, current_state);
        }
        current_state++;
    }
    push_instruction1(&map, EXIT, 0);
    return map;
}


void map_free(InstructionMap* map) {
    free(map->instructions);
}
