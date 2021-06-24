#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "./parser.h"
#include "./lex.h"
#include "./debug.h"

State parse_value(Token* token);

/* check whether current identifier leads to a function call */
static inline bool is_function_call(Token* current_token) {
    bool is_preceded = strcmp(NEXT_TOKEN(current_token).value, FUNC_OPEN) == 0;
    if (!IS_START_TOKEN(current_token)) {
        return is_preceded && (strcmp(PREVIOUS_TOKEN(current_token).value, "function") != 0);
    } else {
        return is_preceded;
    }
}

/* check whether current identifier is a function definition */
static inline bool is_function_definition(Token* current_token) {
    if (!IS_START_TOKEN(current_token)) {
        return strcmp(NEXT_TOKEN(current_token).value, FUNC_OPEN) == 0 && (strcmp(PREVIOUS_TOKEN(current_token).value, "function") == 0);
    } else {
        return false;
    }
}

static inline bool is_variable_declaration(Token* current_token) {
    if (!IS_START_TOKEN(current_token)) {
        return strcmp(PREVIOUS_TOKEN(current_token).value, "global") == 0;
    } else {
        return false;
    }
}

static inline bool is_variable_reassignment(Token* current_token) {
    bool is_assigning = current_token->token == IDENTIFIER && strcmp(NEXT_TOKEN(current_token).value, ASSIGNMENT) == 0;
    if (!IS_START_TOKEN(current_token)) {
        return is_assigning && strcmp(PREVIOUS_TOKEN(current_token).value, "global") != 0;
    } else {
        return is_assigning;
    }
}

static inline bool is_array_index(Token* current_token) {
    return current_token->token == IDENTIFIER && (strcmp(NEXT_TOKEN(current_token).value, INDEX_OPEN) == 0);
}

static inline bool is_array(Token* current_token) {
    return strcmp(current_token->value, ARRAY_OPEN) == 0 && strcmp(PREVIOUS_TOKEN(current_token).value, ASSIGNMENT) == 0;
}

static inline bool is_natural_identifier(Token* current_token) {
    return !is_array_index(current_token) && !is_function_call(current_token) && !is_function_definition(current_token);
}

void skip_to_end_call(Token** ptoken) {
    while (strcmp((*ptoken)->value, FUNC_CLOSE) != 0) {
        (*ptoken)++;
        if (is_function_call(*ptoken)) {
            skip_to_end_call(ptoken);
            (*ptoken)++;
        }
    }
}

/* skips to end of statement */
void skip_to_end(Token** ptoken, const char* end, size_t* current_line) {
    while (strcmp((*ptoken)->value, end) != 0) {
        if ((*ptoken)->token == NEWLINE) {
            (*current_line)++;
        }
        (*ptoken)++;
    }
}

/* parses a function call's arguments */
void parse_function_call(State* state, Token* token) {
    Token* call_identifier = token;
    token += 2; /* skip '(' */
    State* arguments = (State*)malloc(MAX_ARGUMENTS * sizeof(State));
    size_t length = 0;
    while (strcmp(token->value, FUNC_CLOSE) != 0) {
        if (token->token == NEWLINE) {
            ++token;
            continue;
        }
        if (strcmp(token->value, ARG_SEPARATOR) != 0) {
            State value = parse_value(token);
            if (value.type == s_FUNCTIONCALL) {
                skip_to_end_call(&token);
            }
            arguments[length++] = value;
        }
        ++token;
    }
    ss_FunctionCall* func_call = (ss_FunctionCall*)malloc(sizeof(ss_FunctionCall));
    strcpy(func_call->function_name, call_identifier->value);
    func_call->arguments = arguments;
    func_call->arg_count = length;
    state->type = s_FUNCTIONCALL;
    state->state = func_call;
}

void parse_literal(State* state, Token* token) {
    ss_Literal* literal = (ss_Literal*)malloc(sizeof(ss_Literal));
    state->type = s_LITERAL;
    if (token->token == ILITERAL) {
        double* value = (double*)malloc(sizeof(double));
        *value = atof(token->value);
        literal->value = value;
        literal->type = l_INTEGER;
    } else if (token->token == SLITERAL) {
        literal->value = &token->value;
        literal->type = l_STRING;
    }
    state->state = literal;
}

void parse_identifier(State* state, Token* token) {
    ss_Identifier* identifier = (ss_Identifier*)malloc(sizeof(ss_Identifier));
    strcpy(identifier->identifier, token->value);
    state->state = identifier;
    state->type = s_IDENTIFIER;
}

