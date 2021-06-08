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

static inline bool is_array_index(Token* current_token) {
    return current_token->token == IDENTIFIER && (strcmp(NEXT_TOKEN(current_token).value, INDEX_OPEN) == 0);
}

/* skips to end of statement */
void skip_to_end(Token** ptoken) {
    while (strcmp((*ptoken)->value, EOS) != 0) {
        (*ptoken)++;
    }
}
/* skip a function call's arguments */
void skip_to_endcall(Token** ptoken) {
    while (strcmp((*ptoken)->value, FUNC_CLOSE) != 0) {
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
        if (strcmp(token->value, ARG_SEPARATOR) != 0) {
            State value = parse_value(token);
            if (value.type == s_FUNCTIONCALL) {
                skip_to_endcall(&token);
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
    if (token->token == ILITERAL) {
        ss_Literal* literal = (ss_Literal*)malloc(sizeof(ss_Literal));
        literal->value = atof(token->value);
        literal->type = l_INTEGER;
        state->state = &(*literal);
        state->type = s_LITERAL;
    } else if (token->token == SLITERAL) {
        /* todo */
    }
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
    } else {
        return; /* also do comparison and index operators */
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
ParseObject parse_statement(Token* token) {
    ParseObject object;
    State* states = (State*)malloc(255 * sizeof(State));
    int used = 0;
    while (strcmp(token->value, EOS) != 0) {
        states[used++] = parse_value(token);
        token++;
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

/* parses an entire scope */
ParseObject parse(lex_Object object) {
    Token* current_token = object.tokens;
    Token* end = &current_token[object.token_used];
    ParseObject parse_obj;
    int size = 1000;
    int length = 0;
    State* states = (State*)malloc(size * sizeof(State));
    while (current_token != end) {
        if (current_token->token == KEYWORD) {
            if (is_variable_declaration(current_token + 1)) { /* variable declaration? */
                Token* variable_name = ++current_token; /* name of variable */
                ++current_token; /* skip assignment operator */
                Token* value = ++current_token; /* pointer to first token */
                ParseObject var_state = parse_statement(value);
                ss_Variable* variable = create_variable(variable_name->value, var_state);

                State variable_state = {&(*variable), s_VARIABLE};
                states[length++] = variable_state;
                skip_to_end(&current_token);
            }
        } else if (current_token->token == IDENTIFIER) {
            if (is_function_call(current_token)) {
                State fcall = parse_value(current_token);
                states[length++] = fcall;
                skip_to_end(&current_token);
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

void free_ParseObject(ParseObject* object) {
    for (size_t i = 0; i < object->length; ++i) {
        State current = object->states[i];
        if (current.type == s_VARIABLE) {
            ss_Variable var = get_variable(current.state);
            ParseObject states = var.states;
            for (int i = 0; i < states.length; ++i) {
                if (states.states[i].type == s_IDENTIFIER) {
                    free(&(get_identifier(states.states[i].state).identifier));
                } else if (states.states[i].type == s_OPERATOR) {
                    free(&(*(Operator*)get_operator(states.states[i].state).op));
                    free(states.states[i].state);
                }
            }
            free(states.states);
        }
    }
    free(object->states);
}
