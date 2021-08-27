#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <stdarg.h>
#include "compiler.h"
#include "debug.h"
#include "ssfunctions.h"

extern bool shell_env;
extern bool debug_env;
size_t label_length = 0;

void push_function_call(ReferenceTable* reftable, State* state);

size_t get_label_address(InstructionMap* map, size_t label) {
    size_t addr;
    for (addr = 0; addr < map->length; ++addr) {
        if (map->instructions[addr].op == LBL && map->instructions[addr].args[0] == label) {
            return addr;
        }
    }
    return addr == map->length ? -1 : addr;
}

/* Converts labels to addresses and converting label instructions to regular instructions */
void translate_labels(InstructionMap* map) {
    for (size_t i = 0; i < map->length; ++i) {
        switch (map->instructions[i].op) {
            case LBLJMP:
                map->instructions[i].op = JUMP;
                map->instructions[i].args[0] = get_label_address(map, map->instructions[i].args[0]);
                break;
            case LBLJMPIF:
                map->instructions[i].op = JUMPIF;
                map->instructions[i].args[0] = get_label_address(map, map->instructions[i].args[0]);
                break;
            case LBLCALL:
                map->instructions[i].op = CALL;
                map->instructions[i].args[0] = get_label_address(map, map->instructions[i].args[0]);
                break;
            case LBL:
                map->instructions[i].op = NOP;
                break;
        }
    }
}

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
void push_constant(InstructionMap* map, double constant) {
    Instruction instruction;
    instruction.op = LOADC;
    instruction.args[0] = (int64_t)constant;
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
    else if (is_eq(global, "_env"))
        push_instruction1(map, LOADBOOL, debug_env);
}

void push_singular_state(ReferenceTable* reftable, State* state) {
    switch (state->type) {
    case s_LITERAL: {
        ss_Literal value = get_literal(state->state);
        if (value.type == l_NUMBER) {
            double v = load_literal(value);
            push_instruction1(reftable->map, LOADC, v);
        } else if (value.type == l_STRING) {
            char* str = (char*)value.value;
            size_t check = string_exists(reftable->string_pool, str);
            if (check == -1) {
                char* v = ss_malloc(strlen(str) + 1);
                strcpy(v, str);
                push_instruction1(reftable->map, LOADPOOL, push_to_pool(reftable->string_pool, v));
            } else {
                push_instruction1(reftable->map, LOADPOOL, check);
            }
        }
        break;
    }
    case s_IDENTIFIER: {
        char* identifier = get_identifier(state->state).identifier;
        if (is_global(identifier)) {
            push_globalc(reftable->map, identifier);
        } else {
            int ref = get_reference_by_name(identifier, reftable->variable_references, reftable->var_reference_length);
            if (ref != -1) {
                push_instruction1(reftable->map, LOAD, ref);
            } else {
                ss_throw("line %d: Undefined reference to '%s'\n", state->line, identifier);
            }
        }
        break;
    }
    case s_FUNCTIONCALL:
        push_function_call(reftable, state);
        break;
    default:
        ss_throw("Invalid Type\n");
    }
}

void push_math(InstructionMap* map, char op) {
    switch (op) {
    case '+':
        push_argless_instruction(map, ADD);
        break;
    case '-':
        push_argless_instruction(map, SUB);
        break;
    case '*':
        push_argless_instruction(map, MUL);
        break;
    case '/':
        push_argless_instruction(map, DIV);
        break;
    default:
        ss_throw("Bad operator");
    }
}

void push_comparison(InstructionMap* map, char* op) {
    if (is_eq(op, GREATER_THAN)) {
        push_argless_instruction(map, GT);
    } else if (is_eq(op, LESS_THAN)) {
        push_argless_instruction(map, LT);
    } else if (is_eq(op, EQUAL_TO)) {
        push_argless_instruction(map, EQ);
    } else if (is_eq(op, NOT_EQUAL_TO)) {
        push_argless_instruction(map, EQ);
        push_argless_instruction(map, NOT);
    } else {
        ss_throw("Bad operator");
    }
}

void push_expression(ReferenceTable* reftable, ss_Expression* expression) {
    // This only works for one way operations, e.g `x + y`.
    // TODO: Parse this expression to an AST
    if (expression->length == 3 && expression->states[1].type == s_OPERATOR) {
        ss_Operator op = get_operator(expression->states[1].state);
        push_singular_state(reftable, &expression->states[0]);
        push_singular_state(reftable, &expression->states[2]);
        if (op.type == MATH) {
            push_math(reftable->map, *(char*)op.op);
        } else if (op.type == COMPARISON) {
            push_comparison(reftable->map, (char*)op.op);
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
        ss_throw("line %d: Undefined reference to '%s'\n", state->line, reassignment.variable_name);
    }
}

int lookup_local_function(ReferenceTable* reftable, char* function_name) {
    for (size_t i = 0; i < reftable->functions_length; ++i) {
        if (!strcmp(reftable->functions[i].function_name, function_name))
            return reftable->functions[i].reference;
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
            push_expression(reftable, &fcall.arguments[i]);
        }
    }
    int global_func = lookup_global_function(fcall.function_name);
    if (global_func != -1) {
        push_instruction1(reftable->map, CALLC, global_func);
    } else {
        int local_func = lookup_local_function(reftable, fcall.function_name);
        if (local_func != -1) {
            push_instruction1(reftable->map, LBLCALL, local_func);
        } else {
            ss_throw("line %d: Unknown function '%s'\n", state->line, fcall.function_name);
        }
    }
}