void parse_operator(State* state, Token* token) {
    if (is_math_op(token->value[0])) {
        ss_Operator* op = (ss_Operator*)malloc(sizeof(ss_Operator));
        Operator* math_operator = (Operator*)malloc(sizeof(Operator));
        *math_operator = token->value[0];
        op->type = MATH;
        op->op = math_operator;
        state->state = op;
        state->type = s_OPERATOR;
    } else if (is_comparison_op(token->value[0])) {
        /* incomplete */
        ss_Operator* op = (ss_Operator*)malloc(sizeof(ss_Operator));
        Operator* comparison_op = (Operator*)malloc(sizeof(Operator));
        *comparison_op = token->value[0];
        op->type = COMPARISON;
        op->op = comparison_op;
        state->state = op;
        state->type = s_OPERATOR;
    }
}

void parse_array_index(State* state, Token* token) {
    Token* name = token;
    token += 2;
    State* states = (State*)malloc(255 * sizeof(State));
    int used = 0;
    while (strcmp(token->value, INDEX_CLOSE) != 0) {
        states[used++] = parse_value(token);
        if (is_function_call(token)) {
            skip_to_end_call(&token);
        }
        token++;
    }
    ss_IndexOperator* op = (ss_IndexOperator*)malloc(sizeof(ss_IndexOperator));
    op->name = name->value;
    op->states = states;
    state->type = s_INDEX;
    state->state = op;
}

State parse_value(Token* token) {
    State state;
    if (is_function_call(token)) {
        parse_function_call(&state, token);
    } else if (is_array_index(token)) {
        parse_array_index(&state, token);
    } else if (is_literal(token->token)) {
        parse_literal(&state, token);
    } else if (token->token == IDENTIFIER) {
        parse_identifier(&state, token);
    } else if (token->token == OPERATOR) {
        parse_operator(&state, token);
    }
    return state;
}

/* reads the tokens until an ';' is encountered */
ParseObject parse_statement(Token* token, size_t current_line) {
    ParseObject object;
    State* states = (State*)malloc(255 * sizeof(State));
    int used = 0;
    while (strcmp(token->value, EOS) != 0) {
        states[used++] = parse_value(token);
        if (is_function_call(token)) {
            skip_to_end_call(&token);
        } else if (is_array_index(token)) {
            skip_to_end(&token, INDEX_CLOSE, &current_line);
        }
        token++;
        if (token->token == NEWLINE || PREVIOUS_TOKEN(token).is_end) {
            /* statement does not end with an EOS */
            ss_throw("line %lu: expected '%s' after '%s'", current_line, EOS, PREVIOUS_TOKEN(token).value);
        }
    }
    object.states = states;
    object.length = used;
    return object;
}

ss_Variable* create_variable(char* var_name, ParseObject var_states) {
    ss_Variable* variable = (ss_Variable*)malloc(sizeof(ss_Variable));
    strcpy(variable->variable_name, var_name);
    variable->states = var_states;
    variable->is_initialized = true;
    return variable;
}

/* create a reassignment operation */
ss_Reassignment* create_reassignment(char* var_name, ParseObject var_states) {
    ss_Reassignment* reassignment = (ss_Reassignment*)malloc(sizeof(ss_Reassignment));
    strcpy(reassignment->variable_name, var_name);
    reassignment->states = var_states;
    return reassignment;
}

State parse_variable_declaration(Token* current_token, State* states, size_t current_line, size_t length) {
    Token* variable_name = ++current_token; /* name of variable */
    if (variable_name->token == KEYWORD) {
        ss_throw("line %lu: variable name cannot be a reserved keyword", current_line);
    } else if (variable_declared(variable_name, states, length)) {
        ss_throw("line: %lu: attempted redeclaration of variable '%s'\n", current_line, variable_name->value);
    }
    State variable_state;
    if (is_eq(NEXT_TOKEN(current_token).value, EOS)) { /* if declared but uninitialized */
        ss_Variable* variable = (ss_Variable*)malloc(sizeof(ss_Variable));
        strcpy(variable->variable_name, variable_name->value);
        variable->is_initialized = false;

        variable_state.state = variable;
        variable_state.type = s_VARIABLE;
    } else {
        ++current_token; /* skip assignment operator */
        Token* value = ++current_token; /* pointer to first token */
        ParseObject var_state = parse_statement(value, current_line);
        ss_Variable* variable = create_variable(variable_name->value, var_state);

        variable_state.state = variable;
        variable_state.type = s_VARIABLE;
    }
    return variable_state;
}

