#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> /* debug */
#include "./parser.h"
#include "./lex.h"

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

/* skips to end of statement */
void skip_to_end(Token** ptoken, const char* end) {
    while (strcmp((*ptoken)->value, end) != 0) {
        (*ptoken)++;
    }
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
        literal->value = &(*value);
        literal->type = l_INTEGER;
    } else if (token->token == SLITERAL) {
        literal->value = &token->value;
        literal->type = l_STRING;
    }
    state->state = &(*literal);
}

void parse_identifier(State* state, Token* token) {
    ss_Identifier* identifier = (ss_Identifier*)malloc(sizeof(ss_Identifier));
    strcpy(identifier->identifier, token->value);
    state->state = &(*identifier);
    state->type = s_IDENTIFIER;
}

void parse_operator(State* state, Token* token) {
    if (is_math_op(token->value[0])) {
        ss_Operator* op = (ss_Operator*)malloc(sizeof(ss_Operator));
        Operator* math_operator = (Operator*)malloc(sizeof(Operator));
        *math_operator = token->value[0];
        op->type = MATH;
        op->op = &(*math_operator);
        state->state = &(*op);
        state->type = s_OPERATOR;
    } else if (is_comparison_op(token->value[0])) {
        /* incomplete */
        ss_Operator* op = (ss_Operator*)malloc(sizeof(ss_Operator));
        Operator* comparison_op = (Operator*)malloc(sizeof(Operator));
        *comparison_op = token->value[0];
        op->type = COMPARISON;
        op->op = &(*comparison_op);
        state->state = &(*op);
        state->type = s_OPERATOR;
    }
}

State parse_value(Token* token) {
    State state;
    if (is_function_call(token)) {
        parse_function_call(&state, token);
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
        }
        token++;
        if (token->token == NEWLINE || PREVIOUS_TOKEN(token).is_end) {
            /* statement does not end with an EOS */
            parse_error("expected '%s' after '%s' at line %d", EOS, PREVIOUS_TOKEN(token).value, current_line);
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
    return variable;
}

/* create a reassignment operation */
ss_Reassignment* create_reassignment(char* var_name, ParseObject var_states) {
    ss_Reassignment* reassignment = (ss_Reassignment*)malloc(sizeof(ss_Reassignment));
    strcpy(reassignment->variable_name, var_name);
    reassignment->states = var_states;
    return reassignment;
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
                Token* variable_name = ++current_token; /* name of variable */
                if (variable_name->token == KEYWORD) {
                    parse_error("line %d: variable name cannot be a reserved keyword", current_line);
                }
                ++current_token; /* skip assignment operator */
                Token* value = ++current_token; /* pointer to first token */
                ParseObject var_state = parse_statement(value, current_line);
                ss_Variable* variable = create_variable(variable_name->value, var_state);

                State variable_state = {&(*variable), s_VARIABLE};
                states[length++] = variable_state;
                skip_to_end(&current_token, EOS);
            }
        } else if (current_token->token == IDENTIFIER) {
            if (is_function_call(current_token)) {
                State fcall = parse_value(current_token);
                states[length++] = fcall;
                skip_to_end(&current_token, EOS);
            } else if (is_variable_reassignment(current_token)) {
                /* FIXME: Repeated code from variable declaration */
                Token* variable_name = current_token;
                if (!IS_START_TOKEN(variable_name) && PREVIOUS_TOKEN(variable_name).token == IDENTIFIER) {
                    parse_error("line %d: Unknown identifier '%s'\n", current_line, PREVIOUS_TOKEN(variable_name).value);
                } else if (!variable_declared(variable_name, states, length)) {
                    /* reassignment attempt to a variable that doesn't exist */
                    parse_error("line %d: variable undeclared '%s'\n", current_line, variable_name->value);
                }
                ++current_token;
                Token* value = ++current_token;
                ParseObject var_state = parse_statement(value, current_line);
                ss_Reassignment* reassignment = create_reassignment(variable_name->value, var_state);

                State reassignment_state = {&(*reassignment), s_REASSIGN};
                states[length++] = reassignment_state;
                skip_to_end(&current_token, EOS);
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

void parse_error(const char* error, ...) {
    va_list args;
    va_start (args, error);
    printf("PARSE_ERR: "); /* parser error */
    vprintf(error, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void free_state(State* state) {
    if (state->type == s_IDENTIFIER || state->type == s_LITERAL) {
        free(state->state);
    } else if (state->type == s_OPERATOR) {
        ss_Operator obj_op = get_operator(state->state);
        if (obj_op.type == MATH || obj_op.type == COMPARISON) {
            free(obj_op.op);
            free(state->state);
        }
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
        }
    }
    free(object->states);
}