void copy_to(ReferenceTable* to_change, ReferenceTable* data_to_copy) {
    for (size_t i = 0; i < data_to_copy->var_reference_length; ++i) {
        to_change->variable_references[to_change->var_reference_length++] = data_to_copy->variable_references[i];
    }
    to_change->map->global_counter = to_change->var_reference_length;
    to_change->string_pool = data_to_copy->string_pool;
}

void init_map(InstructionMap* map) {
    map->instructions = (Instruction*)ss_malloc(1000 * sizeof(Instruction));
    map->size = 1000;
    map->length = 0;
    map->global_counter = 0;
}

ReferenceTable init_reftable() {
    ReferenceTable table;
    table.map = (InstructionMap*)ss_malloc(sizeof(InstructionMap));
    init_map(table.map);
    table.variable_references = (VariableReference*)ss_malloc(1000 * sizeof(VariableReference));
    table.var_reference_size = 1000;
    table.var_reference_length = 0;
    table.string_pool = (StringPool*)malloc(sizeof(StringPool));
    *table.string_pool = create_string_pool();
    table.functions_size = 5;
    table.functions_length = 0;
    table.functions = (Function*)ss_malloc(sizeof(Function) * table.functions_size);
    return table;
}

void append_function(ReferenceTable* reftable, Function function) {
    if (reftable->functions_length == reftable->functions_size) {
        reftable->functions_size *= 2;
        reftable->functions = realloc(reftable->functions, sizeof(Function) * reftable->functions_size);
    }
    reftable->functions[reftable->functions_length++] = function;
}

void push_function(ReferenceTable* reftable, State* state) {
    InstructionMap* function_instructions = ss_malloc(sizeof(InstructionMap));
    ss_Function function = get_function(state->state);
    Function function_holder;
    init_map(function_instructions);
    strcpy(function_holder.function_name, function.function_name);
    ReferenceTable temp = init_reftable();
    copy_to(&temp, reftable);
    compile_objects(&function.scope, &temp);
    for (int i = 0; i < temp.map->length; ++i) {
        append_instruction(function_instructions, temp.map->instructions[i]);
    }
    function_holder.reference = label_length++;
    function_holder.function_instructions = function_instructions;
    append_function(reftable, function_holder);
}

void push_instructions(InstructionMap* map, InstructionMap* to_push) {
    for (int i = 0; i < to_push->length; ++i) {
        map->instructions[map->length++] = to_push->instructions[i];
        if (map->length == map->size) {
            map->size *= 2;
            map->instructions = (Instruction*)realloc(map->instructions, map->size * sizeof(Instruction));
        }
    }
}

void push_if_statement(ReferenceTable* reftable, ss_IfStatement if_statement) {
    if (if_statement.scope != NULL) {
        if (if_statement.condition.length == 1) {
            push_singular_state(reftable, &if_statement.condition.states[0]);
        } else if (if_statement.condition.length > 1) {
            push_expression(reftable, &if_statement.condition);
        } else {
            push_instruction1(reftable->map, LOADBOOL, 1);
        }
        push_argless_instruction(reftable->map, NOT);
        if (if_statement.else_block == NULL) {
            size_t exit_label = label_length++;
            push_instruction1(reftable->map, LBLJMPIF, exit_label);
            compile_objects(if_statement.scope, reftable);
            push_instruction1(reftable->map, LBL, exit_label);
        } else {
            size_t exit_label = label_length++;
            size_t conditional_label = label_length++;
            push_instruction1(reftable->map, LBLJMPIF, exit_label);
            compile_objects(if_statement.scope, reftable);
            push_instruction1(reftable->map, LBLJMP, conditional_label);
            push_instruction1(reftable->map, LBL, exit_label);
            push_if_statement(reftable, *if_statement.else_block);
            push_instruction1(reftable->map, LBL, conditional_label);
        }
    }
}

void compile_objects(ParseObject* object, ReferenceTable* reftable) {
    if (shell_env) {
        if (reftable->map->length != 0)
            reftable->map->length = 0;
    }
    State* current_state = object->states;
    State* end_state = object->states + object->length;
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
        } else if (current_state->type == s_FUNCTION) {
            push_function(reftable, current_state);
        } else if (current_state->type == s_IFSTATEMENT) {
            push_if_statement(reftable, get_ifstatement(current_state->state));
        }
        current_state++;
    }
}

void compile(ParseObject* object, ReferenceTable* reftable) {
    compile_objects(object, reftable);
    push_instruction1(reftable->map, EXIT, 0);
    for (size_t i = 0; i < reftable->functions_length; ++i) {
        push_instruction1(reftable->map, LBL, reftable->functions[i].reference);
        push_instructions(reftable->map, reftable->functions[i].function_instructions);
        push_argless_instruction(reftable->map, RET);
    }
    translate_labels(reftable->map);
}

void reftable_free(ReferenceTable* table) {
    free_and_null(table->map->instructions);
    free_and_null(table->map);
    free_and_null(table->variable_references);
    free_string_pool(table->string_pool);
}