State parse_variable_reassignment(Token* current_token, State* states, size_t current_line, size_t length) {
    Token* variable_name = current_token;
    if (!IS_START_TOKEN(variable_name) && PREVIOUS_TOKEN(variable_name).token == IDENTIFIER) {
        ss_throw("line %lu: unknown keyword '%s'\n", current_line, PREVIOUS_TOKEN(variable_name).value);
    } else if (!variable_declared(variable_name, states, length)) {
        /* reassignment attempt to a variable that doesn't exist */
        ss_throw("line %lu: variable undeclared '%s'\n", current_line, variable_name->value);
    }
    ++current_token;
    Token* value = ++current_token;
    ParseObject var_state = parse_statement(value, current_line);
    ss_Reassignment* reassignment = create_reassignment(variable_name->value, var_state);

    State reassignment_state = {reassignment, s_REASSIGN};
    return reassignment_state;
}

bool variable_declared(Token* token, State* states, int length) {
    for (int i = 0; i < length; ++i) {
        State state = states[i];
        if (state.type == s_VARIABLE) {
            ss_Variable variable = get_variable(state.state);
            if (strcmp(variable.variable_name, token->value) == 0) {
                return true;
            }
        }
    }
    return false;
}

/* parses an entire scope */
ParseObject parse(lex_Object object) {
    Token* current_token = object.tokens;
    Token* end = &current_token[object.token_used];
    ParseObject parse_obj;
    size_t size = 1000;
    size_t length = 0;
    size_t current_line = 1;
    State* states = (State*)malloc(size * sizeof(State));
    while (current_token != end) {
        if (current_token->token == NEWLINE) {
            current_line++;
        } else if (current_token->token == KEYWORD) {
            if (is_variable_declaration(current_token + 1)) { /* variable declaration? */
                State variable_state = parse_variable_declaration(current_token, states, current_line, length);
                states[length++] = variable_state;
                skip_to_end(&current_token, EOS, &current_line);
            }
        } else if (current_token->token == IDENTIFIER) {
            if (is_function_call(current_token)) {
                states[length++] = parse_value(current_token);
                skip_to_end(&current_token, EOS, &current_line);
            } else if (is_variable_reassignment(current_token)) {
                State variable_state = parse_variable_reassignment(current_token, states, current_line, length);
                states[length++] = variable_state;
                skip_to_end(&current_token, EOS, &current_line);
            } else if (is_array_index(current_token)) {
                states[length++] = parse_value(current_token);
                skip_to_end(&current_token, INDEX_CLOSE, &current_line);
            }
        }
        if (length >= size) {
            size *= 2;
            states = (State*)realloc(states, size * sizeof(State));
        }
        current_token++;
    }
    parse_obj.states = states;
    parse_obj.length = length;
    return parse_obj;
}

void free_state(State* state) {
    if (state->type == s_IDENTIFIER) {
        free(state->state);
    } else if (state->type == s_LITERAL) {
        ss_Literal literal = get_literal(state->state);
        if (literal.type == l_INTEGER) {
            free(literal.value);
        }
        free(state->state);
    } else if (state->type == s_OPERATOR) {
        ss_Operator obj_op = get_operator(state->state);
        if (obj_op.type == MATH || obj_op.type == COMPARISON) {
            free(obj_op.op);
            free(state->state);
        }
    } else if (state->type == s_FUNCTIONCALL) {
        ss_FunctionCall call = get_functioncall(state->state);
        for (int i = 0; i < call.arg_count; ++i) {
            free_state(&call.arguments[i]);
        }
        free(state->state);
    } else if (state->type == s_INDEX) {
        ss_IndexOperator op = get_index(state->state);
        free(op.states);
        free(state->state);
    }
}

void free_ParseObject(ParseObject* object) {
    for (size_t i = 0; i < object->length; ++i) {
        State current = object->states[i];
        if (current.type == s_VARIABLE) {
            ss_Variable var = get_variable(current.state);
            ParseObject states = var.states;
            for (int i = 0; i < states.length; ++i) {
                free_state(&states.states[i]);
            }
            free(states.states);
            free(current.state);
        } else if (current.type == s_FUNCTIONCALL) {
            free_state(&current);
        }
    }
    free(object->states);
}
