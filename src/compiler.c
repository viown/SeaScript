#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include "compiler.h"
#include "debug.h"
#include "ssfunctions.h"

void push_function_call(ReferenceTable* reftable, State* state);

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

void push_instruction1(InstructionMap* map, Opcode op, arg_type arg) {
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

bool is_global(const char* identifier) {
    for (int i = 0; i < global_count; ++i) {
        if (strcmp(identifier, ss_globals[i]) == 0)
            return true;
    }
    return false;
}

void push_globalc(InstructionMap* map, char* global) {
    if (is_eq(global, "true"))
        push_instruction1(map, LOADBOOL, 1);
    else if (is_eq(global, "false"))
        push_instruction1(map, LOADBOOL, 0);
}

void push_singular_state(ReferenceTable* reftable, State* state) {
    switch (state->type) {
        case s_LITERAL: {
            ss_Literal value = get_literal(state->state);
            double v = load_literal(value);
            if (v >= INT_MAX) {
                push_instruction1(reftable->map, LCONST, load_literal(value));
            } else {
                push_instruction1(reftable->map, ICONST, load_literal(value));
            }
            break;
        }
        case s_IDENTIFIER: {
            char* identifier = get_identifier(state->state).identifier;
            if (is_global(identifier)) {
                push_globalc(reftable->map, identifier);
            } else {
                push_instruction1(reftable->map, LOAD, get_reference_by_name(identifier, reftable->variable_references, reftable->var_reference_length));
            }
            break;
        }
        case s_FUNCTIONCALL:
            push_function_call(reftable, state);
            break;
        default:
            ss_throw("Invalid Type");
    }
}

Opcode get_mathop_from_symbol(char symbol) {
    switch (symbol) {
    case '+':
        return IADD;
    case '-':
        return ISUB;
    case '*':
        return IMUL;
    case '/':
        return IDIV;
    default:
        return -1;
    }
}

Opcode get_comparisonop_from_symbol(char* symbol) {
    if (is_eq(symbol, GREATER_THAN))
        return GT;
    else if (is_eq(symbol, LESS_THAN))
        return LT;
    else if (is_eq(symbol, EQUAL_TO))
        return EQ;
    return -1;
}

void push_expression(ReferenceTable* reftable, ss_Expression* expression) {
    if (expression->length == 3 && expression->states[1].type == s_OPERATOR) {
        ss_Operator op = get_operator(expression->states[1].state);
        push_singular_state(reftable, &expression->states[0]);
        push_singular_state(reftable, &expression->states[2]);
        if (op.type == MATH) {
            push_argless_instruction(reftable->map, get_mathop_from_symbol(((char*)op.op)[0]));
        } else if (op.type == COMPARISON) {
            push_argless_instruction(reftable->map, get_comparisonop_from_symbol((char*)op.op));
        }
    }
}

/* Pushes a variable onto the instruction map and returns its global id */
int push_variable(ReferenceTable* reftable, State* state) {
    ss_assert(state->type == s_VARIABLE);
    ss_Variable variable = get_variable(state->state);
    if (variable.is_initialized) {
        if (variable.states.length == 1) {
            push_singular_state(reftable, &variable.states.states[0]);
        } else {
            push_expression(reftable, &variable.states);
        }
    } else {
        push_instruction1(reftable->map, LOADBOOL, 0);
    }
    push_instruction1(reftable->map, STORE, reftable->map->global_counter++);
    return reftable->map->global_counter - 1;
}

void push_reassignment(ReferenceTable* reftable, State* state) {
    ss_assert(state->type == s_REASSIGN);
    ss_Reassignment reassignment = get_reassignment(state->state);
    int ref = get_reference_by_name(reassignment.variable_name, reftable->variable_references, reftable->var_reference_length);
    if (ref != -1) {
        if (reassignment.states.length == 1) {
            push_singular_state(reftable, &reassignment.states.states[0]);
        } else {
            push_expression(reftable, &reassignment.states);
        }
        push_instruction1(reftable->map, STORE, ref);
    } else {
        ss_throw("Compiler Error: Could not get reference to variable");
    }
}


void push_function_call(ReferenceTable* reftable, State* state) {
    ss_assert(state->type == s_FUNCTIONCALL);
    ss_FunctionCall fcall = get_functioncall(state->state);
    for (int i = fcall.arg_count; i >= 0; --i) {
        if (fcall.arguments[i].length == 1) {
            push_singular_state(reftable, &fcall.arguments[i].states[0]);
        } else {
            push_expression(reftable, &fcall.arguments[i]);
        }
    }
    int global_func = lookup_global_function(fcall.function_name);
    if (global_func != -1) {
        push_instruction1(reftable->map, CALLC, global_func);
    } else {
        /* todo */
    }
}

ReferenceTable init_reftable() {
    ReferenceTable table;
    table.map = (InstructionMap*)ss_malloc(sizeof(InstructionMap));
    table.map->instructions = (Instruction*)ss_malloc(1000 * sizeof(Instruction));
    table.map->size = 1000;
    table.map->length = 0;
    table.map->global_counter = 0;
    table.variable_references = (VariableReference*)ss_malloc(1000 * sizeof(VariableReference));
    table.var_reference_size = 1000;
    table.var_reference_length = 0;
    return table;
}

void copy_to(ReferenceTable* to_change, ReferenceTable* data_to_copy) {
    for (size_t i = 0; i < data_to_copy->var_reference_length; ++i) {
        to_change->variable_references[to_change->var_reference_length++] = data_to_copy->variable_references[i];
    }
}

void compile(ParseObject* object, ReferenceTable* reftable) {
    if (reftable->map->length != 0)
        reftable->map->length = 0;
    State* current_state = &object->states[0];
    State* end_state = &object->states[object->length];
    while (current_state != end_state) {
        if (current_state->type == s_VARIABLE) {
            int ref = push_variable(reftable, current_state);
            VariableReference reference;
            reference.var_name = get_variable(current_state->state).variable_name;
            reference.var_reference = ref;
            reftable->variable_references[reftable->var_reference_length++] = reference;
        } else if (current_state->type == s_FUNCTIONCALL) {
            push_function_call(reftable, current_state);
        } else if (current_state->type == s_REASSIGN) {
            push_reassignment(reftable, current_state);
        }
        current_state++;
    }
    push_instruction1(reftable->map, EXIT, 0);
}

void reftable_free(ReferenceTable* table) {
    free_and_null(table->map->instructions);
    free_and_null(table->map);
    free_and_null(table->variable_references);
}
